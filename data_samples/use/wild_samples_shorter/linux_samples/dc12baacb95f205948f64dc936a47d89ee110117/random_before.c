	crng->init_time = jiffies - CRNG_RESEED_INTERVAL - 1;
}

static int crng_fast_load(const char *cp, size_t len)
{
	unsigned long flags;
	char *p;
	return 1;
}

static void crng_reseed(struct crng_state *crng, struct entropy_store *r)
{
	unsigned long	flags;
	int		i, num;
	unsigned long time = random_get_entropy() ^ jiffies;
	unsigned long flags;

	if (!crng_ready()) {
		crng_fast_load(buf, size);
		return;
	}

	trace_add_device_randomness(size, _RET_IP_);
	spin_lock_irqsave(&input_pool.lock, flags);
	_mix_pool_bytes(&input_pool, buf, size);