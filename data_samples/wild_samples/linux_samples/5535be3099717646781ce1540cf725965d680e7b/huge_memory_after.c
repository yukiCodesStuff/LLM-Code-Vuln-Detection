{
	unsigned long pfn = pmd_pfn(*pmd);
	struct mm_struct *mm = vma->vm_mm;
	struct page *page;

	assert_spin_locked(pmd_lockptr(mm, pmd));

	/* FOLL_GET and FOLL_PIN are mutually exclusive. */
	if (WARN_ON_ONCE((flags & (FOLL_PIN | FOLL_GET)) ==
			 (FOLL_PIN | FOLL_GET)))
		return NULL;

	if (flags & FOLL_WRITE && !pmd_write(*pmd))
		return NULL;

	if (pmd_present(*pmd) && pmd_devmap(*pmd))
		/* pass */;
	else
		return NULL;

	if (flags & FOLL_TOUCH)
		touch_pmd(vma, addr, pmd, flags & FOLL_WRITE);

	/*
	 * device mapped pages can only be returned if the
	 * caller will manage the page reference count.
	 */
	if (!(flags & (FOLL_GET | FOLL_PIN)))
		return ERR_PTR(-EEXIST);

	pfn += (addr & ~PMD_MASK) >> PAGE_SHIFT;
	*pgmap = get_dev_pagemap(pfn, *pgmap);
	if (!*pgmap)
		return ERR_PTR(-EFAULT);
	page = pfn_to_page(pfn);
	if (!try_grab_page(page, flags))
		page = ERR_PTR(-ENOMEM);

	return page;
}

int copy_huge_pmd(struct mm_struct *dst_mm, struct mm_struct *src_mm,
		  pmd_t *dst_pmd, pmd_t *src_pmd, unsigned long addr,
		  struct vm_area_struct *dst_vma, struct vm_area_struct *src_vma)
{
	spinlock_t *dst_ptl, *src_ptl;
	struct page *src_page;
	pmd_t pmd;
	pgtable_t pgtable = NULL;
	int ret = -ENOMEM;

	/* Skip if can be re-fill on fault */
	if (!vma_is_anonymous(dst_vma))
		return 0;

	pgtable = pte_alloc_one(dst_mm);
	if (unlikely(!pgtable))
		goto out;

	dst_ptl = pmd_lock(dst_mm, dst_pmd);
	src_ptl = pmd_lockptr(src_mm, src_pmd);
	spin_lock_nested(src_ptl, SINGLE_DEPTH_NESTING);

	ret = -EAGAIN;
	pmd = *src_pmd;

#ifdef CONFIG_ARCH_ENABLE_THP_MIGRATION
	if (unlikely(is_swap_pmd(pmd))) {
		swp_entry_t entry = pmd_to_swp_entry(pmd);

		VM_BUG_ON(!is_pmd_migration_entry(pmd));
		if (!is_readable_migration_entry(entry)) {
			entry = make_readable_migration_entry(
							swp_offset(entry));
			pmd = swp_entry_to_pmd(entry);
			if (pmd_swp_soft_dirty(*src_pmd))
				pmd = pmd_swp_mksoft_dirty(pmd);
			if (pmd_swp_uffd_wp(*src_pmd))
				pmd = pmd_swp_mkuffd_wp(pmd);
			set_pmd_at(src_mm, addr, src_pmd, pmd);
		}
		add_mm_counter(dst_mm, MM_ANONPAGES, HPAGE_PMD_NR);
		mm_inc_nr_ptes(dst_mm);
		pgtable_trans_huge_deposit(dst_mm, dst_pmd, pgtable);
		if (!userfaultfd_wp(dst_vma))
			pmd = pmd_swp_clear_uffd_wp(pmd);
		set_pmd_at(dst_mm, addr, dst_pmd, pmd);
		ret = 0;
		goto out_unlock;
	}
		if (unlikely(unshare)) {
			spin_unlock(vmf->ptl);
			return 0;
		}
		entry = pmd_mkyoung(orig_pmd);
		entry = maybe_pmd_mkwrite(pmd_mkdirty(entry), vma);
		if (pmdp_set_access_flags(vma, haddr, vmf->pmd, entry, 1))
			update_mmu_cache_pmd(vma, vmf->address, vmf->pmd);
		spin_unlock(vmf->ptl);
		return VM_FAULT_WRITE;
	}

