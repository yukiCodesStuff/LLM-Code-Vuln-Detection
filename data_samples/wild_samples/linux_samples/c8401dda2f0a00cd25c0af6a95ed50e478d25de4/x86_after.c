{
	struct x86_emulate_ctxt *ctxt = &vcpu->arch.emulate_ctxt;
	int cs_db, cs_l;

	kvm_x86_ops->get_cs_db_l_bits(vcpu, &cs_db, &cs_l);

	ctxt->eflags = kvm_get_rflags(vcpu);
	ctxt->tf = (ctxt->eflags & X86_EFLAGS_TF) != 0;

	ctxt->eip = kvm_rip_read(vcpu);
	ctxt->mode = (!is_protmode(vcpu))		? X86EMUL_MODE_REAL :
		     (ctxt->eflags & X86_EFLAGS_VM)	? X86EMUL_MODE_VM86 :
		     (cs_l && is_long_mode(vcpu))	? X86EMUL_MODE_PROT64 :
		     cs_db				? X86EMUL_MODE_PROT32 :
							  X86EMUL_MODE_PROT16;
	BUILD_BUG_ON(HF_GUEST_MASK != X86EMUL_GUEST_MASK);
	BUILD_BUG_ON(HF_SMM_MASK != X86EMUL_SMM_MASK);
	BUILD_BUG_ON(HF_SMM_INSIDE_NMI_MASK != X86EMUL_SMM_INSIDE_NMI_MASK);

	init_decode_cache(ctxt);
	vcpu->arch.emulate_regs_need_sync_from_vcpu = false;
}

int kvm_inject_realmode_interrupt(struct kvm_vcpu *vcpu, int irq, int inc_eip)
{
	struct x86_emulate_ctxt *ctxt = &vcpu->arch.emulate_ctxt;
	int ret;

	init_emulate_ctxt(vcpu);

	ctxt->op_bytes = 2;
	ctxt->ad_bytes = 2;
	ctxt->_eip = ctxt->eip + inc_eip;
	ret = emulate_int_real(ctxt, irq);

	if (ret != X86EMUL_CONTINUE)
		return EMULATE_FAIL;

	ctxt->eip = ctxt->_eip;
	kvm_rip_write(vcpu, ctxt->eip);
	kvm_set_rflags(vcpu, ctxt->eflags);

	if (irq == NMI_VECTOR)
		vcpu->arch.nmi_pending = 0;
	else
		vcpu->arch.interrupt.pending = false;

	return EMULATE_DONE;
}
EXPORT_SYMBOL_GPL(kvm_inject_realmode_interrupt);

