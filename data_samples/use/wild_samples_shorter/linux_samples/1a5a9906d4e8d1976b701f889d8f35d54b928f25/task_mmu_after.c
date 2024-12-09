	} else {
		spin_unlock(&walk->mm->page_table_lock);
	}

	if (pmd_trans_unstable(pmd))
		return 0;
	/*
	 * The mmap_sem held all the way back in m_start() is what
	 * keeps khugepaged out of here and from collapsing things
	 * in here.
	struct page *page;

	split_huge_page_pmd(walk->mm, pmd);
	if (pmd_trans_unstable(pmd))
		return 0;

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
	int err = 0;

	split_huge_page_pmd(walk->mm, pmd);
	if (pmd_trans_unstable(pmd))
		return 0;

	/* find the first VMA at or above 'addr' */
	vma = find_vma(walk->mm, addr);
	for (; addr != end; addr += PAGE_SIZE) {
		spin_unlock(&walk->mm->page_table_lock);
	}

	if (pmd_trans_unstable(pmd))
		return 0;
	orig_pte = pte = pte_offset_map_lock(walk->mm, pmd, addr, &ptl);
	do {
		struct page *page = can_gather_numa_stats(*pte, md->vma, addr);
		if (!page)