{
	struct thread_struct *t = &current->thread;
	int idx;

	for (idx = 0; idx < GDT_ENTRY_TLS_ENTRIES; idx++)
		if (desc_empty(&t->tls_array[idx]))
			return idx + GDT_ENTRY_TLS_MIN;
	return -ESRCH;
}

static bool tls_desc_okay(const struct user_desc *info)
{
	if (LDT_empty(info))
		return true;

	/*
	 * espfix is required for 16-bit data segments, but espfix
	 * only works for LDT segments.
	 */
	if (!info->seg_32bit)
		return false;

	/* Only allow data segments in the TLS array. */
	if (info->contents > 1)
		return false;

	/*
	 * Non-present segments with DPL 3 present an interesting attack
	 * surface.  The kernel should handle such segments correctly,
	 * but TLS is very difficult to protect in a sandbox, so prevent
	 * such segments from being created.
	 *
	 * If userspace needs to remove a TLS entry, it can still delete
	 * it outright.
	 */
	if (info->seg_not_present)
		return false;

	return true;
}
{
	struct thread_struct *t = &p->thread;
	struct desc_struct *desc = &t->tls_array[idx - GDT_ENTRY_TLS_MIN];
	int cpu;

	/*
	 * We must not get preempted while modifying the TLS.
	 */
	cpu = get_cpu();

	while (n-- > 0) {
		if (LDT_empty(info))
			desc->a = desc->b = 0;
		else
			fill_ldt(desc, info);
		++info;
		++desc;
	}