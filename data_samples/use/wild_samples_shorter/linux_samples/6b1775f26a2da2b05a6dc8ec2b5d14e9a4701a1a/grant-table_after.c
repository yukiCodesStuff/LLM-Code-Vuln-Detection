	       what, ref, page ? page_to_pfn(page) : -1);
}

int gnttab_try_end_foreign_access(grant_ref_t ref)
{
	int ret = _gnttab_end_foreign_access_ref(ref, 0);

	if (ret)
		put_free_entry(ref);

	return ret;
}
EXPORT_SYMBOL_GPL(gnttab_try_end_foreign_access);

void gnttab_end_foreign_access(grant_ref_t ref, int readonly,
			       unsigned long page)
{
	if (gnttab_try_end_foreign_access(ref)) {
		if (page != 0)
			put_page(virt_to_page(page));
	} else
		gnttab_add_deferred(ref, readonly,