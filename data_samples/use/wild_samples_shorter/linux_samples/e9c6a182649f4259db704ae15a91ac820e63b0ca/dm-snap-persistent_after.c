	return NUM_SNAPSHOT_HDR_CHUNKS + ((ps->exceptions_per_area + 1) * area);
}

static void skip_metadata(struct pstore *ps)
{
	uint32_t stride = ps->exceptions_per_area + 1;
	chunk_t next_free = ps->next_free;
	if (sector_div(next_free, stride) == NUM_SNAPSHOT_HDR_CHUNKS)
		ps->next_free++;
}

/*
 * Read or write a metadata area.  Remembering to skip the first
 * chunk which holds the header.
 */

	ps->current_area--;

	skip_metadata(ps);

	return 0;
}

static struct pstore *get_info(struct dm_exception_store *store)
					struct dm_exception *e)
{
	struct pstore *ps = get_info(store);
	sector_t size = get_dev_size(dm_snap_cow(store->snap)->bdev);

	/* Is there enough room ? */
	if (size < ((ps->next_free + 1) * store->chunk_size))
	 * Move onto the next free pending, making sure to take
	 * into account the location of the metadata chunks.
	 */
	ps->next_free++;
	skip_metadata(ps);

	atomic_inc(&ps->pending_count);
	return 0;
}