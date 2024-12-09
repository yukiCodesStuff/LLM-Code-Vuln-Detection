	WARN_ON_ONCE(!init_event &&
		     (old_cr0 || kvm_read_cr3(vcpu) || kvm_read_cr4(vcpu)));

	kvm_lapic_reset(vcpu, init_event);

	vcpu->arch.hflags = 0;

	vcpu->arch.smi_pending = 0;
	vcpu->arch.smi_count = 0;