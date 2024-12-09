 */
static void notrace start_secondary(void *unused)
{
	/*
	 * Don't put *anything* except direct CPU state initialization
	 * before cpu_init(), SMP booting is too fragile that we want to
	 * limit the things done here to the most necessary things.
	 */
	if (boot_cpu_has(X86_FEATURE_PCID))
		__write_cr4(__read_cr4() | X86_CR4_PCIDE);

#ifdef CONFIG_X86_32
	/* switch away from the initial page table */
	load_cr3(swapper_pg_dir);