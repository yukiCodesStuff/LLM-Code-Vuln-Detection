	return (void *)__get_free_page(GFP_KERNEL | __GFP_REPEAT);
}

/* Only to be called in case of a race for a page just allocated! */
static void free_p2m_page(void *p)
{
	BUG_ON(!slab_is_available());
	free_page((unsigned long)p);
}

/*
			p2m_missing_pte : p2m_identity_pte;
		for (i = 0; i < PMDS_PER_MID_PAGE; i++) {
			pmdp = populate_extra_pmd(
				(unsigned long)(p2m + pfn + i * PTRS_PER_PTE));
			set_pmd(pmdp, __pmd(__pa(ptep) | _KERNPG_TABLE));
		}
	}
}
 * a new pmd is to replace p2m_missing_pte or p2m_identity_pte by a individual
 * pmd. In case of PAE/x86-32 there are multiple pmds to allocate!
 */
static pte_t *alloc_p2m_pmd(unsigned long addr, pte_t *ptep, pte_t *pte_pg)
{
	pte_t *ptechk;
	pte_t *pteret = ptep;
	pte_t *pte_newpg[PMDS_PER_MID_PAGE];
	pmd_t *pmdp;
	unsigned int level;
	unsigned long flags;
		if (ptechk == pte_pg) {
			set_pmd(pmdp,
				__pmd(__pa(pte_newpg[i]) | _KERNPG_TABLE));
			if (vaddr == (addr & ~(PMD_SIZE - 1)))
				pteret = pte_offset_kernel(pmdp, addr);
			pte_newpg[i] = NULL;
		}

		spin_unlock_irqrestore(&p2m_update_lock, flags);
		vaddr += PMD_SIZE;
	}

	return pteret;
}

/*
 * Fully allocate the p2m structure for a given pfn.  We need to check

	if (pte_pg == p2m_missing_pte || pte_pg == p2m_identity_pte) {
		/* PMD level is missing, allocate a new one */
		ptep = alloc_p2m_pmd(addr, ptep, pte_pg);
		if (!ptep)
			return false;
	}
