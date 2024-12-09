	if (!map->active.ring)
		return;

	free_pages_exact(map->active.data.in,
			 PAGE_SIZE << map->active.ring->ring_order);
	free_page((unsigned long)map->active.ring);
}

static int alloc_active_ring(struct sock_mapping *map)
		goto out;

	map->active.ring->ring_order = PVCALLS_RING_ORDER;
	bytes = alloc_pages_exact(PAGE_SIZE << PVCALLS_RING_ORDER,
				  GFP_KERNEL | __GFP_ZERO);
	if (!bytes)
		goto out;

	map->active.data.in = bytes;