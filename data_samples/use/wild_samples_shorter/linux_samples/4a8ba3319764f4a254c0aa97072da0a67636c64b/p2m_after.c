	return (void *)__get_free_page(GFP_KERNEL | __GFP_REPEAT);
}

static void __ref free_p2m_page(void *p)
{
	if (unlikely(!slab_is_available())) {
		free_bootmem((unsigned long)p, PAGE_SIZE);
		return;
	}

	free_page((unsigned long)p);
}

/*
			p2m_missing_pte : p2m_identity_pte;
		for (i = 0; i < PMDS_PER_MID_PAGE; i++) {
			pmdp = populate_extra_pmd(
				(unsigned long)(p2m + pfn) + i * PMD_SIZE);
			set_pmd(pmdp, __pmd(__pa(ptep) | _KERNPG_TABLE));
		}
	}
}
 * a new pmd is to replace p2m_missing_pte or p2m_identity_pte by a individual
 * pmd. In case of PAE/x86-32 there are multiple pmds to allocate!
 */
static pte_t *alloc_p2m_pmd(unsigned long addr, pte_t *pte_pg)
{
	pte_t *ptechk;
	pte_t *pte_newpg[PMDS_PER_MID_PAGE];
	pmd_t *pmdp;
	unsigned int level;
	unsigned long flags;
		if (ptechk == pte_pg) {
			set_pmd(pmdp,
				__pmd(__pa(pte_newpg[i]) | _KERNPG_TABLE));
			pte_newpg[i] = NULL;
		}

		spin_unlock_irqrestore(&p2m_update_lock, flags);
		vaddr += PMD_SIZE;
	}

	return lookup_address(addr, &level);
}

/*
 * Fully allocate the p2m structure for a given pfn.  We need to check

	if (pte_pg == p2m_missing_pte || pte_pg == p2m_identity_pte) {
		/* PMD level is missing, allocate a new one */
		ptep = alloc_p2m_pmd(addr, pte_pg);
		if (!ptep)
			return false;
	}
