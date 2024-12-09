{
	struct mapped_device *md = io->md;
	struct bio *bio = io->orig_bio;
	unsigned long duration = jiffies - io->start_time;

	generic_end_io_acct(md->queue, bio_op(bio), &dm_disk(md)->part0,
			    io->start_time);

	if (unlikely(dm_stats_used(&md->stats)))
		dm_stats_account_io(&md->stats, bio_data_dir(bio),
				    bio->bi_iter.bi_sector, bio_sectors(bio),
				    true, duration, &io->stats_aux);

	/* nudge anyone waiting on suspend queue */
	if (unlikely(waitqueue_active(&md->wait)))
		wake_up(&md->wait);
}

/*
 * Add the bio to the list of deferred io.
 */
static void queue_io(struct mapped_device *md, struct bio *bio)
{
	unsigned long flags;

	spin_lock_irqsave(&md->deferred_lock, flags);
	bio_list_add(&md->deferred, bio);
	spin_unlock_irqrestore(&md->deferred_lock, flags);
	queue_work(md->wq, &md->work);
}

/*
 * Everyone (including functions in this file), should use this
 * function to access the md->map field, and make sure they call
 * dm_put_live_table() when finished.
 */
struct dm_table *dm_get_live_table(struct mapped_device *md, int *srcu_idx) __acquires(md->io_barrier)
{
	*srcu_idx = srcu_read_lock(&md->io_barrier);

	return srcu_dereference(md->map, &md->io_barrier);
}

void dm_put_live_table(struct mapped_device *md, int srcu_idx) __releases(md->io_barrier)
{
	srcu_read_unlock(&md->io_barrier, srcu_idx);
}

void dm_sync_table(struct mapped_device *md)
{
	synchronize_srcu(&md->io_barrier);
	synchronize_rcu_expedited();
}

/*
 * A fast alternative to dm_get_live_table/dm_put_live_table.
 * The caller must not block between these two functions.
 */
static struct dm_table *dm_get_live_table_fast(struct mapped_device *md) __acquires(RCU)
{
	rcu_read_lock();
	return rcu_dereference(md->map);
}

static void dm_put_live_table_fast(struct mapped_device *md) __releases(RCU)
{
	rcu_read_unlock();
}

static char *_dm_claim_ptr = "I belong to device-mapper";

/*
 * Open a table device so we can use it as a map destination.
 */
static int open_table_device(struct table_device *td, dev_t dev,
			     struct mapped_device *md)
{
	struct block_device *bdev;

	int r;

	BUG_ON(td->dm_dev.bdev);

	bdev = blkdev_get_by_dev(dev, td->dm_dev.mode | FMODE_EXCL, _dm_claim_ptr);
	if (IS_ERR(bdev))
		return PTR_ERR(bdev);

	r = bd_link_disk_holder(bdev, dm_disk(md));
	if (r) {
		blkdev_put(bdev, td->dm_dev.mode | FMODE_EXCL);
		return r;
	}
			     !dm_target_passes_integrity(tio->ti->type))) {
			DMWARN("%s: the target %s doesn't support integrity data.",
				dm_device_name(tio->io->md),
				tio->ti->type->name);
			return -EIO;
		}

		r = bio_integrity_clone(clone, bio, GFP_NOIO);
		if (r < 0)
			return r;
	}

	bio_trim(clone, sector - clone->bi_iter.bi_sector, len);

	return 0;
}

static void alloc_multiple_bios(struct bio_list *blist, struct clone_info *ci,
				struct dm_target *ti, unsigned num_bios)
{
	struct dm_target_io *tio;
	int try;

	if (!num_bios)
		return;

	if (num_bios == 1) {
		tio = alloc_tio(ci, ti, 0, GFP_NOIO);
		bio_list_add(blist, &tio->clone);
		return;
	}

	for (try = 0; try < 2; try++) {
		int bio_nr;
		struct bio *bio;

		if (try)
			mutex_lock(&ci->io->md->table_devices_lock);
		for (bio_nr = 0; bio_nr < num_bios; bio_nr++) {
			tio = alloc_tio(ci, ti, bio_nr, try ? GFP_NOIO : GFP_NOWAIT);
			if (!tio)
				break;

			bio_list_add(blist, &tio->clone);
		}