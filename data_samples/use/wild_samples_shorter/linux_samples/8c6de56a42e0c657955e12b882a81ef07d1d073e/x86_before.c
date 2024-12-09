	if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
		return;

	vcpu->arch.st.steal.preempted = KVM_VCPU_PREEMPTED;

	kvm_write_guest_offset_cached(vcpu->kvm, &vcpu->arch.st.stime,
			&vcpu->arch.st.steal.preempted,