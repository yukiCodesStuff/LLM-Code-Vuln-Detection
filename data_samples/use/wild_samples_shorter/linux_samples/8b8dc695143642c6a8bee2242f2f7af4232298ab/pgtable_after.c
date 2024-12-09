	pdshift = PMD_SHIFT;
	pmdp = pmd_offset(&pud, ea);
	pmd  = READ_ONCE(*pmdp);

	/*
	 * A hugepage collapse is captured by this condition, see
	 * pmdp_collapse_flush.
	 */
	if (pmd_none(pmd))
		return NULL;

#ifdef CONFIG_PPC_BOOK3S_64
	/*
	 * A hugepage split is captured by this condition, see
	 * pmdp_invalidate.
	 *
	 * Huge page modification can be caught here too.
	 */
	if (pmd_is_serializing(pmd))
		return NULL;
#endif

	if (pmd_trans_huge(pmd) || pmd_devmap(pmd)) {
		if (is_thp)
			*is_thp = true;
		ret_pte = (pte_t *)pmdp;