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
			new->broadcast = X2APIC_BROADCAST;
		} else if (kvm_apic_get_reg(apic, APIC_LDR)) {
			if (kvm_apic_get_reg(apic, APIC_DFR) ==
							APIC_DFR_CLUSTER) {
		dst = &map->phys_map[irq->dest_id];
	} else {
		u32 mda = irq->dest_id << (32 - map->ldr_bits);
		u16 cid = apic_cluster_id(map, mda);

		if (cid >= ARRAY_SIZE(map->logical_map))
			goto out;

		dst = map->logical_map[cid];

		bitmap = apic_logical_id(map, mda);

		if (irq->delivery_mode == APIC_DM_LOWEST) {