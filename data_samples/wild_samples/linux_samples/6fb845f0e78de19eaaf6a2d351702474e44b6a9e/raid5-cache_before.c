		if (rrdev) {
			atomic_inc(&rrdev->nr_pending);
			rcu_read_unlock();
			sync_page_io(rrdev, sh->sector, PAGE_SIZE,
				     sh->dev[disk_index].page, REQ_OP_WRITE, 0,
				     false);
			rdev_dec_pending(rrdev, rrdev->mddev);
			rcu_read_lock();
		}
		rcu_read_unlock();
	}
	ctx->data_parity_stripes++;
out:
	r5l_recovery_reset_stripe(sh);
}

static struct stripe_head *
r5c_recovery_alloc_stripe(struct r5conf *conf,
			  sector_t stripe_sect)
{
	struct stripe_head *sh;

	sh = raid5_get_active_stripe(conf, stripe_sect, 0, 1, 0);
	if (!sh)
		return NULL;  /* no more stripe available */

	r5l_recovery_reset_stripe(sh);

	return sh;
}
				if (sh) {
					WARN_ON(test_bit(STRIPE_R5C_CACHING, &sh->state));
					r5l_recovery_reset_stripe(sh);
					list_del_init(&sh->lru);
					raid5_release_stripe(sh);
				}
			}

			mb_offset += sizeof(struct r5l_payload_flush) +
				le32_to_cpu(payload_flush->size);
			continue;
		}

		/* DATA or PARITY payload */
		stripe_sect = (le16_to_cpu(payload->header.type) == R5LOG_PAYLOAD_DATA) ?
			raid5_compute_sector(
				conf, le64_to_cpu(payload->location), 0, &dd,
				NULL)
			: le64_to_cpu(payload->location);

		sh = r5c_recovery_lookup_stripe(cached_stripe_list,
						stripe_sect);

		if (!sh) {
			sh = r5c_recovery_alloc_stripe(conf, stripe_sect);
			/*
			 * cannot get stripe from raid5_get_active_stripe
			 * try replay some stripes
			 */
			if (!sh) {
				r5c_recovery_replay_stripes(
					cached_stripe_list, ctx);
				sh = r5c_recovery_alloc_stripe(
					conf, stripe_sect);
			}
			if (!sh) {
				pr_debug("md/raid:%s: Increasing stripe cache size to %d to recovery data on journal.\n",
					mdname(mddev),
					conf->min_nr_stripes * 2);
				raid5_set_cache_size(mddev,
						     conf->min_nr_stripes * 2);
				sh = r5c_recovery_alloc_stripe(conf,
							       stripe_sect);
			}
			if (!sh) {
				pr_err("md/raid:%s: Cannot get enough stripes due to memory pressure. Recovery failed.\n",
				       mdname(mddev));
				return -ENOMEM;
			}
			list_add_tail(&sh->lru, cached_stripe_list);
		}
			if (!sh) {
				r5c_recovery_replay_stripes(
					cached_stripe_list, ctx);
				sh = r5c_recovery_alloc_stripe(
					conf, stripe_sect);
			}
			if (!sh) {
				pr_debug("md/raid:%s: Increasing stripe cache size to %d to recovery data on journal.\n",
					mdname(mddev),
					conf->min_nr_stripes * 2);
				raid5_set_cache_size(mddev,
						     conf->min_nr_stripes * 2);
				sh = r5c_recovery_alloc_stripe(conf,
							       stripe_sect);
			}
			if (!sh) {
				pr_err("md/raid:%s: Cannot get enough stripes due to memory pressure. Recovery failed.\n",
				       mdname(mddev));
				return -ENOMEM;
			}
			list_add_tail(&sh->lru, cached_stripe_list);
		}

		if (le16_to_cpu(payload->header.type) == R5LOG_PAYLOAD_DATA) {
			if (!test_bit(STRIPE_R5C_CACHING, &sh->state) &&
			    test_bit(R5_Wantwrite, &sh->dev[sh->pd_idx].flags)) {
				r5l_recovery_replay_one_stripe(conf, sh, ctx);
				list_move_tail(&sh->lru, cached_stripe_list);
			}
			r5l_recovery_load_data(log, sh, ctx, payload,
					       log_offset);
		} else if (le16_to_cpu(payload->header.type) == R5LOG_PAYLOAD_PARITY)
			r5l_recovery_load_parity(log, sh, ctx, payload,
						 log_offset);
		else
			return -EINVAL;

		log_offset = r5l_ring_add(log, log_offset,
					  le32_to_cpu(payload->size));

		mb_offset += sizeof(struct r5l_payload_data_parity) +
			sizeof(__le32) *
			(le32_to_cpu(payload->size) >> (PAGE_SHIFT - 9));
	}

	return 0;
}

