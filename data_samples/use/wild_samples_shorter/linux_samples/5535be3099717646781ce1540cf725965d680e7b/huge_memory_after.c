
	assert_spin_locked(pmd_lockptr(mm, pmd));

	/* FOLL_GET and FOLL_PIN are mutually exclusive. */
	if (WARN_ON_ONCE((flags & (FOLL_PIN | FOLL_GET)) ==
			 (FOLL_PIN | FOLL_GET)))
		return NULL;
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

	page += (addr & ~HPAGE_PMD_MASK) >> PAGE_SHIFT;
	VM_BUG_ON_PAGE(!PageCompound(page) && !is_zone_device_page(page), page);

	return page;
}

/* NUMA hinting page fault entry point for trans huge pmds */