unlock_fallback:
	unlock_page(page);
	spin_unlock(vmf->ptl);
fallback:
	__split_huge_pmd(vma, vmf->pmd, vmf->address, false, NULL);
	return VM_FAULT_FALLBACK;
}

/* FOLL_FORCE can write to even unwritable PMDs in COW mappings. */
static inline bool can_follow_write_pmd(pmd_t pmd, struct page *page,
					struct vm_area_struct *vma,
					unsigned int flags)
{
	/* If the pmd is writable, we can write to the page. */
	if (pmd_write(pmd))
		return true;

	/* Maybe FOLL_FORCE is set to override it? */
	if (!(flags & FOLL_FORCE))
		return false;

	/* But FOLL_FORCE has no effect on shared mappings */
	if (vma->vm_flags & (VM_MAYSHARE | VM_SHARED))
		return false;

	/* ... or read-only private ones */
	if (!(vma->vm_flags & VM_MAYWRITE))
		return false;

	/* ... or already writable ones that just need to take a write fault */
	if (vma->vm_flags & VM_WRITE)
		return false;

	/*
	 * See can_change_pte_writable(): we broke COW and could map the page
	 * writable if we have an exclusive anonymous page ...
	 */
	if (!page || !PageAnon(page) || !PageAnonExclusive(page))
		return false;

	/* ... and a write-fault isn't required for other reasons. */
	if (vma_soft_dirty_enabled(vma) && !pmd_soft_dirty(pmd))
		return false;
	return !userfaultfd_huge_pmd_wp(vma, pmd);
}

struct page *follow_trans_huge_pmd(struct vm_area_struct *vma,
				   unsigned long addr,
				   pmd_t *pmd,
				   unsigned int flags)
{
	struct mm_struct *mm = vma->vm_mm;
	struct page *page;

	assert_spin_locked(pmd_lockptr(mm, pmd));

	page = pmd_page(*pmd);
	VM_BUG_ON_PAGE(!PageHead(page) && !is_zone_device_page(page), page);

	if ((flags & FOLL_WRITE) &&
	    !can_follow_write_pmd(*pmd, page, vma, flags))
		return NULL;

	/* Avoid dumping huge zero page */
	if ((flags & FOLL_DUMP) && is_huge_zero_pmd(*pmd))
		return ERR_PTR(-EFAULT);

	/* Full NUMA hinting faults to serialise migration in fault paths */
	if ((flags & FOLL_NUMA) && pmd_protnone(*pmd))
		return NULL;

	if (!pmd_write(*pmd) && gup_must_unshare(flags, page))
		return ERR_PTR(-EMLINK);

	VM_BUG_ON_PAGE((flags & FOLL_PIN) && PageAnon(page) &&
			!PageAnonExclusive(page), page);

	if (!try_grab_page(page, flags))
		return ERR_PTR(-ENOMEM);

	if (flags & FOLL_TOUCH)
		touch_pmd(vma, addr, pmd, flags & FOLL_WRITE);

	page += (addr & ~HPAGE_PMD_MASK) >> PAGE_SHIFT;
	VM_BUG_ON_PAGE(!PageCompound(page) && !is_zone_device_page(page), page);

	return page;
}

