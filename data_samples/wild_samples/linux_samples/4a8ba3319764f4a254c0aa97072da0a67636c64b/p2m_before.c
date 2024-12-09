{
	if (unlikely(!slab_is_available()))
		return alloc_bootmem_align(PAGE_SIZE, PAGE_SIZE);

	return (void *)__get_free_page(GFP_KERNEL | __GFP_REPEAT);
}

/* Only to be called in case of a race for a page just allocated! */
static void free_p2m_page(void *p)
{
	BUG_ON(!slab_is_available());
	free_page((unsigned long)p);
}

/*
 * Build the parallel p2m_top_mfn and p2m_mid_mfn structures
 *
 * This is called both at boot time, and after resuming from suspend:
 * - At boot time we're called rather early, and must use alloc_bootmem*()
 *   to allocate memory.
 *
 * - After resume we're called from within stop_machine, but the mfn
 *   tree should already be completely allocated.
 */
void __ref xen_build_mfn_list_list(void)
{
	unsigned long pfn, mfn;
	pte_t *ptep;
	unsigned int level, topidx, mididx;
	unsigned long *mid_mfn_p;

	if (xen_feature(XENFEAT_auto_translated_physmap))
		return;

	/* Pre-initialize p2m_top_mfn to be completely missing */
	if (p2m_top_mfn == NULL) {
		p2m_mid_missing_mfn = alloc_p2m_page();
		p2m_mid_mfn_init(p2m_mid_missing_mfn, p2m_missing);

		p2m_top_mfn_p = alloc_p2m_page();
		p2m_top_mfn_p_init(p2m_top_mfn_p);

		p2m_top_mfn = alloc_p2m_page();
		p2m_top_mfn_init(p2m_top_mfn);
	} else {
		/* Reinitialise, mfn's all change after migration */
		p2m_mid_mfn_init(p2m_mid_missing_mfn, p2m_missing);
	}

	for (pfn = 0; pfn < xen_max_p2m_pfn && pfn < MAX_P2M_PFN;
	     pfn += P2M_PER_PAGE) {
		topidx = p2m_top_index(pfn);
		mididx = p2m_mid_index(pfn);

		mid_mfn_p = p2m_top_mfn_p[topidx];
		ptep = lookup_address((unsigned long)(xen_p2m_addr + pfn),
				      &level);
		BUG_ON(!ptep || level != PG_LEVEL_4K);
		mfn = pte_mfn(*ptep);
		ptep = (pte_t *)((unsigned long)ptep & ~(PAGE_SIZE - 1));

		/* Don't bother allocating any mfn mid levels if
		 * they're just missing, just update the stored mfn,
		 * since all could have changed over a migrate.
		 */
		if (ptep == p2m_missing_pte || ptep == p2m_identity_pte) {
			BUG_ON(mididx);
			BUG_ON(mid_mfn_p != p2m_mid_missing_mfn);
			p2m_top_mfn[topidx] = virt_to_mfn(p2m_mid_missing_mfn);
			pfn += (P2M_MID_PER_PAGE - 1) * P2M_PER_PAGE;
			continue;
		}

		if (mid_mfn_p == p2m_mid_missing_mfn) {
			mid_mfn_p = alloc_p2m_page();
			p2m_mid_mfn_init(mid_mfn_p, p2m_missing);

			p2m_top_mfn_p[topidx] = mid_mfn_p;
		}

		p2m_top_mfn[topidx] = virt_to_mfn(mid_mfn_p);
		mid_mfn_p[mididx] = mfn;
	}
}
		for (i = 0; i < PMDS_PER_MID_PAGE; i++) {
			pmdp = populate_extra_pmd(
				(unsigned long)(p2m + pfn + i * PTRS_PER_PTE));
			set_pmd(pmdp, __pmd(__pa(ptep) | _KERNPG_TABLE));
		}
	if (unlikely(pfn >= xen_p2m_size)) {
		if (pfn < xen_max_p2m_pfn)
			return xen_chk_extra_mem(pfn);

		return IDENTITY_FRAME(pfn);
	}

	ptep = lookup_address((unsigned long)(xen_p2m_addr + pfn), &level);
	BUG_ON(!ptep || level != PG_LEVEL_4K);

	/*
	 * The INVALID_P2M_ENTRY is filled in both p2m_*identity
	 * and in p2m_*missing, so returning the INVALID_P2M_ENTRY
	 * would be wrong.
	 */
	if (pte_pfn(*ptep) == PFN_DOWN(__pa(p2m_identity)))
		return IDENTITY_FRAME(pfn);

	return xen_p2m_addr[pfn];
}
EXPORT_SYMBOL_GPL(get_phys_to_machine);

