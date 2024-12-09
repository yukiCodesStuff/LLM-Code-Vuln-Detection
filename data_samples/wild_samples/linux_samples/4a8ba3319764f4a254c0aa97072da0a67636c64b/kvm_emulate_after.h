{
	vcpu->arch.hcr_el2 = HCR_GUEST_FLAGS;
	if (test_bit(KVM_ARM_VCPU_EL1_32BIT, vcpu->arch.features))
		vcpu->arch.hcr_el2 &= ~HCR_RW;
}