}
EXPORT_SYMBOL_GPL(gfn_to_page);

static int __kvm_map_gfn(struct kvm_memory_slot *slot, gfn_t gfn,
			 struct kvm_host_map *map)
{
	kvm_pfn_t pfn;
	void *hva = NULL;
	struct page *page = KVM_UNMAPPED_PAGE;

	if (!map)
		return -EINVAL;

	pfn = gfn_to_pfn_memslot(slot, gfn);
	if (is_error_noslot_pfn(pfn))
		return -EINVAL;

	if (pfn_valid(pfn)) {
		page = pfn_to_page(pfn);
		hva = kmap(page);
#ifdef CONFIG_HAS_IOMEM
	} else {
		hva = memremap(pfn_to_hpa(pfn), PAGE_SIZE, MEMREMAP_WB);
#endif
	}

	if (!hva)
	return 0;
}

int kvm_vcpu_map(struct kvm_vcpu *vcpu, gfn_t gfn, struct kvm_host_map *map)
{
	return __kvm_map_gfn(kvm_vcpu_gfn_to_memslot(vcpu, gfn), gfn, map);
}
EXPORT_SYMBOL_GPL(kvm_vcpu_map);

void kvm_vcpu_unmap(struct kvm_vcpu *vcpu, struct kvm_host_map *map,
		    bool dirty)
{
	if (!map)
		return;

	if (!map->hva)
		return;

	if (map->page != KVM_UNMAPPED_PAGE)
		kunmap(map->page);
#ifdef CONFIG_HAS_IOMEM
	else
		memunmap(map->hva);
#endif

	if (dirty) {
		kvm_vcpu_mark_page_dirty(vcpu, map->gfn);
		kvm_release_pfn_dirty(map->pfn);
	} else {
		kvm_release_pfn_clean(map->pfn);
	}

	map->hva = NULL;
	map->page = NULL;
}
EXPORT_SYMBOL_GPL(kvm_vcpu_unmap);

struct page *kvm_vcpu_gfn_to_page(struct kvm_vcpu *vcpu, gfn_t gfn)
{