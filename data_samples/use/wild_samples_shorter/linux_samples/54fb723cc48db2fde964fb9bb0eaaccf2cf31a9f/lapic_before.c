	return (kvm_apic_get_reg(apic, APIC_ID) >> 24) & 0xff;
}

static void recalculate_apic_map(struct kvm *kvm)
{
	struct kvm_apic_map *new, *old = NULL;
	struct kvm_vcpu *vcpu;
		if (apic_x2apic_mode(apic)) {
			new->ldr_bits = 32;
			new->cid_shift = 16;
			new->cid_mask = new->lid_mask = 0xffff;
		} else if (kvm_apic_sw_enabled(apic) &&
				!new->cid_mask /* flat mode */ &&
				kvm_apic_get_reg(apic, APIC_DFR) == APIC_DFR_CLUSTER) {
			new->cid_shift = 4;
	ASSERT(apic != NULL);

	/* if initial count is 0, current count should also be 0 */
	if (kvm_apic_get_reg(apic, APIC_TMICT) == 0)
		return 0;

	remaining = hrtimer_get_remaining(&apic->lapic_timer.timer);
	if (ktime_to_ns(remaining) < 0)
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