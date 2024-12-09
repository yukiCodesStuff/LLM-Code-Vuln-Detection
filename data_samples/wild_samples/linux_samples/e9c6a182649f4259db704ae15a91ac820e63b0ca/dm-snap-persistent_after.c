{
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
static int area_io(struct pstore *ps, int rw)
{
	int r;
	chunk_t chunk;

	chunk = area_location(ps, ps->current_area);

	r = chunk_io(ps, ps->area, chunk, rw, 0);
	if (r)
		return r;

	return 0;
}

static void zero_memory_area(struct pstore *ps)
{
	memset(ps->area, 0, ps->store->chunk_size << SECTOR_SHIFT);
}

static int zero_disk_area(struct pstore *ps, chunk_t area)
{
	return chunk_io(ps, ps->zero_area, area_location(ps, area), WRITE, 0);
}

static int read_header(struct pstore *ps, int *new_snapshot)
{
	int r;
	struct disk_header *dh;
	unsigned chunk_size;
	int chunk_size_supplied = 1;
	char *chunk_err;

	/*
	 * Use default chunk size (or logical_block_size, if larger)
	 * if none supplied
	 */
	if (!ps->store->chunk_size) {
		ps->store->chunk_size = max(DM_CHUNK_SIZE_DEFAULT_SECTORS,
		    bdev_logical_block_size(dm_snap_cow(ps->store->snap)->
					    bdev) >> 9);
		ps->store->chunk_mask = ps->store->chunk_size - 1;
		ps->store->chunk_shift = ffs(ps->store->chunk_size) - 1;
		chunk_size_supplied = 0;
	}
	for (ps->current_area = 0; full; ps->current_area++) {
		r = area_io(ps, READ);
		if (r)
			return r;

		r = insert_exceptions(ps, callback, callback_context, &full);
		if (r)
			return r;
	}

	ps->current_area--;

	skip_metadata(ps);

	return 0;
}

static struct pstore *get_info(struct dm_exception_store *store)
{
	return (struct pstore *) store->context;
}

static void persistent_usage(struct dm_exception_store *store,
			     sector_t *total_sectors,
			     sector_t *sectors_allocated,
			     sector_t *metadata_sectors)
{
	struct pstore *ps = get_info(store);

	*sectors_allocated = ps->next_free * store->chunk_size;
	*total_sectors = get_dev_size(dm_snap_cow(store->snap)->bdev);

	/*
	 * First chunk is the fixed header.
	 * Then there are (ps->current_area + 1) metadata chunks, each one
	 * separated from the next by ps->exceptions_per_area data chunks.
	 */
	*metadata_sectors = (ps->current_area + 1 + NUM_SNAPSHOT_HDR_CHUNKS) *
			    store->chunk_size;
}

static void persistent_dtr(struct dm_exception_store *store)
{
	struct pstore *ps = get_info(store);

	destroy_workqueue(ps->metadata_wq);

	/* Created in read_header */
	if (ps->io_client)
		dm_io_client_destroy(ps->io_client);
	free_area(ps);

	/* Allocated in persistent_read_metadata */
	if (ps->callbacks)
		vfree(ps->callbacks);

	kfree(ps);
}

static int persistent_read_metadata(struct dm_exception_store *store,
				    int (*callback)(void *callback_context,
						    chunk_t old, chunk_t new),
				    void *callback_context)
{
	int r, uninitialized_var(new_snapshot);
	struct pstore *ps = get_info(store);

	/*
	 * Read the snapshot header.
	 */
	r = read_header(ps, &new_snapshot);
	if (r)
		return r;

	/*
	 * Now we know correct chunk_size, complete the initialisation.
	 */
	ps->exceptions_per_area = (ps->store->chunk_size << SECTOR_SHIFT) /
				  sizeof(struct disk_exception);
	ps->callbacks = dm_vcalloc(ps->exceptions_per_area,
				   sizeof(*ps->callbacks));
	if (!ps->callbacks)
		return -ENOMEM;

	/*
	 * Do we need to setup a new snapshot ?
	 */
	if (new_snapshot) {
		r = write_header(ps);
		if (r) {
			DMWARN("write_header failed");
			return r;
		}

		ps->current_area = 0;
		zero_memory_area(ps);
		r = zero_disk_area(ps, 0);
		if (r)
			DMWARN("zero_disk_area(0) failed");
		return r;
	}
{
	struct pstore *ps = get_info(store);
	sector_t size = get_dev_size(dm_snap_cow(store->snap)->bdev);

	/* Is there enough room ? */
	if (size < ((ps->next_free + 1) * store->chunk_size))
		return -ENOSPC;

	e->new_chunk = ps->next_free;

	/*
	 * Move onto the next free pending, making sure to take
	 * into account the location of the metadata chunks.
	 */
	ps->next_free++;
	skip_metadata(ps);

	atomic_inc(&ps->pending_count);
	return 0;
}
{
	struct pstore *ps = get_info(store);
	sector_t size = get_dev_size(dm_snap_cow(store->snap)->bdev);

	/* Is there enough room ? */
	if (size < ((ps->next_free + 1) * store->chunk_size))
		return -ENOSPC;

	e->new_chunk = ps->next_free;

	/*
	 * Move onto the next free pending, making sure to take
	 * into account the location of the metadata chunks.
	 */
	ps->next_free++;
	skip_metadata(ps);

	atomic_inc(&ps->pending_count);
	return 0;
}

static void persistent_commit_exception(struct dm_exception_store *store,
					struct dm_exception *e,
					void (*callback) (void *, int success),
					void *callback_context)
{
	unsigned int i;
	struct pstore *ps = get_info(store);
	struct core_exception ce;
	struct commit_callback *cb;

	ce.old_chunk = e->old_chunk;
	ce.new_chunk = e->new_chunk;
	write_exception(ps, ps->current_committed++, &ce);

	/*
	 * Add the callback to the back of the array.  This code
	 * is the only place where the callback array is
	 * manipulated, and we know that it will never be called
	 * multiple times concurrently.
	 */
	cb = ps->callbacks + ps->callback_count++;
	cb->callback = callback;
	cb->context = callback_context;

	/*
	 * If there are exceptions in flight and we have not yet
	 * filled this metadata area there's nothing more to do.
	 */
	if (!atomic_dec_and_test(&ps->pending_count) &&
	    (ps->current_committed != ps->exceptions_per_area))
		return;

	/*
	 * If we completely filled the current area, then wipe the next one.
	 */
	if ((ps->current_committed == ps->exceptions_per_area) &&
	    zero_disk_area(ps, ps->current_area + 1))
		ps->valid = 0;

	/*
	 * Commit exceptions to disk.
	 */
	if (ps->valid && area_io(ps, WRITE_FLUSH_FUA))
		ps->valid = 0;

	/*
	 * Advance to the next area if this one is full.
	 */
	if (ps->current_committed == ps->exceptions_per_area) {
		ps->current_committed = 0;
		ps->current_area++;
		zero_memory_area(ps);
	}