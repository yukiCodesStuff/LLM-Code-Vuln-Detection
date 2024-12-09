	if (PageAnon(vmf->page)) {
		struct page *page = vmf->page;

		/*
		 * We have to verify under page lock: these early checks are
		 * just an optimization to avoid locking the page and freeing
		 * the swapcache if there is little hope that we can reuse.
		 *
		 * PageKsm() doesn't necessarily raise the page refcount.
		 */
		if (PageKsm(page) || page_count(page) > 1 + PageSwapCache(page))
			goto copy;
		if (!trylock_page(page))
			goto copy;
		if (PageSwapCache(page))
			try_to_free_swap(page);
		if (PageKsm(page) || page_count(page) != 1) {
			unlock_page(page);
			goto copy;
		}