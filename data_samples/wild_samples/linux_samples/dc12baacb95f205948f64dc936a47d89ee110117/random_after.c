	for (i = 4; i < 16; i++) {
		if (!arch_get_random_seed_long(&rv) &&
		    !arch_get_random_long(&rv))
			rv = random_get_entropy();
		crng->state[i] ^= rv;
	}
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

	if (!spin_trylock_irqsave(&primary_crng.lock, flags))
		return 0;
	if (crng_init != 0) {
		spin_unlock_irqrestore(&primary_crng.lock, flags);
		return 0;
	}
	if (crng_init_cnt >= CRNG_INIT_CNT_THRESH) {
		invalidate_batched_entropy();
		crng_init = 1;
		wake_up_interruptible(&crng_init_wait);
		pr_notice("random: fast init done\n");
	}
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
{
	unsigned long time = random_get_entropy() ^ jiffies;
	unsigned long flags;

	if (!crng_ready() && size)
		crng_slow_load(buf, size);

	trace_add_device_randomness(size, _RET_IP_);
	spin_lock_irqsave(&input_pool.lock, flags);
	_mix_pool_bytes(&input_pool, buf, size);
	_mix_pool_bytes(&input_pool, &time, sizeof(time));
	spin_unlock_irqrestore(&input_pool.lock, flags);
}
EXPORT_SYMBOL(add_device_randomness);

static struct timer_rand_state input_timer_state = INIT_TIMER_RAND_STATE;

/*
 * This function adds entropy to the entropy "pool" by using timing
 * delays.  It uses the timer_rand_state structure to make an estimate
 * of how many bits of entropy this call has added to the pool.
 *
 * The number "num" is also added to the pool - it should somehow describe
 * the type of event which just happened.  This is currently 0-255 for
 * keyboard scan codes, and 256 upwards for interrupts.
 *
 */
static void add_timer_randomness(struct timer_rand_state *state, unsigned num)
{
	struct entropy_store	*r;
	struct {
		long jiffies;
		unsigned cycles;
		unsigned num;
	} sample;
	long delta, delta2, delta3;

	preempt_disable();

	sample.jiffies = jiffies;
	sample.cycles = random_get_entropy();
	sample.num = num;
	r = &input_pool;
	mix_pool_bytes(r, &sample, sizeof(sample));

	/*
	 * Calculate number of bits of randomness we probably added.
	 * We take into account the first, second and third-order deltas
	 * in order to make our estimate.
	 */
	delta = sample.jiffies - state->last_time;
	state->last_time = sample.jiffies;

	delta2 = delta - state->last_delta;
	state->last_delta = delta;

	delta3 = delta2 - state->last_delta2;
	state->last_delta2 = delta2;

	if (delta < 0)
		delta = -delta;
	if (delta2 < 0)
		delta2 = -delta2;
	if (delta3 < 0)
		delta3 = -delta3;
	if (delta > delta2)
		delta = delta2;
	if (delta > delta3)
		delta = delta3;

	/*
	 * delta is now minimum absolute delta.
	 * Round down by 1 bit on general principles,
	 * and limit entropy entimate to 12 bits.
	 */
	credit_entropy_bits(r, min_t(int, fls(delta>>1), 11));

	preempt_enable();
}