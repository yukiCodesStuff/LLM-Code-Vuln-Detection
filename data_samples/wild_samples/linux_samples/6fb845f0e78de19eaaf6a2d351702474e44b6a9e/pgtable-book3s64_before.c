{
	/*
	 * Hash maps the memory with one size mmu_linear_psize.
	 * So don't bother to print these on hash
	 */
	if (!radix_enabled())
		return;
	seq_printf(m, "DirectMap4k:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_4K]) << 2);
	seq_printf(m, "DirectMap64k:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_64K]) << 6);
	seq_printf(m, "DirectMap2M:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_2M]) << 11);
	seq_printf(m, "DirectMap1G:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_1G]) << 20);
}
#endif /* CONFIG_PROC_FS */