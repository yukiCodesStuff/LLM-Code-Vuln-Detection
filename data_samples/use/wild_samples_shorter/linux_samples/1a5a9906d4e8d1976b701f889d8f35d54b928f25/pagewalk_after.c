			continue;

		split_huge_page_pmd(walk->mm, pmd);
		if (pmd_none_or_trans_huge_or_clear_bad(pmd))
			goto again;
		err = walk_pte_range(pmd, addr, next, walk);
		if (err)
			break;