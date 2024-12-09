#define __pte_to_swp_entry(pte)		((swp_entry_t) { (pte).pte_low })
#define __swp_entry_to_pte(x)		((pte_t) { .pte = (x).val })

/* No inverted PFNs on 2 level page tables */

static inline u64 protnone_mask(u64 val)
{
	return 0;
}

static inline u64 flip_protnone_guard(u64 oldval, u64 val, u64 mask)
{
	return val;
}

static inline bool __pte_needs_invert(u64 val)
{
	return false;
}

#endif /* _ASM_X86_PGTABLE_2LEVEL_H */