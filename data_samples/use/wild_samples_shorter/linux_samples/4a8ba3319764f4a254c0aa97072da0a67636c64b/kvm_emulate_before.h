static inline void vcpu_reset_hcr(struct kvm_vcpu *vcpu)
{
	vcpu->arch.hcr_el2 = HCR_GUEST_FLAGS;
}

static inline unsigned long *vcpu_pc(const struct kvm_vcpu *vcpu)
{