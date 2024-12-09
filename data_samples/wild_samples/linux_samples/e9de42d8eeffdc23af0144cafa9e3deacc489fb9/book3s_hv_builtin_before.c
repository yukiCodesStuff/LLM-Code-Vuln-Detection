{
	struct page *page;
	struct kvm_rma_info *ri;

	ri = kmalloc(sizeof(struct kvm_rma_info), GFP_KERNEL);
	if (!ri)
		return NULL;
	page = cma_alloc(kvm_cma, kvm_rma_pages, get_order(kvm_rma_pages));
	if (!page)
		goto err_out;
	atomic_set(&ri->use_count, 1);
	ri->base_pfn = page_to_pfn(page);
	return ri;
err_out:
	kfree(ri);
	return NULL;
}
EXPORT_SYMBOL_GPL(kvm_alloc_rma);

void kvm_release_rma(struct kvm_rma_info *ri)
{
	if (atomic_dec_and_test(&ri->use_count)) {
		cma_release(kvm_cma, pfn_to_page(ri->base_pfn), kvm_rma_pages);
		kfree(ri);
	}
{
	unsigned long align_pages = HPT_ALIGN_PAGES;

	VM_BUG_ON(get_order(nr_pages) < KVM_CMA_CHUNK_ORDER - PAGE_SHIFT);

	/* Old CPUs require HPT aligned on a multiple of its size */
	if (!cpu_has_feature(CPU_FTR_ARCH_206))
		align_pages = nr_pages;
	return cma_alloc(kvm_cma, nr_pages, get_order(align_pages));
}
EXPORT_SYMBOL_GPL(kvm_alloc_hpt);

void kvm_release_hpt(struct page *page, unsigned long nr_pages)
{
	cma_release(kvm_cma, page, nr_pages);
}
EXPORT_SYMBOL_GPL(kvm_release_hpt);

/**
 * kvm_cma_reserve() - reserve area for kvm hash pagetable
 *
 * This function reserves memory from early allocator. It should be
 * called by arch specific code once the early allocator (memblock or bootmem)
 * has been activated and all other subsystems have already allocated/reserved
 * memory.
 */
void __init kvm_cma_reserve(void)
{
	unsigned long align_size;
	struct memblock_region *reg;
	phys_addr_t selected_size = 0;
	/*
	 * We cannot use memblock_phys_mem_size() here, because
	 * memblock_analyze() has not been called yet.
	 */
	for_each_memblock(memory, reg)
		selected_size += memblock_region_memory_end_pfn(reg) -
				 memblock_region_memory_base_pfn(reg);

	selected_size = (selected_size * kvm_cma_resv_ratio / 100) << PAGE_SHIFT;
	if (selected_size) {
		pr_debug("%s: reserving %ld MiB for global area\n", __func__,
			 (unsigned long)selected_size / SZ_1M);
		/*
		 * Old CPUs require HPT aligned on a multiple of its size. So for them
		 * make the alignment as max size we could request.
		 */
		if (!cpu_has_feature(CPU_FTR_ARCH_206))
			align_size = __rounddown_pow_of_two(selected_size);
		else
			align_size = HPT_ALIGN_PAGES << PAGE_SHIFT;

		align_size = max(kvm_rma_pages << PAGE_SHIFT, align_size);
		cma_declare_contiguous(0, selected_size, 0, align_size,
			KVM_CMA_CHUNK_ORDER - PAGE_SHIFT, false, &kvm_cma);
	}