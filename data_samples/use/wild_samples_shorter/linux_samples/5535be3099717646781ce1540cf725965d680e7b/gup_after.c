	return -EEXIST;
}

/* FOLL_FORCE can write to even unwritable PTEs in COW mappings. */
static inline bool can_follow_write_pte(pte_t pte, struct page *page,
					struct vm_area_struct *vma,
					unsigned int flags)
{
	/* If the pte is writable, we can write to the page. */
	if (pte_write(pte))
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
	if (vma_soft_dirty_enabled(vma) && !pte_soft_dirty(pte))
		return false;
	return !userfaultfd_pte_wp(vma, pte);
}

static struct page *follow_page_pte(struct vm_area_struct *vma,
		unsigned long address, pmd_t *pmd, unsigned int flags,
	}
	if ((flags & FOLL_NUMA) && pte_protnone(pte))
		goto no_page;

	page = vm_normal_page(vma, address, pte);

	/*
	 * We only care about anon pages in can_follow_write_pte() and don't
	 * have to worry about pte_devmap() because they are never anon.
	 */
	if ((flags & FOLL_WRITE) &&
	    !can_follow_write_pte(pte, page, vma, flags)) {
		page = NULL;
		goto out;
	}

	if (!page && pte_devmap(pte) && (flags & (FOLL_GET | FOLL_PIN))) {
		/*
		 * Only return device mapping pages in the FOLL_GET or FOLL_PIN
		 * case since they are only valid while holding the pgmap
		return -EBUSY;
	}

	return 0;
}

static int check_vma_flags(struct vm_area_struct *vma, unsigned long gup_flags)