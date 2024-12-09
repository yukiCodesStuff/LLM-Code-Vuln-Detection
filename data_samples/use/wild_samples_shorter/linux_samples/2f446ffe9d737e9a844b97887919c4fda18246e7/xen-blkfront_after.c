			goto out_of_memory;

		if (info->feature_persistent) {
			granted_page = alloc_page(GFP_NOIO | __GFP_ZERO);
			if (!granted_page) {
				kfree(gnt_list_entry);
				goto out_of_memory;
			}

		BUG_ON(!list_empty(&rinfo->indirect_pages));
		for (i = 0; i < num; i++) {
			struct page *indirect_page = alloc_page(GFP_KERNEL |
								__GFP_ZERO);
			if (!indirect_page)
				goto out_of_memory;
			list_add(&indirect_page->lru, &rinfo->indirect_pages);
		}