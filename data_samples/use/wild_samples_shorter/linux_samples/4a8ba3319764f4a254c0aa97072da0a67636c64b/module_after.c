

/* Free memory returned from module_alloc */
void module_memfree(void *module_region)
{
	vfree(module_region);

	/* Globally flush the L1 icache. */
		     0, 0, 0, NULL, NULL, 0);

	/*
	 * FIXME: Add module_arch_freeing_init to trim exception
	 * table entries.
	 */
}
