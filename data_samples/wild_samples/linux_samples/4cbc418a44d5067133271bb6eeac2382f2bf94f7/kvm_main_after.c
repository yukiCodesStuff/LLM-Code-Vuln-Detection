{
	kvm_pfn_t pfn;

	pfn = gfn_to_pfn(kvm, gfn);

	return kvm_pfn_to_page(pfn);
}
EXPORT_SYMBOL_GPL(gfn_to_page);

void kvm_release_pfn(kvm_pfn_t pfn, bool dirty, struct gfn_to_pfn_cache *cache)
{
	if (pfn == 0)
		return;

	if (cache)
		cache->pfn = cache->gfn = 0;

	if (dirty)
		kvm_release_pfn_dirty(pfn);
	else
		kvm_release_pfn_clean(pfn);
}
	} else {
		return -EINVAL;
#endif
	}

	if (!hva)
		return -EFAULT;

	map->page = page;
	map->hva = hva;
	map->pfn = pfn;
	map->gfn = gfn;

	return 0;
}

int kvm_map_gfn(struct kvm_vcpu *vcpu, gfn_t gfn, struct kvm_host_map *map,
		struct gfn_to_pfn_cache *cache, bool atomic)
{
	return __kvm_map_gfn(kvm_memslots(vcpu->kvm), gfn, map,
			cache, atomic);
}
EXPORT_SYMBOL_GPL(kvm_map_gfn);

int kvm_vcpu_map(struct kvm_vcpu *vcpu, gfn_t gfn, struct kvm_host_map *map)
{
	return __kvm_map_gfn(kvm_vcpu_memslots(vcpu), gfn, map,
		NULL, false);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_map);

static void __kvm_unmap_gfn(struct kvm_memory_slot *memslot,
			struct kvm_host_map *map,
			struct gfn_to_pfn_cache *cache,
			bool dirty, bool atomic)
{
	if (!map)
		return;

	if (!map->hva)
		return;

	if (map->page != KVM_UNMAPPED_PAGE) {
		if (atomic)
			kunmap_atomic(map->hva);
		else
			kunmap(map->page);
	}