	ri = kmalloc(sizeof(struct kvm_rma_info), GFP_KERNEL);
	if (!ri)
		return NULL;
	page = cma_alloc(kvm_cma, kvm_rma_pages, order_base_2(kvm_rma_pages));
	if (!page)
		goto err_out;
	atomic_set(&ri->use_count, 1);
	ri->base_pfn = page_to_pfn(page);
{
	unsigned long align_pages = HPT_ALIGN_PAGES;

	VM_BUG_ON(order_base_2(nr_pages) < KVM_CMA_CHUNK_ORDER - PAGE_SHIFT);

	/* Old CPUs require HPT aligned on a multiple of its size */
	if (!cpu_has_feature(CPU_FTR_ARCH_206))
		align_pages = nr_pages;
	return cma_alloc(kvm_cma, nr_pages, order_base_2(align_pages));
}
EXPORT_SYMBOL_GPL(kvm_alloc_hpt);

void kvm_release_hpt(struct page *page, unsigned long nr_pages)