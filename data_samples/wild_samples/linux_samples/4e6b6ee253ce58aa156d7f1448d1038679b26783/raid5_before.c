{
	struct stripe_head *sh;
	int hash = (conf->max_nr_stripes - 1) % NR_STRIPE_HASH_LOCKS;

	spin_lock_irq(conf->hash_locks + hash);
	sh = get_free_stripe(conf, hash);
	spin_unlock_irq(conf->hash_locks + hash);
	if (!sh)
		return 0;
	BUG_ON(atomic_read(&sh->count));
	shrink_buffers(sh);
	kmem_cache_free(conf->slab_cache, sh);
	atomic_dec(&conf->active_stripes);
	conf->max_nr_stripes--;
	return 1;
}
	if (mutex_trylock(&conf->cache_size_mutex)) {
		ret= 0;
		while (ret < sc->nr_to_scan) {
			if (drop_one_stripe(conf) == 0) {
				ret = SHRINK_STOP;
				break;
			}
			ret++;
		}
		mutex_unlock(&conf->cache_size_mutex);
	}