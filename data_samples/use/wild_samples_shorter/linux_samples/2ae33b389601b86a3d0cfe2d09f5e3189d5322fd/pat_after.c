	if (base > __pa(high_memory-1))
		return 0;

	/*
	 * some areas in the middle of the kernel identity range
	 * are not mapped, like the PCI space.
	 */
	if (!page_is_ram(base >> PAGE_SHIFT))
		return 0;

	id_sz = (__pa(high_memory-1) <= base + size) ?
				__pa(high_memory) - base :
				size;