/*
 * Load the stripe into cache. The stripe will be written out later by
 * the stripe cache state machine.
 */
static void r5c_recovery_load_one_stripe(struct r5l_log *log,
					 struct stripe_head *sh)
{
	struct r5dev *dev;
	int i;

	for (i = sh->disks; i--; ) {
		dev = sh->dev + i;
		if (test_and_clear_bit(R5_Wantwrite, &dev->flags)) {
			set_bit(R5_InJournal, &dev->flags);
			set_bit(R5_UPTODATE, &dev->flags);
		}
	}
}

/*
 * Scan through the log for all to-be-flushed data
 *
 * For stripes with data and parity, namely Data-Parity stripe
 * (STRIPE_R5C_CACHING == 0), we simply replay all the writes.
 *
 * For stripes with only data, namely Data-Only stripe
 * (STRIPE_R5C_CACHING == 1), we load them to stripe cache state machine.
 *
 * For a stripe, if we see data after parity, we should discard all previous
 * data and parity for this stripe, as these data are already flushed to
 * the array.
 *
 * At the end of the scan, we return the new journal_tail, which points to
 * first data-only stripe on the journal device, or next invalid meta block.
 */
static int r5c_recovery_flush_log(struct r5l_log *log,
				  struct r5l_recovery_ctx *ctx)
{
	struct stripe_head *sh;
	int ret = 0;

	/* scan through the log */
	while (1) {
		if (r5l_recovery_read_meta_block(log, ctx))
			break;

		ret = r5c_recovery_analyze_meta_block(log, ctx,
						      &ctx->cached_list);
		/*
		 * -EAGAIN means mismatch in data block, in this case, we still
		 * try scan the next metablock
		 */
		if (ret && ret != -EAGAIN)
			break;   /* ret == -EINVAL or -ENOMEM */
		ctx->seq++;
		ctx->pos = r5l_ring_add(log, ctx->pos, ctx->meta_total_blocks);
	}

	if (ret == -ENOMEM) {
		r5c_recovery_drop_stripes(&ctx->cached_list, ctx);
		return ret;
	}

	/* replay data-parity stripes */
	r5c_recovery_replay_stripes(&ctx->cached_list, ctx);

	/* load data-only stripes to stripe cache */
	list_for_each_entry(sh, &ctx->cached_list, lru) {
		WARN_ON(!test_bit(STRIPE_R5C_CACHING, &sh->state));
		r5c_recovery_load_one_stripe(log, sh);
		ctx->data_only_stripes++;
	}

	return 0;
}

/*
 * we did a recovery. Now ctx.pos points to an invalid meta block. New
 * log will start here. but we can't let superblock point to last valid
 * meta block. The log might looks like:
 * | meta 1| meta 2| meta 3|
 * meta 1 is valid, meta 2 is invalid. meta 3 could be valid. If
 * superblock points to meta 1, we write a new valid meta 2n.  if crash
 * happens again, new recovery will start from meta 1. Since meta 2n is
 * valid now, recovery will think meta 3 is valid, which is wrong.
 * The solution is we create a new meta in meta2 with its seq == meta
 * 1's seq + 10000 and let superblock points to meta2. The same recovery
 * will not think meta 3 is a valid meta, because its seq doesn't match
 */

