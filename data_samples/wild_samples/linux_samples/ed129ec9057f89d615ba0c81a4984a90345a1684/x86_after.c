{
	struct kvm_cpuid_entry2 *cpuid_0x1;
	unsigned long old_cr0 = kvm_read_cr0(vcpu);
	unsigned long new_cr0;

	/*
	 * Several of the "set" flows, e.g. ->set_cr0(), read other registers
	 * to handle side effects.  RESET emulation hits those flows and relies
	 * on emulated/virtualized registers, including those that are loaded
	 * into hardware, to be zeroed at vCPU creation.  Use CRs as a sentinel
	 * to detect improper or missing initialization.
	 */
	WARN_ON_ONCE(!init_event &&
		     (old_cr0 || kvm_read_cr3(vcpu) || kvm_read_cr4(vcpu)));

	/*
	 * SVM doesn't unconditionally VM-Exit on INIT and SHUTDOWN, thus it's
	 * possible to INIT the vCPU while L2 is active.  Force the vCPU back
	 * into L1 as EFER.SVME is cleared on INIT (along with all other EFER
	 * bits), i.e. virtualization is disabled.
	 */
	if (is_guest_mode(vcpu))
		kvm_leave_nested(vcpu);

	kvm_lapic_reset(vcpu, init_event);

	WARN_ON_ONCE(is_guest_mode(vcpu) || is_smm(vcpu));
	vcpu->arch.hflags = 0;

	vcpu->arch.smi_pending = 0;
	vcpu->arch.smi_count = 0;
	atomic_set(&vcpu->arch.nmi_queued, 0);
	vcpu->arch.nmi_pending = 0;
	vcpu->arch.nmi_injected = false;
	kvm_clear_interrupt_queue(vcpu);
	kvm_clear_exception_queue(vcpu);

	memset(vcpu->arch.db, 0, sizeof(vcpu->arch.db));
	kvm_update_dr0123(vcpu);
	vcpu->arch.dr6 = DR6_ACTIVE_LOW;
	vcpu->arch.dr7 = DR7_FIXED_1;
	kvm_update_dr7(vcpu);

	vcpu->arch.cr2 = 0;

	kvm_make_request(KVM_REQ_EVENT, vcpu);
	vcpu->arch.apf.msr_en_val = 0;
	vcpu->arch.apf.msr_int_val = 0;
	vcpu->arch.st.msr_val = 0;

	kvmclock_reset(vcpu);

	kvm_clear_async_pf_completion_queue(vcpu);
	kvm_async_pf_hash_reset(vcpu);
	vcpu->arch.apf.halted = false;

	if (vcpu->arch.guest_fpu.fpstate && kvm_mpx_supported()) {
		struct fpstate *fpstate = vcpu->arch.guest_fpu.fpstate;

		/*
		 * All paths that lead to INIT are required to load the guest's
		 * FPU state (because most paths are buried in KVM_RUN).
		 */
		if (init_event)
			kvm_put_guest_fpu(vcpu);

		fpstate_clear_xstate_component(fpstate, XFEATURE_BNDREGS);
		fpstate_clear_xstate_component(fpstate, XFEATURE_BNDCSR);

		if (init_event)
			kvm_load_guest_fpu(vcpu);
	}