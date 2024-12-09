	int idx = segment >> 3;

	if ((segment & SEGMENT_TI_MASK) == SEGMENT_LDT) {
		struct ldt_struct *ldt;

		if (idx > LDT_ENTRIES)
			return 0;

		/* IRQs are off, so this synchronizes with smp_store_release */
		ldt = lockless_dereference(current->active_mm->context.ldt);
		if (!ldt || idx > ldt->size)
			return 0;

		desc = &ldt->entries[idx];
	} else {
		if (idx > GDT_ENTRIES)
			return 0;

		desc = raw_cpu_ptr(gdt_page.gdt) + idx;
	}

	return get_desc_base(desc);
}

#ifdef CONFIG_COMPAT
