static int drop_one_stripe(struct r5conf *conf)
{
	struct stripe_head *sh;
	int hash = (conf->max_nr_stripes - 1) % NR_STRIPE_HASH_LOCKS;

	spin_lock_irq(conf->hash_locks + hash);
	sh = get_free_stripe(conf, hash);
	spin_unlock_irq(conf->hash_locks + hash);

	if (mutex_trylock(&conf->cache_size_mutex)) {
		ret= 0;
		while (ret < sc->nr_to_scan) {
			if (drop_one_stripe(conf) == 0) {
				ret = SHRINK_STOP;
				break;
			}