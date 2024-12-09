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