/*
 * Allocate new pmd(s). It is checked whether the old pmd is still in place.
 * If not, nothing is changed. This is okay as the only reason for allocating
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
	unsigned long vaddr;
	int i;

	/* Do all allocations first to bail out in error case. */
	for (i = 0; i < PMDS_PER_MID_PAGE; i++) {
		pte_newpg[i] = alloc_p2m_page();
		if (!pte_newpg[i]) {
			for (i--; i >= 0; i--)
				free_p2m_page(pte_newpg[i]);

			return NULL;
		}
	}

	vaddr = addr & ~(PMD_SIZE * PMDS_PER_MID_PAGE - 1);

	for (i = 0; i < PMDS_PER_MID_PAGE; i++) {
		copy_page(pte_newpg[i], pte_pg);
		paravirt_alloc_pte(&init_mm, __pa(pte_newpg[i]) >> PAGE_SHIFT);

		pmdp = lookup_pmd_address(vaddr);
		BUG_ON(!pmdp);

		spin_lock_irqsave(&p2m_update_lock, flags);

		ptechk = lookup_address(vaddr, &level);
		if (ptechk == pte_pg) {
			set_pmd(pmdp,
				__pmd(__pa(pte_newpg[i]) | _KERNPG_TABLE));
			if (vaddr == (addr & ~(PMD_SIZE - 1)))
				pteret = pte_offset_kernel(pmdp, addr);
			pte_newpg[i] = NULL;
		}

		spin_unlock_irqrestore(&p2m_update_lock, flags);

		if (pte_newpg[i]) {
			paravirt_release_pte(__pa(pte_newpg[i]) >> PAGE_SHIFT);
			free_p2m_page(pte_newpg[i]);
		}

		vaddr += PMD_SIZE;
	}

	return pteret;
}
		if (ptechk == pte_pg) {
			set_pmd(pmdp,
				__pmd(__pa(pte_newpg[i]) | _KERNPG_TABLE));
			if (vaddr == (addr & ~(PMD_SIZE - 1)))
				pteret = pte_offset_kernel(pmdp, addr);
			pte_newpg[i] = NULL;
		}

		spin_unlock_irqrestore(&p2m_update_lock, flags);

		if (pte_newpg[i]) {
			paravirt_release_pte(__pa(pte_newpg[i]) >> PAGE_SHIFT);
			free_p2m_page(pte_newpg[i]);
		}

		vaddr += PMD_SIZE;
	}

	return pteret;
}

/*
 * Fully allocate the p2m structure for a given pfn.  We need to check
 * that both the top and mid levels are allocated, and make sure the
 * parallel mfn tree is kept in sync.  We may race with other cpus, so
 * the new pages are installed with cmpxchg; if we lose the race then
 * simply free the page we allocated and use the one that's there.
 */
static bool alloc_p2m(unsigned long pfn)
{
	unsigned topidx, mididx;
	unsigned long *top_mfn_p, *mid_mfn;
	pte_t *ptep, *pte_pg;
	unsigned int level;
	unsigned long flags;
	unsigned long addr = (unsigned long)(xen_p2m_addr + pfn);
	unsigned long p2m_pfn;

	topidx = p2m_top_index(pfn);
	mididx = p2m_mid_index(pfn);

	ptep = lookup_address(addr, &level);
	BUG_ON(!ptep || level != PG_LEVEL_4K);
	pte_pg = (pte_t *)((unsigned long)ptep & ~(PAGE_SIZE - 1));

	if (pte_pg == p2m_missing_pte || pte_pg == p2m_identity_pte) {
		/* PMD level is missing, allocate a new one */
		ptep = alloc_p2m_pmd(addr, ptep, pte_pg);
		if (!ptep)
			return false;
	}

	if (p2m_top_mfn) {
		top_mfn_p = &p2m_top_mfn[topidx];
		mid_mfn = ACCESS_ONCE(p2m_top_mfn_p[topidx]);

		BUG_ON(virt_to_mfn(mid_mfn) != *top_mfn_p);

		if (mid_mfn == p2m_mid_missing_mfn) {
			/* Separately check the mid mfn level */
			unsigned long missing_mfn;
			unsigned long mid_mfn_mfn;
			unsigned long old_mfn;

			mid_mfn = alloc_p2m_page();
			if (!mid_mfn)
				return false;

			p2m_mid_mfn_init(mid_mfn, p2m_missing);

			missing_mfn = virt_to_mfn(p2m_mid_missing_mfn);
			mid_mfn_mfn = virt_to_mfn(mid_mfn);
			old_mfn = cmpxchg(top_mfn_p, missing_mfn, mid_mfn_mfn);
			if (old_mfn != missing_mfn) {
				free_p2m_page(mid_mfn);
				mid_mfn = mfn_to_virt(old_mfn);
			} else {
				p2m_top_mfn_p[topidx] = mid_mfn;
			}
		}
		if (pte_newpg[i]) {
			paravirt_release_pte(__pa(pte_newpg[i]) >> PAGE_SHIFT);
			free_p2m_page(pte_newpg[i]);
		}

		vaddr += PMD_SIZE;
	}

	return pteret;
}

