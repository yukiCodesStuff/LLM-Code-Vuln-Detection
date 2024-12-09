	       what, ref, page ? page_to_pfn(page) : -1);
}

void gnttab_end_foreign_access(grant_ref_t ref, int readonly,
			       unsigned long page)
{
	if (gnttab_end_foreign_access_ref(ref, readonly)) {
		put_free_entry(ref);
		if (page != 0)
			put_page(virt_to_page(page));
	} else
		gnttab_add_deferred(ref, readonly,