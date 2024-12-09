	spinlock_t *ptl;

	split_huge_page_pmd(walk->mm, pmd);

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE)
		if (is_target_pte_for_mc(vma, addr, *pte, NULL))
	spinlock_t *ptl;

	split_huge_page_pmd(walk->mm, pmd);
retry:
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; addr += PAGE_SIZE) {
		pte_t ptent = *(pte++);