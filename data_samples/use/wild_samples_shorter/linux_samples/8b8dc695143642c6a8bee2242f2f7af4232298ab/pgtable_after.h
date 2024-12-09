	return false;
}

static inline int pmd_is_serializing(pmd_t pmd)
{
	/*
	 * If the pmd is undergoing a split, the _PAGE_PRESENT bit is clear
	 * and _PAGE_INVALID is set (see pmd_present, pmdp_invalidate).
	 *
	 * This condition may also occur when flushing a pmd while flushing
	 * it (see ptep_modify_prot_start), so callers must ensure this
	 * case is fine as well.
	 */
	if ((pmd_raw(pmd) & cpu_to_be64(_PAGE_PRESENT | _PAGE_INVALID)) ==
						cpu_to_be64(_PAGE_INVALID))
		return true;

	return false;
}

static inline int pmd_bad(pmd_t pmd)
{
	if (radix_enabled())
		return radix__pmd_bad(pmd);
#define pmd_access_permitted pmd_access_permitted
static inline bool pmd_access_permitted(pmd_t pmd, bool write)
{
	/*
	 * pmdp_invalidate sets this combination (which is not caught by
	 * !pte_present() check in pte_access_permitted), to prevent
	 * lock-free lookups, as part of the serialize_against_pte_lookup()
	 * synchronisation.
	 *
	 * This also catches the case where the PTE's hardware PRESENT bit is
	 * cleared while TLB is flushed, which is suboptimal but should not
	 * be frequent.
	 */
	if (pmd_is_serializing(pmd))
		return false;

	return pte_access_permitted(pmd_pte(pmd), write);
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE