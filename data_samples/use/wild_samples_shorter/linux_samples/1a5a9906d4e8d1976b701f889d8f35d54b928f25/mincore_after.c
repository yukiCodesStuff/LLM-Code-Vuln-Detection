			}
			/* fall through */
		}
		if (pmd_none_or_trans_huge_or_clear_bad(pmd))
			mincore_unmapped_range(vma, addr, next, vec);
		else
			mincore_pte_range(vma, pmd, addr, next, vec);
		vec += (next - addr) >> PAGE_SHIFT;