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