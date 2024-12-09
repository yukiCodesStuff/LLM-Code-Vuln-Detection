	return 0;
}

static inline bool should_try_to_free_swap(struct page *page,
					   struct vm_area_struct *vma,
					   unsigned int fault_flags)
{
	if (!PageSwapCache(page))
		return false;
	if (mem_cgroup_swap_full(page) || (vma->vm_flags & VM_LOCKED) ||
	    PageMlocked(page))
		return true;
	/*
	 * If we want to map a page that's in the swapcache writable, we
	 * have to detect via the refcount if we're really the exclusive
	 * user. Try freeing the swapcache to get rid of the swapcache
	 * reference only in case it's likely that we'll be the exlusive user.
	 */
	return (fault_flags & FAULT_FLAG_WRITE) && !PageKsm(page) &&
		page_count(page) == 2;
}

/*
 * We enter with non-exclusive mmap_lock (to exclude vma changes,
 * but allow concurrent faults), and pte mapped but not yet locked.
 * We return with pte unmapped and unlocked.
			page = swapcache;
			goto out_page;
		}

		/*
		 * If we want to map a page that's in the swapcache writable, we
		 * have to detect via the refcount if we're really the exclusive
		 * owner. Try removing the extra reference from the local LRU
		 * pagevecs if required.
		 */
		if ((vmf->flags & FAULT_FLAG_WRITE) && page == swapcache &&
		    !PageKsm(page) && !PageLRU(page))
			lru_add_drain();
	}

	cgroup_throttle_swaprate(page, GFP_KERNEL);

	}

	/*
	 * Remove the swap entry and conditionally try to free up the swapcache.
	 * We're already holding a reference on the page but haven't mapped it
	 * yet.
	 */
	swap_free(entry);
	if (should_try_to_free_swap(page, vma, vmf->flags))
		try_to_free_swap(page);

	inc_mm_counter_fast(vma->vm_mm, MM_ANONPAGES);
	dec_mm_counter_fast(vma->vm_mm, MM_SWAPENTS);
	pte = mk_pte(page, vma->vm_page_prot);

	/*
	 * Same logic as in do_wp_page(); however, optimize for fresh pages
	 * that are certainly not shared because we just allocated them without
	 * exposing them to the swapcache.
	 */
	if ((vmf->flags & FAULT_FLAG_WRITE) && !PageKsm(page) &&
	    (page != swapcache || page_count(page) == 1)) {
		pte = maybe_mkwrite(pte_mkdirty(pte), vma);
		vmf->flags &= ~FAULT_FLAG_WRITE;
		ret |= VM_FAULT_WRITE;
		exclusive = RMAP_EXCLUSIVE;
	set_pte_at(vma->vm_mm, vmf->address, vmf->pte, pte);
	arch_do_swap_page(vma->vm_mm, vma, vmf->address, pte, vmf->orig_pte);

	unlock_page(page);
	if (page != swapcache && swapcache) {
		/*
		 * Hold the lock to avoid the swap entry to be reused