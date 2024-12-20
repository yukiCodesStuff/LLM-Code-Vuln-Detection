	[_PAGE_CACHE_MODE_WT]		= _PAGE_PCD,
	[_PAGE_CACHE_MODE_WP]		= _PAGE_PCD,
};
EXPORT_SYMBOL(__cachemode2pte_tbl);
uint8_t __pte2cachemode_tbl[8] = {
	[__pte2cm_idx(0)] = _PAGE_CACHE_MODE_WB,
	[__pte2cm_idx(_PAGE_PWT)] = _PAGE_CACHE_MODE_WC,
	[__pte2cm_idx(_PAGE_PCD)] = _PAGE_CACHE_MODE_UC_MINUS,
	[__pte2cm_idx(_PAGE_PCD | _PAGE_PAT)] = _PAGE_CACHE_MODE_UC_MINUS,
	[__pte2cm_idx(_PAGE_PWT | _PAGE_PCD | _PAGE_PAT)] = _PAGE_CACHE_MODE_UC,
};
EXPORT_SYMBOL(__pte2cachemode_tbl);

static unsigned long __initdata pgt_buf_start;
static unsigned long __initdata pgt_buf_end;
static unsigned long __initdata pgt_buf_top;
static unsigned long __init get_new_step_size(unsigned long step_size)
{
	/*
	 * Initial mapped size is PMD_SIZE (2M).
	 * We can not set step_size to be PUD_SIZE (1G) yet.
	 * In worse case, when we cross the 1G boundary, and
	 * PG_LEVEL_2M is not set, we will need 1+1+512 pages (2M + 8k)
	 * to map 1G range with PTE. Hence we use one less than the
	 * difference of page table level shifts.
	 *
	 * Don't need to worry about overflow in the top-down case, on 32bit,
	 * when step_size is 0, round_down() returns 0 for start, and that
	 * turns it into 0x100000000ULL.
	 * In the bottom-up case, round_up(x, 0) returns 0 though too, which
	 * needs to be taken into consideration by the code below.
	 */
	return step_size << (PMD_SHIFT - PAGE_SHIFT - 1);
}

/**
 * memory_map_top_down - Map [map_start, map_end) top down
	unsigned long step_size;
	unsigned long addr;
	unsigned long mapped_ram_size = 0;

	/* xen has big range in reserved near end of ram, skip it at first.*/
	addr = memblock_find_in_range(map_start, map_end, PMD_SIZE, PMD_SIZE);
	real_end = addr + PMD_SIZE;
				start = map_start;
		} else
			start = map_start;
		mapped_ram_size += init_range_memory_mapping(start,
							last_start);
		last_start = start;
		min_pfn_mapped = last_start >> PAGE_SHIFT;
		if (mapped_ram_size >= step_size)
			step_size = get_new_step_size(step_size);
	}

	if (real_end < map_end)
		init_range_memory_mapping(real_end, map_end);
static void __init memory_map_bottom_up(unsigned long map_start,
					unsigned long map_end)
{
	unsigned long next, start;
	unsigned long mapped_ram_size = 0;
	/* step_size need to be small so pgt_buf from BRK could cover it */
	unsigned long step_size = PMD_SIZE;

	 * for page table.
	 */
	while (start < map_end) {
		if (step_size && map_end - start > step_size) {
			next = round_up(start + 1, step_size);
			if (next > map_end)
				next = map_end;
		} else {
			next = map_end;
		}

		mapped_ram_size += init_range_memory_mapping(start, next);
		start = next;

		if (mapped_ram_size >= step_size)
			step_size = get_new_step_size(step_size);
	}
}

void __init init_mem_mapping(void)