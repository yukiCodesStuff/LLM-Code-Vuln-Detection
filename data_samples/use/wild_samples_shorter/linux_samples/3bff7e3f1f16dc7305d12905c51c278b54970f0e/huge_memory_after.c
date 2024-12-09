	page = pmd_page(orig_pmd);
	VM_BUG_ON_PAGE(!PageHead(page), page);

	if (!trylock_page(page)) {
		get_page(page);
		spin_unlock(vmf->ptl);
		lock_page(page);
	}

	/*
	 * See do_wp_page(): we can only map the page writable if there are
	 * no additional references. Note that we always drain the LRU
	 * pagevecs immediately after adding a THP.
	 */
	if (page_count(page) > 1 + PageSwapCache(page) * thp_nr_pages(page))
		goto unlock_fallback;
	if (PageSwapCache(page))
		try_to_free_swap(page);
	if (page_count(page) == 1) {
		pmd_t entry;
		entry = pmd_mkyoung(orig_pmd);
		entry = maybe_pmd_mkwrite(pmd_mkdirty(entry), vma);
		if (pmdp_set_access_flags(vma, haddr, vmf->pmd, entry, 1))
		return VM_FAULT_WRITE;
	}

unlock_fallback:
	unlock_page(page);
	spin_unlock(vmf->ptl);
fallback:
	__split_huge_pmd(vma, vmf->pmd, vmf->address, false, NULL);