	crng->init_time = jiffies - CRNG_RESEED_INTERVAL - 1;
}

/*
 * crng_fast_load() can be called by code in the interrupt service
 * path.  So we can't afford to dilly-dally.
 */
static int crng_fast_load(const char *cp, size_t len)
{
	unsigned long flags;
	char *p;
	return 1;
}

/*
 * crng_slow_load() is called by add_device_randomness, which has two
 * attributes.  (1) We can't trust the buffer passed to it is
 * guaranteed to be unpredictable (so it might not have any entropy at
 * all), and (2) it doesn't have the performance constraints of
 * crng_fast_load().
 *
 * So we do something more comprehensive which is guaranteed to touch
 * all of the primary_crng's state, and which uses a LFSR with a
 * period of 255 as part of the mixing algorithm.  Finally, we do
 * *not* advance crng_init_cnt since buffer we may get may be something
 * like a fixed DMI table (for example), which might very well be
 * unique to the machine, but is otherwise unvarying.
 */
static int crng_slow_load(const char *cp, size_t len)
{
	unsigned long		flags;
	static unsigned char	lfsr = 1;
	unsigned char		tmp;
	unsigned		i, max = CHACHA20_KEY_SIZE;
	const char *		src_buf = cp;
	char *			dest_buf = (char *) &primary_crng.state[4];

	if (!spin_trylock_irqsave(&primary_crng.lock, flags))
		return 0;
	if (crng_init != 0) {
		spin_unlock_irqrestore(&primary_crng.lock, flags);
		return 0;
	}
	if (len > max)
		max = len;

	for (i = 0; i < max ; i++) {
		tmp = lfsr;
		lfsr >>= 1;
		if (tmp & 1)
			lfsr ^= 0xE1;
		tmp = dest_buf[i % CHACHA20_KEY_SIZE];
		dest_buf[i % CHACHA20_KEY_SIZE] ^= src_buf[i % len] ^ lfsr;
		lfsr += (tmp << 3) | (tmp >> 5);
	}
	spin_unlock_irqrestore(&primary_crng.lock, flags);
	return 1;
}

static void crng_reseed(struct crng_state *crng, struct entropy_store *r)
{
	unsigned long	flags;
	int		i, num;
	unsigned long time = random_get_entropy() ^ jiffies;
	unsigned long flags;

	if (!crng_ready() && size)
		crng_slow_load(buf, size);

	trace_add_device_randomness(size, _RET_IP_);
	spin_lock_irqsave(&input_pool.lock, flags);
	_mix_pool_bytes(&input_pool, buf, size);