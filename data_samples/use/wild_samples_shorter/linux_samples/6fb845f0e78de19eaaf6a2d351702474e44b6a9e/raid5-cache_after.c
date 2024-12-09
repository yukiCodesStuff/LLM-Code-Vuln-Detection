}

static struct stripe_head *
r5c_recovery_alloc_stripe(
		struct r5conf *conf,
		sector_t stripe_sect,
		int noblock)
{
	struct stripe_head *sh;

	sh = raid5_get_active_stripe(conf, stripe_sect, 0, noblock, 0);
	if (!sh)
		return NULL;  /* no more stripe available */

	r5l_recovery_reset_stripe(sh);
						stripe_sect);

		if (!sh) {
			sh = r5c_recovery_alloc_stripe(conf, stripe_sect, 1);
			/*
			 * cannot get stripe from raid5_get_active_stripe
			 * try replay some stripes
			 */
				r5c_recovery_replay_stripes(
					cached_stripe_list, ctx);
				sh = r5c_recovery_alloc_stripe(
					conf, stripe_sect, 1);
			}
			if (!sh) {
				int new_size = conf->min_nr_stripes * 2;
				pr_debug("md/raid:%s: Increasing stripe cache size to %d to recovery data on journal.\n",
					mdname(mddev),
					new_size);
				ret = raid5_set_cache_size(mddev, new_size);
				if (conf->min_nr_stripes <= new_size / 2) {
					pr_err("md/raid:%s: Cannot increase cache size, ret=%d, new_size=%d, min_nr_stripes=%d, max_nr_stripes=%d\n",
						mdname(mddev),
						ret,
						new_size,
						conf->min_nr_stripes,
						conf->max_nr_stripes);
					return -ENOMEM;
				}
				sh = r5c_recovery_alloc_stripe(
					conf, stripe_sect, 0);
			}
			if (!sh) {
				pr_err("md/raid:%s: Cannot get enough stripes due to memory pressure. Recovery failed.\n",
					mdname(mddev));
				return -ENOMEM;
			}
			list_add_tail(&sh->lru, cached_stripe_list);
		}