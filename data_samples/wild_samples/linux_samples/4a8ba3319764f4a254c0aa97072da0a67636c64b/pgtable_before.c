{
	struct page *page;
	unsigned long offset;

	offset = (unsigned long) entry / sizeof(unsigned long);
	offset = (offset & (PTRS_PER_PMD - 1)) * PMD_SIZE;
	page = pmd_to_page((pmd_t *) entry);
	return page->index + offset;
}