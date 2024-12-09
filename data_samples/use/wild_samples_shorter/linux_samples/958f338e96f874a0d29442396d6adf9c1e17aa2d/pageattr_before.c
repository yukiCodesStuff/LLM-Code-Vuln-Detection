
		pmd = pmd_offset(pud, start);

		set_pmd(pmd, __pmd(cpa->pfn << PAGE_SHIFT | _PAGE_PSE |
				   massage_pgprot(pmd_pgprot)));

		start	  += PMD_SIZE;
		cpa->pfn  += PMD_SIZE >> PAGE_SHIFT;
		cur_pages += PMD_SIZE >> PAGE_SHIFT;
	 * Map everything starting from the Gb boundary, possibly with 1G pages
	 */
	while (boot_cpu_has(X86_FEATURE_GBPAGES) && end - start >= PUD_SIZE) {
		set_pud(pud, __pud(cpa->pfn << PAGE_SHIFT | _PAGE_PSE |
				   massage_pgprot(pud_pgprot)));

		start	  += PUD_SIZE;
		cpa->pfn  += PUD_SIZE >> PAGE_SHIFT;
		cur_pages += PUD_SIZE >> PAGE_SHIFT;