{
	/* Switch to the identity mapping. */
	cpu_switch_mm(idmap_pgd, &init_mm);

#ifdef CONFIG_CPU_HAS_ASID
	/*
	 * We don't have a clean ASID for the identity mapping, which