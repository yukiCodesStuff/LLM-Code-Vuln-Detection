		      unsigned int nr_pages, grant_ref_t *grefs)
{
	int err;
	unsigned int i;
	grant_ref_t gref_head;

	err = gnttab_alloc_grant_references(nr_pages, &gref_head);
	if (err) {
		xenbus_dev_fatal(dev, err, "granting access to ring page");
		return err;
	}

	for (i = 0; i < nr_pages; i++) {
		unsigned long gfn;

		else
			gfn = virt_to_gfn(vaddr);

		grefs[i] = gnttab_claim_grant_reference(&gref_head);
		gnttab_grant_foreign_access_ref(grefs[i], dev->otherend_id,
						gfn, 0);

		vaddr = vaddr + XEN_PAGE_SIZE;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(xenbus_grant_ring);

