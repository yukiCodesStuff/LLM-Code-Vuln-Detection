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

static void kvm_cache_gfn_to_pfn(struct kvm_memory_slot *slot, gfn_t gfn,
				 struct gfn_to_pfn_cache *cache, u64 gen)
{
	kvm_release_pfn(cache->pfn, cache->dirty, cache);

	cache->pfn = gfn_to_pfn_memslot(slot, gfn);
	cache->gfn = gfn;
	cache->dirty = false;
	cache->generation = gen;
}

static int __kvm_map_gfn(struct kvm_memslots *slots, gfn_t gfn,
			 struct kvm_host_map *map,
			 struct gfn_to_pfn_cache *cache,
			 bool atomic)
{
	kvm_pfn_t pfn;
	void *hva = NULL;
	struct page *page = KVM_UNMAPPED_PAGE;
	struct kvm_memory_slot *slot = __gfn_to_memslot(slots, gfn);
	u64 gen = slots->generation;

	if (!map)
		return -EINVAL;

	if (cache) {
		if (!cache->pfn || cache->gfn != gfn ||
			cache->generation != gen) {
			if (atomic)
				return -EAGAIN;
			kvm_cache_gfn_to_pfn(slot, gfn, cache, gen);
		}
		pfn = cache->pfn;
	} else {
		if (atomic)
			return -EAGAIN;
		pfn = gfn_to_pfn_memslot(slot, gfn);
	}
	if (is_error_noslot_pfn(pfn))
		return -EINVAL;

	if (pfn_valid(pfn)) {
		page = pfn_to_page(pfn);
		if (atomic)
			hva = kmap_atomic(page);
		else
			hva = kmap(page);
#ifdef CONFIG_HAS_IOMEM
	} else if (!atomic) {
		hva = memremap(pfn_to_hpa(pfn), PAGE_SIZE, MEMREMAP_WB);
	} else {
		return -EINVAL;
#endif
	}

	if (!hva)
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
#ifdef CONFIG_HAS_IOMEM
	else if (!atomic)
		memunmap(map->hva);
	else
		WARN_ONCE(1, "Unexpected unmapping in atomic context");
#endif

	if (dirty)
		mark_page_dirty_in_slot(memslot, map->gfn);

	if (cache)
		cache->dirty |= dirty;
	else
		kvm_release_pfn(map->pfn, dirty, NULL);

	map->hva = NULL;
	map->page = NULL;
}

int kvm_unmap_gfn(struct kvm_vcpu *vcpu, struct kvm_host_map *map, 
		  struct gfn_to_pfn_cache *cache, bool dirty, bool atomic)
{
	__kvm_unmap_gfn(gfn_to_memslot(vcpu->kvm, map->gfn), map,
			cache, dirty, atomic);
	return 0;
}
EXPORT_SYMBOL_GPL(kvm_unmap_gfn);

void kvm_vcpu_unmap(struct kvm_vcpu *vcpu, struct kvm_host_map *map, bool dirty)
{
	__kvm_unmap_gfn(kvm_vcpu_gfn_to_memslot(vcpu, map->gfn), map, NULL,
			dirty, false);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_unmap);

struct page *kvm_vcpu_gfn_to_page(struct kvm_vcpu *vcpu, gfn_t gfn)