

/* Free memory returned from module_alloc */
void module_free(struct module *mod, void *module_region)
{
	vfree(module_region);

	/* Globally flush the L1 icache. */
		     0, 0, 0, NULL, NULL, 0);

	/*
	 * FIXME: If module_region == mod->module_init, trim exception
	 * table entries.
	 */
}
