void kvm_lapic_sync_from_vapic(struct kvm_vcpu *vcpu)
{
	u32 data;
	void *vapic;

	if (test_bit(KVM_APIC_PV_EOI_PENDING, &vcpu->arch.apic_attention))
		apic_sync_pv_eoi_from_guest(vcpu, vcpu->arch.apic);

	if (!test_bit(KVM_APIC_CHECK_VAPIC, &vcpu->arch.apic_attention))
		return;

	vapic = kmap_atomic(vcpu->arch.apic->vapic_page);
	data = *(u32 *)(vapic + offset_in_page(vcpu->arch.apic->vapic_addr));
	kunmap_atomic(vapic);

	apic_set_tpr(vcpu->arch.apic, data & 0xff);
}

	u32 data, tpr;
	int max_irr, max_isr;
	struct kvm_lapic *apic = vcpu->arch.apic;
	void *vapic;

	apic_sync_pv_eoi_to_guest(vcpu, apic);

	if (!test_bit(KVM_APIC_CHECK_VAPIC, &vcpu->arch.apic_attention))
		max_isr = 0;
	data = (tpr & 0xff) | ((max_isr & 0xf0) << 8) | (max_irr << 24);

	vapic = kmap_atomic(vcpu->arch.apic->vapic_page);
	*(u32 *)(vapic + offset_in_page(vcpu->arch.apic->vapic_addr)) = data;
	kunmap_atomic(vapic);
}

void kvm_lapic_set_vapic_addr(struct kvm_vcpu *vcpu, gpa_t vapic_addr)
{
	vcpu->arch.apic->vapic_addr = vapic_addr;
	if (vapic_addr)
		__set_bit(KVM_APIC_CHECK_VAPIC, &vcpu->arch.apic_attention);
	else
		__clear_bit(KVM_APIC_CHECK_VAPIC, &vcpu->arch.apic_attention);
}

int kvm_x2apic_msr_write(struct kvm_vcpu *vcpu, u32 msr, u64 data)
{