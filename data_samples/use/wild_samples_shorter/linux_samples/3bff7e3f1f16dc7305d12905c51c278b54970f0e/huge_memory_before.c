	page = pmd_page(orig_pmd);
	VM_BUG_ON_PAGE(!PageHead(page), page);

	/* Lock page for reuse_swap_page() */
	if (!trylock_page(page)) {
		get_page(page);
		spin_unlock(vmf->ptl);
		lock_page(page);
	}

	/*
	 * We can only reuse the page if nobody else maps the huge page or it's
	 * part.
	 */
	if (reuse_swap_page(page)) {
		pmd_t entry;
		entry = pmd_mkyoung(orig_pmd);
		entry = maybe_pmd_mkwrite(pmd_mkdirty(entry), vma);
		if (pmdp_set_access_flags(vma, haddr, vmf->pmd, entry, 1))
		return VM_FAULT_WRITE;
	}

	unlock_page(page);
	spin_unlock(vmf->ptl);
fallback:
	__split_huge_pmd(vma, vmf->pmd, vmf->address, false, NULL);