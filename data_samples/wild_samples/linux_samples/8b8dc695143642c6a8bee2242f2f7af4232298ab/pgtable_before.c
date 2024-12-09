	if (is_hugepd(__hugepd(pud_val(pud)))) {
		hpdp = (hugepd_t *)&pud;
		goto out_huge;
	}
	pdshift = PMD_SHIFT;
	pmdp = pmd_offset(&pud, ea);
	pmd  = READ_ONCE(*pmdp);
	/*
	 * A hugepage collapse is captured by pmd_none, because
	 * it mark the pmd none and do a hpte invalidate.
	 */
	if (pmd_none(pmd))
		return NULL;

	if (pmd_trans_huge(pmd) || pmd_devmap(pmd)) {
		if (is_thp)
			*is_thp = true;
		ret_pte = (pte_t *)pmdp;
		goto out;
	}