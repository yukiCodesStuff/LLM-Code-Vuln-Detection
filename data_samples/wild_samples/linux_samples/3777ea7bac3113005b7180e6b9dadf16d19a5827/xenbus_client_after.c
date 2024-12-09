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

		if (is_vmalloc_addr(vaddr))
			gfn = pfn_to_gfn(vmalloc_to_pfn(vaddr));
		else
			gfn = virt_to_gfn(vaddr);

		grefs[i] = gnttab_claim_grant_reference(&gref_head);
		gnttab_grant_foreign_access_ref(grefs[i], dev->otherend_id,
						gfn, 0);

		vaddr = vaddr + XEN_PAGE_SIZE;
	}

	return 0;
}
	for (i = 0; i < nr_pages; i++) {
		unsigned long gfn;

		if (is_vmalloc_addr(vaddr))
			gfn = pfn_to_gfn(vmalloc_to_pfn(vaddr));
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


/**
 * Allocate an event channel for the given xenbus_device, assigning the newly
 * created local port to *port.  Return 0 on success, or -errno on error.  On
 * error, the device will switch to XenbusStateClosing, and the error will be
 * saved in the store.
 */
int xenbus_alloc_evtchn(struct xenbus_device *dev, evtchn_port_t *port)
{
	struct evtchn_alloc_unbound alloc_unbound;
	int err;

	alloc_unbound.dom = DOMID_SELF;
	alloc_unbound.remote_dom = dev->otherend_id;

	err = HYPERVISOR_event_channel_op(EVTCHNOP_alloc_unbound,
					  &alloc_unbound);
	if (err)
		xenbus_dev_fatal(dev, err, "allocating event channel");
	else
		*port = alloc_unbound.port;

	return err;
}
EXPORT_SYMBOL_GPL(xenbus_alloc_evtchn);


/**
 * Free an existing event channel. Returns 0 on success or -errno on error.
 */
int xenbus_free_evtchn(struct xenbus_device *dev, evtchn_port_t port)
{
	struct evtchn_close close;
	int err;

	close.port = port;

	err = HYPERVISOR_event_channel_op(EVTCHNOP_close, &close);
	if (err)
		xenbus_dev_error(dev, err, "freeing event channel %u", port);

	return err;
}
EXPORT_SYMBOL_GPL(xenbus_free_evtchn);


/**
 * xenbus_map_ring_valloc
 * @dev: xenbus device
 * @gnt_refs: grant reference array
 * @nr_grefs: number of grant references
 * @vaddr: pointer to address to be filled out by mapping
 *
 * Map @nr_grefs pages of memory into this domain from another
 * domain's grant table.  xenbus_map_ring_valloc allocates @nr_grefs
 * pages of virtual address space, maps the pages to that address, and
 * sets *vaddr to that address.  Returns 0 on success, and -errno on
 * error. If an error is returned, device will switch to
 * XenbusStateClosing and the error message will be saved in XenStore.
 */
int xenbus_map_ring_valloc(struct xenbus_device *dev, grant_ref_t *gnt_refs,
			   unsigned int nr_grefs, void **vaddr)
{
	int err;
	struct map_ring_valloc *info;

	*vaddr = NULL;

	if (nr_grefs > XENBUS_MAX_RING_GRANTS)
		return -EINVAL;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->node = kzalloc(sizeof(*info->node), GFP_KERNEL);
	if (!info->node)
		err = -ENOMEM;
	else
		err = ring_ops->map(dev, info, gnt_refs, nr_grefs, vaddr);

	kfree(info->node);
	kfree(info);
	return err;
}
EXPORT_SYMBOL_GPL(xenbus_map_ring_valloc);

/* N.B. sizeof(phys_addr_t) doesn't always equal to sizeof(unsigned
 * long), e.g. 32-on-64.  Caller is responsible for preparing the
 * right array to feed into this function */
static int __xenbus_map_ring(struct xenbus_device *dev,
			     grant_ref_t *gnt_refs,
			     unsigned int nr_grefs,
			     grant_handle_t *handles,
			     struct map_ring_valloc *info,
			     unsigned int flags,
			     bool *leaked)
{
	int i, j;

	if (nr_grefs > XENBUS_MAX_RING_GRANTS)
		return -EINVAL;

	for (i = 0; i < nr_grefs; i++) {
		gnttab_set_map_op(&info->map[i], info->phys_addrs[i], flags,
				  gnt_refs[i], dev->otherend_id);
		handles[i] = INVALID_GRANT_HANDLE;
	}

	gnttab_batch_map(info->map, i);

	for (i = 0; i < nr_grefs; i++) {
		if (info->map[i].status != GNTST_okay) {
			xenbus_dev_fatal(dev, info->map[i].status,
					 "mapping in shared page %d from domain %d",
					 gnt_refs[i], dev->otherend_id);
			goto fail;
		} else
			handles[i] = info->map[i].handle;
	}