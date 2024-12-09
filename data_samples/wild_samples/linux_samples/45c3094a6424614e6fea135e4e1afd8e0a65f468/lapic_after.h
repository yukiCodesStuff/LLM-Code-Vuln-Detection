{
	u16 cid;
	ldr >>= 32 - map->ldr_bits;
	cid = (ldr >> map->cid_shift) & map->cid_mask;

	return cid;
}

static inline u16 apic_logical_id(struct kvm_apic_map *map, u32 ldr)
{
	ldr >>= (32 - map->ldr_bits);
	return ldr & map->lid_mask;
}

static inline bool kvm_apic_has_events(struct kvm_vcpu *vcpu)
{
	return vcpu->arch.apic->pending_events;
}

bool kvm_apic_pending_eoi(struct kvm_vcpu *vcpu, int vector);

#endif