/* NUMA hinting page fault entry point for trans huge pmds */
vm_fault_t do_huge_pmd_numa_page(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	pmd_t oldpmd = vmf->orig_pmd;
	pmd_t pmd;
	struct page *page;
	unsigned long haddr = vmf->address & HPAGE_PMD_MASK;
	int page_nid = NUMA_NO_NODE;
	int target_nid, last_cpupid = -1;
	bool migrated = false;
	bool was_writable = pmd_savedwrite(oldpmd);
	int flags = 0;

	vmf->ptl = pmd_lock(vma->vm_mm, vmf->pmd);
	if (unlikely(!pmd_same(oldpmd, *vmf->pmd))) {
		spin_unlock(vmf->ptl);
		goto out;
	}
{
	struct mm_struct *mm = vma->vm_mm;
	struct page *page;

	assert_spin_locked(pmd_lockptr(mm, pmd));

	page = pmd_page(*pmd);
	VM_BUG_ON_PAGE(!PageHead(page) && !is_zone_device_page(page), page);

	if ((flags & FOLL_WRITE) &&
	    !can_follow_write_pmd(*pmd, page, vma, flags))
		return NULL;

	/* Avoid dumping huge zero page */
	if ((flags & FOLL_DUMP) && is_huge_zero_pmd(*pmd))
		return ERR_PTR(-EFAULT);

	/* Full NUMA hinting faults to serialise migration in fault paths */
	if ((flags & FOLL_NUMA) && pmd_protnone(*pmd))
		return NULL;

	if (!pmd_write(*pmd) && gup_must_unshare(flags, page))
		return ERR_PTR(-EMLINK);

	VM_BUG_ON_PAGE((flags & FOLL_PIN) && PageAnon(page) &&
			!PageAnonExclusive(page), page);

	if (!try_grab_page(page, flags))
		return ERR_PTR(-ENOMEM);

	if (flags & FOLL_TOUCH)
		touch_pmd(vma, addr, pmd, flags & FOLL_WRITE);

	page += (addr & ~HPAGE_PMD_MASK) >> PAGE_SHIFT;
	VM_BUG_ON_PAGE(!PageCompound(page) && !is_zone_device_page(page), page);

	return page;
}
{
	struct mm_struct *mm = vma->vm_mm;
	struct page *page;

	assert_spin_locked(pmd_lockptr(mm, pmd));

	page = pmd_page(*pmd);
	VM_BUG_ON_PAGE(!PageHead(page) && !is_zone_device_page(page), page);

	if ((flags & FOLL_WRITE) &&
	    !can_follow_write_pmd(*pmd, page, vma, flags))
		return NULL;

	/* Avoid dumping huge zero page */
	if ((flags & FOLL_DUMP) && is_huge_zero_pmd(*pmd))
		return ERR_PTR(-EFAULT);

	/* Full NUMA hinting faults to serialise migration in fault paths */
	if ((flags & FOLL_NUMA) && pmd_protnone(*pmd))
		return NULL;

	if (!pmd_write(*pmd) && gup_must_unshare(flags, page))
		return ERR_PTR(-EMLINK);

	VM_BUG_ON_PAGE((flags & FOLL_PIN) && PageAnon(page) &&
			!PageAnonExclusive(page), page);

	if (!try_grab_page(page, flags))
		return ERR_PTR(-ENOMEM);

	if (flags & FOLL_TOUCH)
		touch_pmd(vma, addr, pmd, flags & FOLL_WRITE);

	page += (addr & ~HPAGE_PMD_MASK) >> PAGE_SHIFT;
	VM_BUG_ON_PAGE(!PageCompound(page) && !is_zone_device_page(page), page);

	return page;
}

/* NUMA hinting page fault entry point for trans huge pmds */
vm_fault_t do_huge_pmd_numa_page(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	pmd_t oldpmd = vmf->orig_pmd;
	pmd_t pmd;
	struct page *page;
	unsigned long haddr = vmf->address & HPAGE_PMD_MASK;
	int page_nid = NUMA_NO_NODE;
	int target_nid, last_cpupid = -1;
	bool migrated = false;
	bool was_writable = pmd_savedwrite(oldpmd);
	int flags = 0;

	vmf->ptl = pmd_lock(vma->vm_mm, vmf->pmd);
	if (unlikely(!pmd_same(oldpmd, *vmf->pmd))) {
		spin_unlock(vmf->ptl);
		goto out;
	}