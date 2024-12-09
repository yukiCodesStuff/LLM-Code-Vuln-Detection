	if (map_vm_area(area, prot_rwx, pages)) {
		vunmap(area->addr);
		goto error;
	}

	return area->addr;

error:
	while (--i >= 0)
		__free_page(pages[i]);
	kfree(pages);
	return NULL;
}


/* Free memory returned from module_alloc */
void module_memfree(void *module_region)
{
	vfree(module_region);

	/* Globally flush the L1 icache. */
	flush_remote(0, HV_FLUSH_EVICT_L1I, cpu_online_mask,
		     0, 0, 0, NULL, NULL, 0);

	/*
	 * FIXME: Add module_arch_freeing_init to trim exception
	 * table entries.
	 */
}
{
	vfree(module_region);

	/* Globally flush the L1 icache. */
	flush_remote(0, HV_FLUSH_EVICT_L1I, cpu_online_mask,
		     0, 0, 0, NULL, NULL, 0);

	/*
	 * FIXME: Add module_arch_freeing_init to trim exception
	 * table entries.
	 */
}

#ifdef __tilegx__
/*
 * Validate that the high 16 bits of "value" is just the sign-extension of
 * the low 48 bits.
 */
static int validate_hw2_last(long value, struct module *me)
{
	if (((value << 16) >> 16) != value) {
		pr_warn("module %s: Out of range HW2_LAST value %#lx\n",
			me->name, value);
		return 0;
	}