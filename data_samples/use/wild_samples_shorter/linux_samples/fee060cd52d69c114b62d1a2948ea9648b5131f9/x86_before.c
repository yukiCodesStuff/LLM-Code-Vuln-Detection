}
EXPORT_SYMBOL_GPL(kvm_skip_emulated_instruction);

static bool kvm_vcpu_check_breakpoint(struct kvm_vcpu *vcpu, int *r)
{
	if (unlikely(vcpu->guest_debug & KVM_GUESTDBG_USE_HW_BP) &&
	    (vcpu->arch.guest_debug_dr7 & DR7_BP_EN_MASK)) {
		struct kvm_run *kvm_run = vcpu->run;
}

/*
 * Decode to be emulated instruction. Return EMULATION_OK if success.
 */
int x86_decode_emulated_instruction(struct kvm_vcpu *vcpu, int emulation_type,
				    void *insn, int insn_len)
{
	int r = EMULATION_OK;
	struct x86_emulate_ctxt *ctxt = vcpu->arch.emulate_ctxt;

	init_emulate_ctxt(vcpu);

	/*
	 * We will reenter on the same instruction since we do not set
	 * complete_userspace_io. This does not handle watchpoints yet,
	 * those would be handled in the emulate_ops.
	 */
	if (!(emulation_type & EMULTYPE_SKIP) &&
	    kvm_vcpu_check_breakpoint(vcpu, &r))
		return r;

	r = x86_decode_insn(ctxt, insn, insn_len, emulation_type);

	trace_kvm_emulate_insn_start(vcpu);
	++vcpu->stat.insn_emulation;
	if (!(emulation_type & EMULTYPE_NO_DECODE)) {
		kvm_clear_exception_queue(vcpu);

		r = x86_decode_emulated_instruction(vcpu, emulation_type,
						    insn, insn_len);
		if (r != EMULATION_OK)  {
			if ((emulation_type & EMULTYPE_TRAP_UD) ||