/*
 * Before recovery, the log looks like the following
 *
 *   ---------------------------------------------
 *   |           valid log        | invalid log  |
 *   ---------------------------------------------
 *   ^
 *   |- log->last_checkpoint
 *   |- log->last_cp_seq
 *
 * Now we scan through the log until we see invalid entry
 *
 *   ---------------------------------------------
 *   |           valid log        | invalid log  |
 *   ---------------------------------------------
 *   ^                            ^
 *   |- log->last_checkpoint      |- ctx->pos
 *   |- log->last_cp_seq          |- ctx->seq
 *
 * From this point, we need to increase seq number by 10 to avoid
 * confusing next recovery.
 *
 *   ---------------------------------------------
 *   |           valid log        | invalid log  |
 *   ---------------------------------------------
 *   ^                              ^
 *   |- log->last_checkpoint        |- ctx->pos+1
 *   |- log->last_cp_seq            |- ctx->seq+10001
 *
 * However, it is not safe to start the state machine yet, because data only
 * parities are not yet secured in RAID. To save these data only parities, we
 * rewrite them from seq+11.
 *
 *   -----------------------------------------------------------------
 *   |           valid log        | data only stripes | invalid log  |
 *   -----------------------------------------------------------------
 *   ^                                                ^
 *   |- log->last_checkpoint                          |- ctx->pos+n
 *   |- log->last_cp_seq                              |- ctx->seq+10000+n
 *
 * If failure happens again during this process, the recovery can safe start
 * again from log->last_checkpoint.
 *
 * Once data only stripes are rewritten to journal, we move log_tail
 *
 *   -----------------------------------------------------------------
 *   |     old log        |    data only stripes    | invalid log  |
 *   -----------------------------------------------------------------
 *                        ^                         ^
 *                        |- log->last_checkpoint   |- ctx->pos+n
 *                        |- log->last_cp_seq       |- ctx->seq+10000+n
 *
 * Then we can safely start the state machine. If failure happens from this
 * point on, the recovery will start from new log->last_checkpoint.
 */
static int
r5c_recovery_rewrite_data_only_stripes(struct r5l_log *log,
				       struct r5l_recovery_ctx *ctx)
{
	struct stripe_head *sh;
	struct mddev *mddev = log->rdev->mddev;
	struct page *page;
	sector_t next_checkpoint = MaxSector;

	page = alloc_page(GFP_KERNEL);
	if (!page) {
		pr_err("md/raid:%s: cannot allocate memory to rewrite data only stripes\n",
		       mdname(mddev));
		return -ENOMEM;
	}

	WARN_ON(list_empty(&ctx->cached_list));

	list_for_each_entry(sh, &ctx->cached_list, lru) {
		struct r5l_meta_block *mb;
		int i;
		int offset;
		sector_t write_pos;

		WARN_ON(!test_bit(STRIPE_R5C_CACHING, &sh->state));
		r5l_recovery_create_empty_meta_block(log, page,
						     ctx->pos, ctx->seq);
		mb = page_address(page);
		offset = le32_to_cpu(mb->meta_size);
		write_pos = r5l_ring_add(log, ctx->pos, BLOCK_SECTORS);

		for (i = sh->disks; i--; ) {
			struct r5dev *dev = &sh->dev[i];
			struct r5l_payload_data_parity *payload;
			void *addr;

			if (test_bit(R5_InJournal, &dev->flags)) {
				payload = (void *)mb + offset;
				payload->header.type = cpu_to_le16(
					R5LOG_PAYLOAD_DATA);
				payload->size = cpu_to_le32(BLOCK_SECTORS);
				payload->location = cpu_to_le64(
					raid5_compute_blocknr(sh, i, 0));
				addr = kmap_atomic(dev->page);
				payload->checksum[0] = cpu_to_le32(
					crc32c_le(log->uuid_checksum, addr,
						  PAGE_SIZE));
				kunmap_atomic(addr);
				sync_page_io(log->rdev, write_pos, PAGE_SIZE,
					     dev->page, REQ_OP_WRITE, 0, false);
				write_pos = r5l_ring_add(log, write_pos,
							 BLOCK_SECTORS);
				offset += sizeof(__le32) +
					sizeof(struct r5l_payload_data_parity);

			}