static int handle_emulation_failure(struct kvm_vcpu *vcpu)
{
	int r = EMULATE_DONE;

	++vcpu->stat.insn_emulation_fail;
	trace_kvm_emulate_insn_failed(vcpu);
	if (!is_guest_mode(vcpu) && kvm_x86_ops->get_cpl(vcpu) == 0) {
		vcpu->run->exit_reason = KVM_EXIT_INTERNAL_ERROR;
		vcpu->run->internal.suberror = KVM_INTERNAL_ERROR_EMULATION;
		vcpu->run->internal.ndata = 0;
		r = EMULATE_FAIL;
	}
{
	u32 dr6 = 0;
	int i;
	u32 enable, rwlen;

	enable = dr7;
	rwlen = dr7 >> 16;
	for (i = 0; i < 4; i++, enable >>= 2, rwlen >>= 4)
		if ((enable & 3) && (rwlen & 15) == type && db[i] == addr)
			dr6 |= (1 << i);
	return dr6;
}

static void kvm_vcpu_do_singlestep(struct kvm_vcpu *vcpu, int *r)
{
	struct kvm_run *kvm_run = vcpu->run;

	if (vcpu->guest_debug & KVM_GUESTDBG_SINGLESTEP) {
		kvm_run->debug.arch.dr6 = DR6_BS | DR6_FIXED_1 | DR6_RTM;
		kvm_run->debug.arch.pc = vcpu->arch.singlestep_rip;
		kvm_run->debug.arch.exception = DB_VECTOR;
		kvm_run->exit_reason = KVM_EXIT_DEBUG;
		*r = EMULATE_USER_EXIT;
	} else {
		/*
		 * "Certain debug exceptions may clear bit 0-3.  The
		 * remaining contents of the DR6 register are never
		 * cleared by the processor".
		 */
		vcpu->arch.dr6 &= ~15;
		vcpu->arch.dr6 |= DR6_BS | DR6_RTM;
		kvm_queue_exception(vcpu, DB_VECTOR);
	}
}
{
	unsigned long rflags = kvm_x86_ops->get_rflags(vcpu);
	int r = EMULATE_DONE;

	kvm_x86_ops->skip_emulated_instruction(vcpu);

	/*
	 * rflags is the old, "raw" value of the flags.  The new value has
	 * not been saved yet.
	 *
	 * This is correct even for TF set by the guest, because "the
	 * processor will not generate this exception after the instruction
	 * that sets the TF flag".
	 */
	if (unlikely(rflags & X86_EFLAGS_TF))
		kvm_vcpu_do_singlestep(vcpu, &r);
	return r == EMULATE_DONE;
}
EXPORT_SYMBOL_GPL(kvm_skip_emulated_instruction);

static bool kvm_vcpu_check_breakpoint(struct kvm_vcpu *vcpu, int *r)
{
	if (unlikely(vcpu->guest_debug & KVM_GUESTDBG_USE_HW_BP) &&
	    (vcpu->arch.guest_debug_dr7 & DR7_BP_EN_MASK)) {
		struct kvm_run *kvm_run = vcpu->run;
		unsigned long eip = kvm_get_linear_rip(vcpu);
		u32 dr6 = kvm_vcpu_check_hw_bp(eip, 0,
					   vcpu->arch.guest_debug_dr7,
					   vcpu->arch.eff_db);

		if (dr6 != 0) {
			kvm_run->debug.arch.dr6 = dr6 | DR6_FIXED_1 | DR6_RTM;
			kvm_run->debug.arch.pc = eip;
			kvm_run->debug.arch.exception = DB_VECTOR;
			kvm_run->exit_reason = KVM_EXIT_DEBUG;
			*r = EMULATE_USER_EXIT;
			return true;
		}
	}
	if (writeback) {
		unsigned long rflags = kvm_x86_ops->get_rflags(vcpu);
		toggle_interruptibility(vcpu, ctxt->interruptibility);
		vcpu->arch.emulate_regs_need_sync_to_vcpu = false;
		kvm_rip_write(vcpu, ctxt->eip);
		if (r == EMULATE_DONE &&
		    (ctxt->tf || (vcpu->guest_debug & KVM_GUESTDBG_SINGLESTEP)))
			kvm_vcpu_do_singlestep(vcpu, &r);
		if (!ctxt->have_exception ||
		    exception_type(ctxt->exception.vector) == EXCPT_TRAP)
			__kvm_set_rflags(vcpu, ctxt->eflags);

		/*
		 * For STI, interrupts are shadowed; so KVM_REQ_EVENT will
		 * do nothing, and it will be requested again as soon as
		 * the shadow expires.  But we still need to check here,
		 * because POPF has no interrupt shadow.
		 */
		if (unlikely((ctxt->eflags & ~rflags) & X86_EFLAGS_IF))
			kvm_make_request(KVM_REQ_EVENT, vcpu);
	} else
		vcpu->arch.emulate_regs_need_sync_to_vcpu = true;

	return r;
}
EXPORT_SYMBOL_GPL(x86_emulate_instruction);

int kvm_fast_pio_out(struct kvm_vcpu *vcpu, int size, unsigned short port)
{
	unsigned long val = kvm_register_read(vcpu, VCPU_REGS_RAX);
	int ret = emulator_pio_out_emulated(&vcpu->arch.emulate_ctxt,
					    size, port, &val, 1);
	/* do not return to emulator after return from userspace */
	vcpu->arch.pio.count = 0;
	return ret;
}
EXPORT_SYMBOL_GPL(kvm_fast_pio_out);

static int complete_fast_pio_in(struct kvm_vcpu *vcpu)
{
	unsigned long val;

	/* We should only ever be called with arch.pio.count equal to 1 */
	BUG_ON(vcpu->arch.pio.count != 1);

	/* For size less than 4 we merge, else we zero extend */
	val = (vcpu->arch.pio.size < 4) ? kvm_register_read(vcpu, VCPU_REGS_RAX)
					: 0;

	/*
	 * Since vcpu->arch.pio.count == 1 let emulator_pio_in_emulated perform
	 * the copy and tracing
	 */
	emulator_pio_in_emulated(&vcpu->arch.emulate_ctxt, vcpu->arch.pio.size,
				 vcpu->arch.pio.port, &val, 1);
	kvm_register_write(vcpu, VCPU_REGS_RAX, val);

	return 1;
}

int kvm_fast_pio_in(struct kvm_vcpu *vcpu, int size, unsigned short port)
{
	unsigned long val;
	int ret;

	/* For size less than 4 we merge, else we zero extend */
	val = (size < 4) ? kvm_register_read(vcpu, VCPU_REGS_RAX) : 0;

	ret = emulator_pio_in_emulated(&vcpu->arch.emulate_ctxt, size, port,
				       &val, 1);
	if (ret) {
		kvm_register_write(vcpu, VCPU_REGS_RAX, val);
		return ret;
	}