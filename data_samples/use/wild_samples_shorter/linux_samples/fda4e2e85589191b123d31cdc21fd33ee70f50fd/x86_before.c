		r = -EFAULT;
		if (copy_from_user(&va, argp, sizeof va))
			goto out;
		r = 0;
		kvm_lapic_set_vapic_addr(vcpu, va.vapic_addr);
		break;
	}
	case KVM_X86_SETUP_MCE: {
		u64 mcg_cap;
			!kvm_event_needs_reinjection(vcpu);
}

static int vapic_enter(struct kvm_vcpu *vcpu)
{
	struct kvm_lapic *apic = vcpu->arch.apic;
	struct page *page;

	if (!apic || !apic->vapic_addr)
		return 0;

	page = gfn_to_page(vcpu->kvm, apic->vapic_addr >> PAGE_SHIFT);
	if (is_error_page(page))
		return -EFAULT;

	vcpu->arch.apic->vapic_page = page;
	return 0;
}

static void vapic_exit(struct kvm_vcpu *vcpu)
{
	struct kvm_lapic *apic = vcpu->arch.apic;
	int idx;

	if (!apic || !apic->vapic_addr)
		return;

	idx = srcu_read_lock(&vcpu->kvm->srcu);
	kvm_release_page_dirty(apic->vapic_page);
	mark_page_dirty(vcpu->kvm, apic->vapic_addr >> PAGE_SHIFT);
	srcu_read_unlock(&vcpu->kvm->srcu, idx);
}

static void update_cr8_intercept(struct kvm_vcpu *vcpu)
{
	int max_irr, tpr;

	struct kvm *kvm = vcpu->kvm;

	vcpu->srcu_idx = srcu_read_lock(&kvm->srcu);
	r = vapic_enter(vcpu);
	if (r) {
		srcu_read_unlock(&kvm->srcu, vcpu->srcu_idx);
		return r;
	}

	r = 1;
	while (r > 0) {
		if (vcpu->arch.mp_state == KVM_MP_STATE_RUNNABLE &&

	srcu_read_unlock(&kvm->srcu, vcpu->srcu_idx);

	vapic_exit(vcpu);

	return r;
}

static inline int complete_emulated_io(struct kvm_vcpu *vcpu)