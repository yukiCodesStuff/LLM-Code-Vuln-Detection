	kvm_x86_ops->get_cs_db_l_bits(vcpu, &cs_db, &cs_l);

	ctxt->eflags = kvm_get_rflags(vcpu);
	ctxt->eip = kvm_rip_read(vcpu);
	ctxt->mode = (!is_protmode(vcpu))		? X86EMUL_MODE_REAL :
		     (ctxt->eflags & X86_EFLAGS_VM)	? X86EMUL_MODE_VM86 :
		     (cs_l && is_long_mode(vcpu))	? X86EMUL_MODE_PROT64 :
	return dr6;
}

static void kvm_vcpu_check_singlestep(struct kvm_vcpu *vcpu, unsigned long rflags, int *r)
{
	struct kvm_run *kvm_run = vcpu->run;

	/*
	 * rflags is the old, "raw" value of the flags.  The new value has
	 * not been saved yet.
	 *
	 * This is correct even for TF set by the guest, because "the
	 * processor will not generate this exception after the instruction
	 * that sets the TF flag".
	 */
	if (unlikely(rflags & X86_EFLAGS_TF)) {
		if (vcpu->guest_debug & KVM_GUESTDBG_SINGLESTEP) {
			kvm_run->debug.arch.dr6 = DR6_BS | DR6_FIXED_1 |
						  DR6_RTM;
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
}

int kvm_skip_emulated_instruction(struct kvm_vcpu *vcpu)
	int r = EMULATE_DONE;

	kvm_x86_ops->skip_emulated_instruction(vcpu);
	kvm_vcpu_check_singlestep(vcpu, rflags, &r);
	return r == EMULATE_DONE;
}
EXPORT_SYMBOL_GPL(kvm_skip_emulated_instruction);

		toggle_interruptibility(vcpu, ctxt->interruptibility);
		vcpu->arch.emulate_regs_need_sync_to_vcpu = false;
		kvm_rip_write(vcpu, ctxt->eip);
		if (r == EMULATE_DONE)
			kvm_vcpu_check_singlestep(vcpu, rflags, &r);
		if (!ctxt->have_exception ||
		    exception_type(ctxt->exception.vector) == EXCPT_TRAP)
			__kvm_set_rflags(vcpu, ctxt->eflags);
