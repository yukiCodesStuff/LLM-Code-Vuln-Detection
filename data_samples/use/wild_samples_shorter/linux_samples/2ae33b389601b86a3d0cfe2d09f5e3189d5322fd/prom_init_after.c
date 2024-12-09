{
}
#else
static void __reloc_toc(unsigned long offset, unsigned long nr_entries)
{
	unsigned long i;
	unsigned long *toc_entry;

	/* Get the start of the TOC by using r2 directly. */
	asm volatile("addi %0,2,-0x8000" : "=b" (toc_entry));

	for (i = 0; i < nr_entries; i++) {
		*toc_entry = *toc_entry + offset;
		toc_entry++;
	unsigned long nr_entries =
		(__prom_init_toc_end - __prom_init_toc_start) / sizeof(long);

	__reloc_toc(offset, nr_entries);

	mb();
}


	mb();

	__reloc_toc(-offset, nr_entries);
}
#endif
#endif
