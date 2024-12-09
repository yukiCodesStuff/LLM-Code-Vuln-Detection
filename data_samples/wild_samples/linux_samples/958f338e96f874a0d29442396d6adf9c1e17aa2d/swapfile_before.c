	} else if (S_ISREG(inode->i_mode)) {
		p->bdev = inode->i_sb->s_bdev;
		inode_lock(inode);
		if (IS_SWAPFILE(inode))
			return -EBUSY;
	} else
		return -EINVAL;

	return 0;
}

static unsigned long read_swap_header(struct swap_info_struct *p,
					union swap_header *swap_header,
					struct inode *inode)
{
	int i;
	unsigned long maxpages;
	unsigned long swapfilepages;
	unsigned long last_page;

	if (memcmp("SWAPSPACE2", swap_header->magic.magic, 10)) {
		pr_err("Unable to find swap-space signature\n");
		return 0;
	}
	if (swap_header->info.version != 1) {
		pr_warn("Unable to handle swap header version %d\n",
			swap_header->info.version);
		return 0;
	}

	p->lowest_bit  = 1;
	p->cluster_next = 1;
	p->cluster_nr = 0;

	/*
	 * Find out how many pages are allowed for a single swap
	 * device. There are two limiting factors: 1) the number
	 * of bits for the swap offset in the swp_entry_t type, and
	 * 2) the number of bits in the swap pte as defined by the
	 * different architectures. In order to find the
	 * largest possible bit mask, a swap entry with swap type 0
	 * and swap offset ~0UL is created, encoded to a swap pte,
	 * decoded to a swp_entry_t again, and finally the swap
	 * offset is extracted. This will mask all the bits from
	 * the initial ~0UL mask that can't be encoded in either
	 * the swp_entry_t or the architecture definition of a
	 * swap pte.
	 */
	maxpages = swp_offset(pte_to_swp_entry(
			swp_entry_to_pte(swp_entry(0, ~0UL)))) + 1;
	last_page = swap_header->info.last_page;
	if (!last_page) {
		pr_warn("Empty swap-file\n");
		return 0;
	}