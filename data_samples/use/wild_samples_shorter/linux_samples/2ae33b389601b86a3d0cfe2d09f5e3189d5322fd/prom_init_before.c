{
}
#else
static void __reloc_toc(void *tocstart, unsigned long offset,
			unsigned long nr_entries)
{
	unsigned long i;
	unsigned long *toc_entry = (unsigned long *)tocstart;

	for (i = 0; i < nr_entries; i++) {
		*toc_entry = *toc_entry + offset;
		toc_entry++;
	unsigned long nr_entries =
		(__prom_init_toc_end - __prom_init_toc_start) / sizeof(long);

	/* Need to add offset to get at __prom_init_toc_start */
	__reloc_toc(__prom_init_toc_start + offset, offset, nr_entries);

	mb();
}


	mb();

	/* __prom_init_toc_start has been relocated, no need to add offset */
	__reloc_toc(__prom_init_toc_start, -offset, nr_entries);
}
#endif
#endif
