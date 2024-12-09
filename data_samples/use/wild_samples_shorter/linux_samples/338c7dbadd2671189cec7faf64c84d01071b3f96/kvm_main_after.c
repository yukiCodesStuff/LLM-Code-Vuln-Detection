	int r;
	struct kvm_vcpu *vcpu, *v;

	if (id >= KVM_MAX_VCPUS)
		return -EINVAL;

	vcpu = kvm_arch_vcpu_create(kvm, id);
	if (IS_ERR(vcpu))
		return PTR_ERR(vcpu);
