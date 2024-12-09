	return NUM_SNAPSHOT_HDR_CHUNKS + ((ps->exceptions_per_area + 1) * area);
}

/*
 * Read or write a metadata area.  Remembering to skip the first
 * chunk which holds the header.
 */

	ps->current_area--;

	return 0;
}

static struct pstore *get_info(struct dm_exception_store *store)
					struct dm_exception *e)
{
	struct pstore *ps = get_info(store);
	uint32_t stride;
	chunk_t next_free;
	sector_t size = get_dev_size(dm_snap_cow(store->snap)->bdev);

	/* Is there enough room ? */
	if (size < ((ps->next_free + 1) * store->chunk_size))
	 * Move onto the next free pending, making sure to take
	 * into account the location of the metadata chunks.
	 */
	stride = (ps->exceptions_per_area + 1);
	next_free = ++ps->next_free;
	if (sector_div(next_free, stride) == 1)
		ps->next_free++;

	atomic_inc(&ps->pending_count);
	return 0;
}