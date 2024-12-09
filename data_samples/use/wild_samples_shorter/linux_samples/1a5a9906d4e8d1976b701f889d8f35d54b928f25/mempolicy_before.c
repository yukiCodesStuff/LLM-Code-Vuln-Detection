	do {
		next = pmd_addr_end(addr, end);
		split_huge_page_pmd(vma->vm_mm, pmd);
		if (pmd_none_or_clear_bad(pmd))
			continue;
		if (check_pte_range(vma, pmd, addr, next, nodes,
				    flags, private))
			return -EIO;