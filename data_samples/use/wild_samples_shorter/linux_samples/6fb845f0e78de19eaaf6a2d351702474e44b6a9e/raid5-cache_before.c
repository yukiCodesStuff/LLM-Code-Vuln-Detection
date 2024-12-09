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
						stripe_sect);

		if (!sh) {
			sh = r5c_recovery_alloc_stripe(conf, stripe_sect);
			/*
			 * cannot get stripe from raid5_get_active_stripe
			 * try replay some stripes
			 */
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