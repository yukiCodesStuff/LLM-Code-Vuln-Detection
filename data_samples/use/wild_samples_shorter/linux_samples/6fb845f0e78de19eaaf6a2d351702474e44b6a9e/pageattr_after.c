
#endif

/*
 * See set_mce_nospec().
 *
 * Machine check recovery code needs to change cache mode of poisoned pages to
 * UC to avoid speculative access logging another error. But passing the
 * address of the 1:1 mapping to set_memory_uc() is a fine way to encourage a
 * speculative access. So we cheat and flip the top bit of the address. This
 * works fine for the code that updates the page tables. But at the end of the
 * process we need to flush the TLB and cache and the non-canonical address
 * causes a #GP fault when used by the INVLPG and CLFLUSH instructions.
 *
 * But in the common case we already have a canonical address. This code
 * will fix the top bit if needed and is a no-op otherwise.
 */
static inline unsigned long fix_addr(unsigned long addr)
{
#ifdef CONFIG_X86_64
	return (long)(addr << 1) >> 1;
#else
	return addr;
#endif
}

static unsigned long __cpa_addr(struct cpa_data *cpa, unsigned long idx)
{
	if (cpa->flags & CPA_PAGES_ARRAY) {
		struct page *page = cpa->pages[idx];
	unsigned int i;

	for (i = 0; i < cpa->numpages; i++)
		__flush_tlb_one_kernel(fix_addr(__cpa_addr(cpa, i)));
}

static void cpa_flush(struct cpa_data *data, int cache)
{
		 * Only flush present addresses:
		 */
		if (pte && (pte_val(*pte) & _PAGE_PRESENT))
			clflush_cache_range_opt((void *)fix_addr(addr), PAGE_SIZE);
	}
	mb();
}

	return ret;
}

static int change_page_attr_set_clr(unsigned long *addr, int numpages,
				    pgprot_t mask_set, pgprot_t mask_clr,
				    int force_split, int in_flag,
				    struct page **pages)