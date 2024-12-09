		r = -EFAULT;
		if (copy_from_user(&va, argp, sizeof va))
			goto out;
		r = kvm_lapic_set_vapic_addr(vcpu, va.vapic_addr);
		break;
	}
	case KVM_X86_SETUP_MCE: {
		u64 mcg_cap;
			!kvm_event_needs_reinjection(vcpu);
}

static void update_cr8_intercept(struct kvm_vcpu *vcpu)
{
	int max_irr, tpr;

	struct kvm *kvm = vcpu->kvm;

	vcpu->srcu_idx = srcu_read_lock(&kvm->srcu);

	r = 1;
	while (r > 0) {
		if (vcpu->arch.mp_state == KVM_MP_STATE_RUNNABLE &&

	srcu_read_unlock(&kvm->srcu, vcpu->srcu_idx);

	return r;
}

static inline int complete_emulated_io(struct kvm_vcpu *vcpu)