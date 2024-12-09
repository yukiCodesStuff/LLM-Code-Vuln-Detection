{
	struct dma_pool *pool = &atomic_pool;
	pgprot_t prot = pgprot_dmacoherent(pgprot_kernel);
	unsigned long nr_pages = pool->size >> PAGE_SHIFT;
	unsigned long *bitmap;
	struct page *page;
	struct page **pages;
	void *ptr;
	int bitmap_size = BITS_TO_LONGS(nr_pages) * sizeof(long);

	bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!bitmap)
		goto no_bitmap;

	pages = kzalloc(nr_pages * sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		goto no_pages;

	if (IS_ENABLED(CONFIG_CMA))
		ptr = __alloc_from_contiguous(NULL, pool->size, prot, &page,
					      atomic_pool_init);
	else
		ptr = __alloc_remap_buffer(NULL, pool->size, GFP_KERNEL, prot,
					   &page, atomic_pool_init);
	if (ptr) {
		int i;

		for (i = 0; i < nr_pages; i++)
			pages[i] = page + i;

		spin_lock_init(&pool->lock);
		pool->vaddr = ptr;
		pool->pages = pages;
		pool->bitmap = bitmap;
		pool->nr_pages = nr_pages;
		pr_info("DMA: preallocated %u KiB pool for atomic coherent allocations\n",
		       (unsigned)pool->size / 1024);
		return 0;
	}
{
	struct dma_pool *pool = &atomic_pool;
	pgprot_t prot = pgprot_dmacoherent(pgprot_kernel);
	unsigned long nr_pages = pool->size >> PAGE_SHIFT;
	unsigned long *bitmap;
	struct page *page;
	struct page **pages;
	void *ptr;
	int bitmap_size = BITS_TO_LONGS(nr_pages) * sizeof(long);

	bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!bitmap)
		goto no_bitmap;

	pages = kzalloc(nr_pages * sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		goto no_pages;

	if (IS_ENABLED(CONFIG_CMA))
		ptr = __alloc_from_contiguous(NULL, pool->size, prot, &page,
					      atomic_pool_init);
	else
		ptr = __alloc_remap_buffer(NULL, pool->size, GFP_KERNEL, prot,
					   &page, atomic_pool_init);
	if (ptr) {
		int i;

		for (i = 0; i < nr_pages; i++)
			pages[i] = page + i;

		spin_lock_init(&pool->lock);
		pool->vaddr = ptr;
		pool->pages = pages;
		pool->bitmap = bitmap;
		pool->nr_pages = nr_pages;
		pr_info("DMA: preallocated %u KiB pool for atomic coherent allocations\n",
		       (unsigned)pool->size / 1024);
		return 0;
	}