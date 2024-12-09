{
	int err;
	int i, j;

	for (i = 0; i < nr_pages; i++) {
		unsigned long gfn;

		if (is_vmalloc_addr(vaddr))
			gfn = pfn_to_gfn(vmalloc_to_pfn(vaddr));
		else
			gfn = virt_to_gfn(vaddr);

		err = gnttab_grant_foreign_access(dev->otherend_id, gfn, 0);
		if (err < 0) {
			xenbus_dev_fatal(dev, err,
					 "granting access to ring page");
			goto fail;
		}
		grefs[i] = err;

		vaddr = vaddr + XEN_PAGE_SIZE;
	}

	return 0;

fail:
	for (j = 0; j < i; j++)
		gnttab_end_foreign_access_ref(grefs[j], 0);
	return err;
}
	for (i = 0; i < nr_pages; i++) {
		unsigned long gfn;

		if (is_vmalloc_addr(vaddr))
			gfn = pfn_to_gfn(vmalloc_to_pfn(vaddr));
		else
			gfn = virt_to_gfn(vaddr);

		err = gnttab_grant_foreign_access(dev->otherend_id, gfn, 0);
		if (err < 0) {
			xenbus_dev_fatal(dev, err,
					 "granting access to ring page");
			goto fail;
		}