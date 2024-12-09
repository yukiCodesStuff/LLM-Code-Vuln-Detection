{
	unsigned long rflags = static_call(kvm_x86_get_rflags)(vcpu);
	int r;

	r = static_call(kvm_x86_skip_emulated_instruction)(vcpu);
	if (unlikely(!r))
		return 0;

	kvm_pmu_trigger_event(vcpu, PERF_COUNT_HW_INSTRUCTIONS);

	/*
	 * rflags is the old, "raw" value of the flags.  The new value has
	 * not been saved yet.
	 *
	 * This is correct even for TF set by the guest, because "the
	 * processor will not generate this exception after the instruction
	 * that sets the TF flag".
	 */
	if (unlikely(rflags & X86_EFLAGS_TF))
		r = kvm_vcpu_do_singlestep(vcpu);
	return r;
}
EXPORT_SYMBOL_GPL(kvm_skip_emulated_instruction);

static bool kvm_vcpu_check_code_breakpoint(struct kvm_vcpu *vcpu, int *r)
{
	if (unlikely(vcpu->guest_debug & KVM_GUESTDBG_USE_HW_BP) &&
	    (vcpu->arch.guest_debug_dr7 & DR7_BP_EN_MASK)) {
		struct kvm_run *kvm_run = vcpu->run;
		unsigned long eip = kvm_get_linear_rip(vcpu);
		u32 dr6 = kvm_vcpu_check_hw_bp(eip, 0,
					   vcpu->arch.guest_debug_dr7,
					   vcpu->arch.eff_db);

		if (dr6 != 0) {
			kvm_run->debug.arch.dr6 = dr6 | DR6_ACTIVE_LOW;
			kvm_run->debug.arch.pc = eip;
			kvm_run->debug.arch.exception = DB_VECTOR;
			kvm_run->exit_reason = KVM_EXIT_DEBUG;
			*r = 0;
			return true;
		}
	}

	if (unlikely(vcpu->arch.dr7 & DR7_BP_EN_MASK) &&
	    !(kvm_get_rflags(vcpu) & X86_EFLAGS_RF)) {
		unsigned long eip = kvm_get_linear_rip(vcpu);
		u32 dr6 = kvm_vcpu_check_hw_bp(eip, 0,
					   vcpu->arch.dr7,
					   vcpu->arch.db);

		if (dr6 != 0) {
			kvm_queue_exception_p(vcpu, DB_VECTOR, dr6);
			*r = 1;
			return true;
		}
	}

	return false;
}
		switch (ctxt->b) {
		case 0x33:	/* RDPMC */
			return true;
		}
		break;
	}

	return false;
}

/*
 * Decode an instruction for emulation.  The caller is responsible for handling
 * code breakpoints.  Note, manually detecting code breakpoints is unnecessary
 * (and wrong) when emulating on an intercepted fault-like exception[*], as
 * code breakpoints have higher priority and thus have already been done by
 * hardware.
 *
 * [*] Except #MC, which is higher priority, but KVM should never emulate in
 *     response to a machine check.
 */
int x86_decode_emulated_instruction(struct kvm_vcpu *vcpu, int emulation_type,
				    void *insn, int insn_len)
{
	struct x86_emulate_ctxt *ctxt = vcpu->arch.emulate_ctxt;
	int r;

	init_emulate_ctxt(vcpu);

	r = x86_decode_insn(ctxt, insn, insn_len, emulation_type);

	trace_kvm_emulate_insn_start(vcpu);
	++vcpu->stat.insn_emulation;

	return r;
}
	if (!(emulation_type & EMULTYPE_NO_DECODE)) {
		kvm_clear_exception_queue(vcpu);

		/*
		 * Return immediately if RIP hits a code breakpoint, such #DBs
		 * are fault-like and are higher priority than any faults on
		 * the code fetch itself.
		 */
		if (!(emulation_type & EMULTYPE_SKIP) &&
		    kvm_vcpu_check_code_breakpoint(vcpu, &r))
			return r;

		r = x86_decode_emulated_instruction(vcpu, emulation_type,
						    insn, insn_len);
		if (r != EMULATION_OK)  {
			if ((emulation_type & EMULTYPE_TRAP_UD) ||
			    (emulation_type & EMULTYPE_TRAP_UD_FORCED)) {
				kvm_queue_exception(vcpu, UD_VECTOR);
				return 1;
			}
			if (reexecute_instruction(vcpu, cr2_or_gpa,
						  write_fault_to_spt,
						  emulation_type))
				return 1;
			if (ctxt->have_exception) {
				/*
				 * #UD should result in just EMULATION_FAILED, and trap-like
				 * exception should not be encountered during decode.
				 */
				WARN_ON_ONCE(ctxt->exception.vector == UD_VECTOR ||
					     exception_type(ctxt->exception.vector) == EXCPT_TRAP);
				inject_emulated_exception(vcpu);
				return 1;
			}
			return handle_emulation_failure(vcpu, emulation_type);
		}