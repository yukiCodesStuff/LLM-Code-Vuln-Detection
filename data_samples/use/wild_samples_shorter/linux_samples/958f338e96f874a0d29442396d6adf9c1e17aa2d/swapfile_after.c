	return 0;
}


/*
 * Find out how many pages are allowed for a single swap device. There
 * are two limiting factors:
 * 1) the number of bits for the swap offset in the swp_entry_t type, and
 * 2) the number of bits in the swap pte, as defined by the different
 * architectures.
 *
 * In order to find the largest possible bit mask, a swap entry with
 * swap type 0 and swap offset ~0UL is created, encoded to a swap pte,
 * decoded to a swp_entry_t again, and finally the swap offset is
 * extracted.
 *
 * This will mask all the bits from the initial ~0UL mask that can't
 * be encoded in either the swp_entry_t or the architecture definition
 * of a swap pte.
 */
unsigned long generic_max_swapfile_size(void)
{
	return swp_offset(pte_to_swp_entry(
			swp_entry_to_pte(swp_entry(0, ~0UL)))) + 1;
}

/* Can be overridden by an architecture for additional checks. */
__weak unsigned long max_swapfile_size(void)
{
	return generic_max_swapfile_size();
}

static unsigned long read_swap_header(struct swap_info_struct *p,
					union swap_header *swap_header,
					struct inode *inode)
{
	p->cluster_next = 1;
	p->cluster_nr = 0;

	maxpages = max_swapfile_size();
	last_page = swap_header->info.last_page;
	if (!last_page) {
		pr_warn("Empty swap-file\n");
		return 0;