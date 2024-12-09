{
	struct dma_pool *pool = &atomic_pool;
	pgprot_t prot = pgprot_dmacoherent(pgprot_kernel);
	gfp_t gfp = GFP_KERNEL | GFP_DMA;
	unsigned long nr_pages = pool->size >> PAGE_SHIFT;
	unsigned long *bitmap;
	struct page *page;
	struct page **pages;
		ptr = __alloc_from_contiguous(NULL, pool->size, prot, &page,
					      atomic_pool_init);
	else
		ptr = __alloc_remap_buffer(NULL, pool->size, gfp, prot, &page,
					   atomic_pool_init);
	if (ptr) {
		int i;

		for (i = 0; i < nr_pages; i++)