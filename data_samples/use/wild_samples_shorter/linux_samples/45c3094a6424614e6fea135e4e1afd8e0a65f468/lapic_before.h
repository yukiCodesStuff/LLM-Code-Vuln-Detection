	ldr >>= 32 - map->ldr_bits;
	cid = (ldr >> map->cid_shift) & map->cid_mask;

	BUG_ON(cid >= ARRAY_SIZE(map->logical_map));

	return cid;
}

static inline u16 apic_logical_id(struct kvm_apic_map *map, u32 ldr)