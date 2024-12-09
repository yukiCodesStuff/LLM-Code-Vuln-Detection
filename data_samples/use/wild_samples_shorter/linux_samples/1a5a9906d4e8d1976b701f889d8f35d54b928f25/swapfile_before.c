	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (unlikely(pmd_trans_huge(*pmd)))
			continue;
		if (pmd_none_or_clear_bad(pmd))
			continue;
		ret = unuse_pte_range(vma, pmd, addr, next, entry, page);
		if (ret)
			return ret;