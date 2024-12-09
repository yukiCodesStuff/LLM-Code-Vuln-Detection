{
	struct desc_struct *desc;
	int idx = segment >> 3;

	if ((segment & SEGMENT_TI_MASK) == SEGMENT_LDT) {
		if (idx > LDT_ENTRIES)
			return 0;

		if (idx > current->active_mm->context.size)
			return 0;

		desc = current->active_mm->context.ldt;
	} else {
		if (idx > GDT_ENTRIES)
			return 0;

		desc = raw_cpu_ptr(gdt_page.gdt);
	}

	return get_desc_base(desc + idx);
}