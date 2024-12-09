	if (unlikely(!pmd_same(*vmf->pmd, orig_pmd))) {
		spin_unlock(vmf->ptl);
		return 0;
	}

	page = pmd_page(orig_pmd);
	VM_BUG_ON_PAGE(!PageHead(page), page);

	/* Lock page for reuse_swap_page() */
	if (!trylock_page(page)) {
		get_page(page);
		spin_unlock(vmf->ptl);
		lock_page(page);
		spin_lock(vmf->ptl);
		if (unlikely(!pmd_same(*vmf->pmd, orig_pmd))) {
			spin_unlock(vmf->ptl);
			unlock_page(page);
			put_page(page);
			return 0;
		}
		put_page(page);
	}
		if (unlikely(!pmd_same(*vmf->pmd, orig_pmd))) {
			spin_unlock(vmf->ptl);
			unlock_page(page);
			put_page(page);
			return 0;
		}
		put_page(page);
	}

	/*
	 * We can only reuse the page if nobody else maps the huge page or it's
	 * part.
	 */
	if (reuse_swap_page(page)) {
		pmd_t entry;
		entry = pmd_mkyoung(orig_pmd);
		entry = maybe_pmd_mkwrite(pmd_mkdirty(entry), vma);
		if (pmdp_set_access_flags(vma, haddr, vmf->pmd, entry, 1))
			update_mmu_cache_pmd(vma, vmf->address, vmf->pmd);
		unlock_page(page);
		spin_unlock(vmf->ptl);
		return VM_FAULT_WRITE;
	}
	if (reuse_swap_page(page)) {
		pmd_t entry;
		entry = pmd_mkyoung(orig_pmd);
		entry = maybe_pmd_mkwrite(pmd_mkdirty(entry), vma);
		if (pmdp_set_access_flags(vma, haddr, vmf->pmd, entry, 1))
			update_mmu_cache_pmd(vma, vmf->address, vmf->pmd);
		unlock_page(page);
		spin_unlock(vmf->ptl);
		return VM_FAULT_WRITE;
	}

	unlock_page(page);
	spin_unlock(vmf->ptl);
fallback:
	__split_huge_pmd(vma, vmf->pmd, vmf->address, false, NULL);
	return VM_FAULT_FALLBACK;
}

/*
 * FOLL_FORCE can write to even unwritable pmd's, but only
 * after we've gone through a COW cycle and they are dirty.
 */
static inline bool can_follow_write_pmd(pmd_t pmd, unsigned int flags)
{
	return pmd_write(pmd) ||
	       ((flags & FOLL_FORCE) && (flags & FOLL_COW) && pmd_dirty(pmd));
}

struct page *follow_trans_huge_pmd(struct vm_area_struct *vma,
				   unsigned long addr,
				   pmd_t *pmd,
				   unsigned int flags)
{
	struct mm_struct *mm = vma->vm_mm;
	struct page *page = NULL;

	assert_spin_locked(pmd_lockptr(mm, pmd));

	if (flags & FOLL_WRITE && !can_follow_write_pmd(*pmd, flags))
		goto out;

	/* Avoid dumping huge zero page */
	if ((flags & FOLL_DUMP) && is_huge_zero_pmd(*pmd))
		return ERR_PTR(-EFAULT);

	/* Full NUMA hinting faults to serialise migration in fault paths */
	if ((flags & FOLL_NUMA) && pmd_protnone(*pmd))
		goto out;

	page = pmd_page(*pmd);
	VM_BUG_ON_PAGE(!PageHead(page) && !is_zone_device_page(page), page);

	if (!try_grab_page(page, flags))
		return ERR_PTR(-ENOMEM);

	if (flags & FOLL_TOUCH)
		touch_pmd(vma, addr, pmd, flags);

	page += (addr & ~HPAGE_PMD_MASK) >> PAGE_SHIFT;
	VM_BUG_ON_PAGE(!PageCompound(page) && !is_zone_device_page(page), page);

out:
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

	pmd = pmd_modify(oldpmd, vma->vm_page_prot);
	page = vm_normal_page_pmd(vma, haddr, pmd);
	if (!page)
		goto out_map;

	/* See similar comment in do_numa_page for explanation */
	if (!was_writable)
		flags |= TNF_NO_GROUP;

	page_nid = page_to_nid(page);
	last_cpupid = page_cpupid_last(page);
	target_nid = numa_migrate_prep(page, vma, haddr, page_nid,
				       &flags);

	if (target_nid == NUMA_NO_NODE) {
		put_page(page);
		goto out_map;
	}

	spin_unlock(vmf->ptl);

	migrated = migrate_misplaced_page(page, vma, target_nid);
	if (migrated) {
		flags |= TNF_MIGRATED;
		page_nid = target_nid;
	} else {
		flags |= TNF_MIGRATE_FAIL;
		vmf->ptl = pmd_lock(vma->vm_mm, vmf->pmd);
		if (unlikely(!pmd_same(oldpmd, *vmf->pmd))) {
			spin_unlock(vmf->ptl);
			goto out;
		}
		goto out_map;
	}