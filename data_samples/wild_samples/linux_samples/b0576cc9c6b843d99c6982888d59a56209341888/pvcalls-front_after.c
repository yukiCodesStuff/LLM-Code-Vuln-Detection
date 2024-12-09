{
	if (!map->active.ring)
		return;

	free_pages_exact(map->active.data.in,
			 PAGE_SIZE << map->active.ring->ring_order);
	free_page((unsigned long)map->active.ring);
}

static int alloc_active_ring(struct sock_mapping *map)
{
	void *bytes;

	map->active.ring = (struct pvcalls_data_intf *)
		get_zeroed_page(GFP_KERNEL);
	if (!map->active.ring)
		goto out;

	map->active.ring->ring_order = PVCALLS_RING_ORDER;
	bytes = alloc_pages_exact(PAGE_SIZE << PVCALLS_RING_ORDER,
				  GFP_KERNEL | __GFP_ZERO);
	if (!bytes)
		goto out;

	map->active.data.in = bytes;
	map->active.data.out = bytes +
		XEN_FLEX_RING_SIZE(PVCALLS_RING_ORDER);

	return 0;

out:
	free_active_ring(map);
	return -ENOMEM;
}

static int create_active(struct sock_mapping *map, evtchn_port_t *evtchn)
{
	void *bytes;
	int ret, irq = -1, i;

	*evtchn = 0;
	init_waitqueue_head(&map->active.inflight_conn_req);

	bytes = map->active.data.in;
	for (i = 0; i < (1 << PVCALLS_RING_ORDER); i++)
		map->active.ring->ref[i] = gnttab_grant_foreign_access(
			pvcalls_front_dev->otherend_id,
			pfn_to_gfn(virt_to_pfn(bytes) + i), 0);

	map->active.ref = gnttab_grant_foreign_access(
		pvcalls_front_dev->otherend_id,
		pfn_to_gfn(virt_to_pfn((void *)map->active.ring)), 0);

	ret = xenbus_alloc_evtchn(pvcalls_front_dev, evtchn);
	if (ret)
		goto out_error;
	irq = bind_evtchn_to_irqhandler(*evtchn, pvcalls_front_conn_handler,
					0, "pvcalls-frontend", map);
	if (irq < 0) {
		ret = irq;
		goto out_error;
	}
{
	void *bytes;

	map->active.ring = (struct pvcalls_data_intf *)
		get_zeroed_page(GFP_KERNEL);
	if (!map->active.ring)
		goto out;

	map->active.ring->ring_order = PVCALLS_RING_ORDER;
	bytes = alloc_pages_exact(PAGE_SIZE << PVCALLS_RING_ORDER,
				  GFP_KERNEL | __GFP_ZERO);
	if (!bytes)
		goto out;

	map->active.data.in = bytes;
	map->active.data.out = bytes +
		XEN_FLEX_RING_SIZE(PVCALLS_RING_ORDER);

	return 0;

out:
	free_active_ring(map);
	return -ENOMEM;
}

static int create_active(struct sock_mapping *map, evtchn_port_t *evtchn)
{
	void *bytes;
	int ret, irq = -1, i;

	*evtchn = 0;
	init_waitqueue_head(&map->active.inflight_conn_req);

	bytes = map->active.data.in;
	for (i = 0; i < (1 << PVCALLS_RING_ORDER); i++)
		map->active.ring->ref[i] = gnttab_grant_foreign_access(
			pvcalls_front_dev->otherend_id,
			pfn_to_gfn(virt_to_pfn(bytes) + i), 0);

	map->active.ref = gnttab_grant_foreign_access(
		pvcalls_front_dev->otherend_id,
		pfn_to_gfn(virt_to_pfn((void *)map->active.ring)), 0);

	ret = xenbus_alloc_evtchn(pvcalls_front_dev, evtchn);
	if (ret)
		goto out_error;
	irq = bind_evtchn_to_irqhandler(*evtchn, pvcalls_front_conn_handler,
					0, "pvcalls-frontend", map);
	if (irq < 0) {
		ret = irq;
		goto out_error;
	}