/*
 * Fully allocate the p2m structure for a given pfn.  We need to check
 * that both the top and mid levels are allocated, and make sure the
 * parallel mfn tree is kept in sync.  We may race with other cpus, so
 * the new pages are installed with cmpxchg; if we lose the race then
 * simply free the page we allocated and use the one that's there.
 */
static bool alloc_p2m(unsigned long pfn)
{
	unsigned topidx, mididx;
	unsigned long *top_mfn_p, *mid_mfn;
	pte_t *ptep, *pte_pg;
	unsigned int level;
	unsigned long flags;
	unsigned long addr = (unsigned long)(xen_p2m_addr + pfn);
	unsigned long p2m_pfn;

	topidx = p2m_top_index(pfn);
	mididx = p2m_mid_index(pfn);

	ptep = lookup_address(addr, &level);
	BUG_ON(!ptep || level != PG_LEVEL_4K);
	pte_pg = (pte_t *)((unsigned long)ptep & ~(PAGE_SIZE - 1));

	if (pte_pg == p2m_missing_pte || pte_pg == p2m_identity_pte) {
		/* PMD level is missing, allocate a new one */
		ptep = alloc_p2m_pmd(addr, ptep, pte_pg);
		if (!ptep)
			return false;
	}

	if (p2m_top_mfn) {
		top_mfn_p = &p2m_top_mfn[topidx];
		mid_mfn = ACCESS_ONCE(p2m_top_mfn_p[topidx]);

		BUG_ON(virt_to_mfn(mid_mfn) != *top_mfn_p);

		if (mid_mfn == p2m_mid_missing_mfn) {
			/* Separately check the mid mfn level */
			unsigned long missing_mfn;
			unsigned long mid_mfn_mfn;
			unsigned long old_mfn;

			mid_mfn = alloc_p2m_page();
			if (!mid_mfn)
				return false;

			p2m_mid_mfn_init(mid_mfn, p2m_missing);

			missing_mfn = virt_to_mfn(p2m_mid_missing_mfn);
			mid_mfn_mfn = virt_to_mfn(mid_mfn);
			old_mfn = cmpxchg(top_mfn_p, missing_mfn, mid_mfn_mfn);
			if (old_mfn != missing_mfn) {
				free_p2m_page(mid_mfn);
				mid_mfn = mfn_to_virt(old_mfn);
			} else {
				p2m_top_mfn_p[topidx] = mid_mfn;
			}
		}
	} else {
		mid_mfn = NULL;
	}

	p2m_pfn = pte_pfn(ACCESS_ONCE(*ptep));
	if (p2m_pfn == PFN_DOWN(__pa(p2m_identity)) ||
	    p2m_pfn == PFN_DOWN(__pa(p2m_missing))) {
		/* p2m leaf page is missing */
		unsigned long *p2m;

		p2m = alloc_p2m_page();
		if (!p2m)
			return false;

		if (p2m_pfn == PFN_DOWN(__pa(p2m_missing)))
			p2m_init(p2m);
		else
			p2m_init_identity(p2m, pfn);

		spin_lock_irqsave(&p2m_update_lock, flags);

		if (pte_pfn(*ptep) == p2m_pfn) {
			set_pte(ptep,
				pfn_pte(PFN_DOWN(__pa(p2m)), PAGE_KERNEL));
			if (mid_mfn)
				mid_mfn[mididx] = virt_to_mfn(p2m);
			p2m = NULL;
		}

		spin_unlock_irqrestore(&p2m_update_lock, flags);

		if (p2m)
			free_p2m_page(p2m);
	}

	return true;
}
	if (pte_pg == p2m_missing_pte || pte_pg == p2m_identity_pte) {
		/* PMD level is missing, allocate a new one */
		ptep = alloc_p2m_pmd(addr, ptep, pte_pg);
		if (!ptep)
			return false;
	}