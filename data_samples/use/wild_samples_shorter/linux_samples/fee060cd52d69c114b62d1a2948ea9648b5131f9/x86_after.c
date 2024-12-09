}
EXPORT_SYMBOL_GPL(kvm_skip_emulated_instruction);

static bool kvm_vcpu_check_code_breakpoint(struct kvm_vcpu *vcpu, int *r)
{
	if (unlikely(vcpu->guest_debug & KVM_GUESTDBG_USE_HW_BP) &&
	    (vcpu->arch.guest_debug_dr7 & DR7_BP_EN_MASK)) {
		struct kvm_run *kvm_run = vcpu->run;
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