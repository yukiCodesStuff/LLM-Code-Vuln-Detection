	if (!map->active.ring)
		return;

	free_pages((unsigned long)map->active.data.in,
			map->active.ring->ring_order);
	free_page((unsigned long)map->active.ring);
}

static int alloc_active_ring(struct sock_mapping *map)
		goto out;

	map->active.ring->ring_order = PVCALLS_RING_ORDER;
	bytes = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
					PVCALLS_RING_ORDER);
	if (!bytes)
		goto out;

	map->active.data.in = bytes;