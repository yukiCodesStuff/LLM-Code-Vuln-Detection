unsigned long __ref xen_chk_extra_mem(unsigned long pfn)
{
	int i;
	phys_addr_t addr = PFN_PHYS(pfn);

	for (i = 0; i < XEN_EXTRA_MEM_MAX_REGIONS; i++) {
		if (addr >= xen_extra_mem[i].start &&
		    addr < xen_extra_mem[i].start + xen_extra_mem[i].size)
	int i;

	for (i = 0; i < XEN_EXTRA_MEM_MAX_REGIONS; i++) {
		if (!xen_extra_mem[i].size)
			continue;
		pfn_s = PFN_DOWN(xen_extra_mem[i].start);
		pfn_e = PFN_UP(xen_extra_mem[i].start + xen_extra_mem[i].size);
		for (pfn = pfn_s; pfn < pfn_e; pfn++)
			set_phys_to_machine(pfn, INVALID_P2M_ENTRY);
 * as a fallback if the remapping fails.
 */
static void __init xen_set_identity_and_release_chunk(unsigned long start_pfn,
	unsigned long end_pfn, unsigned long nr_pages, unsigned long *released)
{
	unsigned long pfn, end;
	int ret;

	WARN_ON(start_pfn > end_pfn);

	/* Release pages first. */
	end = min(end_pfn, nr_pages);
	for (pfn = start_pfn; pfn < end; pfn++) {
		unsigned long mfn = pfn_to_mfn(pfn);

		WARN(ret != 1, "Failed to release pfn %lx err=%d\n", pfn, ret);

		if (ret == 1) {
			(*released)++;
			if (!__set_phys_to_machine(pfn, INVALID_P2M_ENTRY))
				break;
		} else
			break;
	}

	set_phys_range_identity(start_pfn, end_pfn);
}

/*
 * Helper function to update the p2m and m2p tables and kernel mapping.
	}

	/* Update kernel mapping, but not for highmem. */
	if (pfn >= PFN_UP(__pa(high_memory - 1)))
		return;

	if (HYPERVISOR_update_va_mapping((unsigned long)__va(pfn << PAGE_SHIFT),
					 mfn_pte(mfn, PAGE_KERNEL), 0)) {
	unsigned long ident_pfn_iter, remap_pfn_iter;
	unsigned long ident_end_pfn = start_pfn + size;
	unsigned long left = size;
	unsigned int i, chunk;

	WARN_ON(size == 0);

		xen_remap_mfn = mfn;

		/* Set identity map */
		set_phys_range_identity(ident_pfn_iter, ident_pfn_iter + chunk);

		left -= chunk;
	}

static unsigned long __init xen_set_identity_and_remap_chunk(
        const struct e820entry *list, size_t map_size, unsigned long start_pfn,
	unsigned long end_pfn, unsigned long nr_pages, unsigned long remap_pfn,
	unsigned long *released, unsigned long *remapped)
{
	unsigned long pfn;
	unsigned long i = 0;
	unsigned long n = end_pfn - start_pfn;
		/* Do not remap pages beyond the current allocation */
		if (cur_pfn >= nr_pages) {
			/* Identity map remaining pages */
			set_phys_range_identity(cur_pfn, cur_pfn + size);
			break;
		}
		if (cur_pfn + size > nr_pages)
			size = nr_pages - cur_pfn;
		if (!remap_range_size) {
			pr_warning("Unable to find available pfn range, not remapping identity pages\n");
			xen_set_identity_and_release_chunk(cur_pfn,
				cur_pfn + left, nr_pages, released);
			break;
		}
		/* Adjust size to fit in current e820 RAM region */
		if (size > remap_range_size)
		/* Update variables to reflect new mappings. */
		i += size;
		remap_pfn += size;
		*remapped += size;
	}

	/*
	 * If the PFNs are currently mapped, the VA mapping also needs

static void __init xen_set_identity_and_remap(
	const struct e820entry *list, size_t map_size, unsigned long nr_pages,
	unsigned long *released, unsigned long *remapped)
{
	phys_addr_t start = 0;
	unsigned long last_pfn = nr_pages;
	const struct e820entry *entry;
	unsigned long num_released = 0;
	unsigned long num_remapped = 0;
	int i;

	/*
	 * Combine non-RAM regions and gaps until a RAM region (or the
				last_pfn = xen_set_identity_and_remap_chunk(
						list, map_size, start_pfn,
						end_pfn, nr_pages, last_pfn,
						&num_released, &num_remapped);
			start = end;
		}
	}

	*released = num_released;
	*remapped = num_remapped;

	pr_info("Released %ld page(s)\n", num_released);
}

/*
	struct xen_memory_map memmap;
	unsigned long max_pages;
	unsigned long extra_pages = 0;
	unsigned long remapped_pages;
	int i;
	int op;

	max_pfn = min(MAX_DOMAIN_PAGES, max_pfn);
	 * underlying RAM.
	 */
	xen_set_identity_and_remap(map, memmap.nr_entries, max_pfn,
				   &xen_released_pages, &remapped_pages);

	extra_pages += xen_released_pages;
	extra_pages += remapped_pages;

	/*
	 * Clamp the amount of extra memory to a EXTRA_MEM_RATIO
	 * factor the base size.  On non-highmem systems, the base