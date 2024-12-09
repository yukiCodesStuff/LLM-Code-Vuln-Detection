	do {
		next = pmd_addr_end(addr, end);
		if (pmd_trans_huge(*pmd)) {
			if (next-addr != HPAGE_PMD_SIZE) {
				VM_BUG_ON(!rwsem_is_locked(&tlb->mm->mmap_sem));
				split_huge_page_pmd(vma->vm_mm, pmd);
			} else if (zap_huge_pmd(tlb, vma, pmd, addr))
				continue;
			/* fall through */
		}
		if (pmd_none_or_clear_bad(pmd))
			continue;
		next = zap_pte_range(tlb, vma, pmd, addr, next, details);
		cond_resched();
	} while (pmd++, addr = next, addr != end);

	return addr;
}