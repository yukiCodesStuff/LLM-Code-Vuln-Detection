	for (i = 4; i < 16; i++) {
		if (!arch_get_random_seed_long(&rv) &&
		    !arch_get_random_long(&rv))
			rv = random_get_entropy();
		crng->state[i] ^= rv;
	}
	crng->init_time = jiffies - CRNG_RESEED_INTERVAL - 1;
}

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

static void crng_reseed(struct crng_state *crng, struct entropy_store *r)
{
	unsigned long	flags;
	int		i, num;
	union {
		__u32	block[CHACHA20_BLOCK_WORDS];
		__u32	key[8];
	} buf;

	if (r) {
		num = extract_entropy(r, &buf, 32, 16, 0);
		if (num == 0)
			return;
	} else {
		_extract_crng(&primary_crng, buf.block);
		_crng_backtrack_protect(&primary_crng, buf.block,
					CHACHA20_KEY_SIZE);
	}
	spin_lock_irqsave(&primary_crng.lock, flags);
	for (i = 0; i < 8; i++) {
		unsigned long	rv;
		if (!arch_get_random_seed_long(&rv) &&
		    !arch_get_random_long(&rv))
			rv = random_get_entropy();
		crng->state[i+4] ^= buf.key[i] ^ rv;
	}
	memzero_explicit(&buf, sizeof(buf));
	crng->init_time = jiffies;
	spin_unlock_irqrestore(&primary_crng.lock, flags);
	if (crng == &primary_crng && crng_init < 2) {
		invalidate_batched_entropy();
		crng_init = 2;
		process_random_ready_list();
		wake_up_interruptible(&crng_init_wait);
		pr_notice("random: crng init done\n");
	}
}

static void _extract_crng(struct crng_state *crng,
			  __u32 out[CHACHA20_BLOCK_WORDS])
{
	unsigned long v, flags;

	if (crng_ready() &&
	    time_after(jiffies, crng->init_time + CRNG_RESEED_INTERVAL))
		crng_reseed(crng, crng == &primary_crng ? &input_pool : NULL);
	spin_lock_irqsave(&crng->lock, flags);
	if (arch_get_random_long(&v))
		crng->state[14] ^= v;
	chacha20_block(&crng->state[0], out);
	if (crng->state[12] == 0)
		crng->state[13]++;
	spin_unlock_irqrestore(&crng->lock, flags);
}

static void extract_crng(__u32 out[CHACHA20_BLOCK_WORDS])
{
	struct crng_state *crng = NULL;

#ifdef CONFIG_NUMA
	if (crng_node_pool)
		crng = crng_node_pool[numa_node_id()];
	if (crng == NULL)
#endif
		crng = &primary_crng;
	_extract_crng(crng, out);
}

/*
 * Use the leftover bytes from the CRNG block output (if there is
 * enough) to mutate the CRNG key to provide backtracking protection.
 */
static void _crng_backtrack_protect(struct crng_state *crng,
				    __u32 tmp[CHACHA20_BLOCK_WORDS], int used)
{
	unsigned long	flags;
	__u32		*s, *d;
	int		i;

	used = round_up(used, sizeof(__u32));
	if (used + CHACHA20_KEY_SIZE > CHACHA20_BLOCK_SIZE) {
		extract_crng(tmp);
		used = 0;
	}
	spin_lock_irqsave(&crng->lock, flags);
	s = &tmp[used / sizeof(__u32)];
	d = &crng->state[4];
	for (i=0; i < 8; i++)
		*d++ ^= *s++;
	spin_unlock_irqrestore(&crng->lock, flags);
}

static void crng_backtrack_protect(__u32 tmp[CHACHA20_BLOCK_WORDS], int used)
{
	struct crng_state *crng = NULL;

#ifdef CONFIG_NUMA
	if (crng_node_pool)
		crng = crng_node_pool[numa_node_id()];
	if (crng == NULL)
#endif
		crng = &primary_crng;
	_crng_backtrack_protect(crng, tmp, used);
}

static ssize_t extract_crng_user(void __user *buf, size_t nbytes)
{
	ssize_t ret = 0, i = CHACHA20_BLOCK_SIZE;
	__u32 tmp[CHACHA20_BLOCK_WORDS];
	int large_request = (nbytes > 256);

	while (nbytes) {
		if (large_request && need_resched()) {
			if (signal_pending(current)) {
				if (ret == 0)
					ret = -ERESTARTSYS;
				break;
			}
			schedule();
		}

		extract_crng(tmp);
		i = min_t(int, nbytes, CHACHA20_BLOCK_SIZE);
		if (copy_to_user(buf, tmp, i)) {
			ret = -EFAULT;
			break;
		}

		nbytes -= i;
		buf += i;
		ret += i;
	}
	crng_backtrack_protect(tmp, i);

	/* Wipe data just written to memory */
	memzero_explicit(tmp, sizeof(tmp));

	return ret;
}


/*********************************************************************
 *
 * Entropy input management
 *
 *********************************************************************/

/* There is one of these per entropy source */
struct timer_rand_state {
	cycles_t last_time;
	long last_delta, last_delta2;
};

#define INIT_TIMER_RAND_STATE { INITIAL_JIFFIES, };

/*
 * Add device- or boot-specific data to the input pool to help
 * initialize it.
 *
 * None of this adds any entropy; it is meant to avoid the problem of
 * the entropy pool having similar initial state across largely
 * identical devices.
 */
void add_device_randomness(const void *buf, unsigned int size)
{
	unsigned long time = random_get_entropy() ^ jiffies;
	unsigned long flags;

	if (!crng_ready()) {
		crng_fast_load(buf, size);
		return;
	}

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

void add_input_randomness(unsigned int type, unsigned int code,
				 unsigned int value)
{
	static unsigned char last_value;

	/* ignore autorepeat and the like */
	if (value == last_value)
		return;

	last_value = value;
	add_timer_randomness(&input_timer_state,
			     (type << 4) ^ code ^ (code >> 4) ^ value);
	trace_add_input_randomness(ENTROPY_BITS(&input_pool));
}
EXPORT_SYMBOL_GPL(add_input_randomness);

static DEFINE_PER_CPU(struct fast_pool, irq_randomness);

#ifdef ADD_INTERRUPT_BENCH
static unsigned long avg_cycles, avg_deviation;

#define AVG_SHIFT 8     /* Exponential average factor k=1/256 */
#define FIXED_1_2 (1 << (AVG_SHIFT-1))

static void add_interrupt_bench(cycles_t start)
{
        long delta = random_get_entropy() - start;

        /* Use a weighted moving average */
        delta = delta - ((avg_cycles + FIXED_1_2) >> AVG_SHIFT);
        avg_cycles += delta;
        /* And average deviation */
        delta = abs(delta) - ((avg_deviation + FIXED_1_2) >> AVG_SHIFT);
        avg_deviation += delta;
}
#else
#define add_interrupt_bench(x)
#endif

static __u32 get_reg(struct fast_pool *f, struct pt_regs *regs)
{
	__u32 *ptr = (__u32 *) regs;
	unsigned int idx;

	if (regs == NULL)
		return 0;
	idx = READ_ONCE(f->reg_idx);
	if (idx >= sizeof(struct pt_regs) / sizeof(__u32))
		idx = 0;
	ptr += idx++;
	WRITE_ONCE(f->reg_idx, idx);
	return *ptr;
}

void add_interrupt_randomness(int irq, int irq_flags)
{
	struct entropy_store	*r;
	struct fast_pool	*fast_pool = this_cpu_ptr(&irq_randomness);
	struct pt_regs		*regs = get_irq_regs();
	unsigned long		now = jiffies;
	cycles_t		cycles = random_get_entropy();
	__u32			c_high, j_high;
	__u64			ip;
	unsigned long		seed;
	int			credit = 0;

	if (cycles == 0)
		cycles = get_reg(fast_pool, regs);
	c_high = (sizeof(cycles) > 4) ? cycles >> 32 : 0;
	j_high = (sizeof(now) > 4) ? now >> 32 : 0;
	fast_pool->pool[0] ^= cycles ^ j_high ^ irq;
	fast_pool->pool[1] ^= now ^ c_high;
	ip = regs ? instruction_pointer(regs) : _RET_IP_;
	fast_pool->pool[2] ^= ip;
	fast_pool->pool[3] ^= (sizeof(ip) > 4) ? ip >> 32 :
		get_reg(fast_pool, regs);

	fast_mix(fast_pool);
	add_interrupt_bench(cycles);

	if (unlikely(crng_init == 0)) {
		if ((fast_pool->count >= 64) &&
		    crng_fast_load((char *) fast_pool->pool,
				   sizeof(fast_pool->pool))) {
			fast_pool->count = 0;
			fast_pool->last = now;
		}
		return;
	}

	if ((fast_pool->count < 64) &&
	    !time_after(now, fast_pool->last + HZ))
		return;

	r = &input_pool;
	if (!spin_trylock(&r->lock))
		return;

	fast_pool->last = now;
	__mix_pool_bytes(r, &fast_pool->pool, sizeof(fast_pool->pool));

	/*
	 * If we have architectural seed generator, produce a seed and
	 * add it to the pool.  For the sake of paranoia don't let the
	 * architectural seed generator dominate the input from the
	 * interrupt noise.
	 */
	if (arch_get_random_seed_long(&seed)) {
		__mix_pool_bytes(r, &seed, sizeof(seed));
		credit = 1;
	}
	spin_unlock(&r->lock);

	fast_pool->count = 0;

	/* award one bit for the contents of the fast pool */
	credit_entropy_bits(r, credit + 1);
}
EXPORT_SYMBOL_GPL(add_interrupt_randomness);

#ifdef CONFIG_BLOCK
void add_disk_randomness(struct gendisk *disk)
{
	if (!disk || !disk->random)
		return;
	/* first major is 1, so we get >= 0x200 here */
	add_timer_randomness(disk->random, 0x100 + disk_devt(disk));
	trace_add_disk_randomness(disk_devt(disk), ENTROPY_BITS(&input_pool));
}
EXPORT_SYMBOL_GPL(add_disk_randomness);
#endif

/*********************************************************************
 *
 * Entropy extraction routines
 *
 *********************************************************************/

/*
 * This utility inline function is responsible for transferring entropy
 * from the primary pool to the secondary extraction pool. We make
 * sure we pull enough for a 'catastrophic reseed'.
 */
static void _xfer_secondary_pool(struct entropy_store *r, size_t nbytes);
static void xfer_secondary_pool(struct entropy_store *r, size_t nbytes)
{
	if (!r->pull ||
	    r->entropy_count >= (nbytes << (ENTROPY_SHIFT + 3)) ||
	    r->entropy_count > r->poolinfo->poolfracbits)
		return;

	_xfer_secondary_pool(r, nbytes);
}

static void _xfer_secondary_pool(struct entropy_store *r, size_t nbytes)
{
	__u32	tmp[OUTPUT_POOL_WORDS];

	int bytes = nbytes;

	/* pull at least as much as a wakeup */
	bytes = max_t(int, bytes, random_read_wakeup_bits / 8);
	/* but never more than the buffer size */
	bytes = min_t(int, bytes, sizeof(tmp));

	trace_xfer_secondary_pool(r->name, bytes * 8, nbytes * 8,
				  ENTROPY_BITS(r), ENTROPY_BITS(r->pull));
	bytes = extract_entropy(r->pull, tmp, bytes,
				random_read_wakeup_bits / 8, 0);
	mix_pool_bytes(r, tmp, bytes);
	credit_entropy_bits(r, bytes*8);
}

/*
 * Used as a workqueue function so that when the input pool is getting
 * full, we can "spill over" some entropy to the output pools.  That
 * way the output pools can store some of the excess entropy instead
 * of letting it go to waste.
 */
static void push_to_pool(struct work_struct *work)
{
	struct entropy_store *r = container_of(work, struct entropy_store,
					      push_work);
	BUG_ON(!r);
	_xfer_secondary_pool(r, random_read_wakeup_bits/8);
	trace_push_to_pool(r->name, r->entropy_count >> ENTROPY_SHIFT,
			   r->pull->entropy_count >> ENTROPY_SHIFT);
}

/*
 * This function decides how many bytes to actually take from the
 * given pool, and also debits the entropy count accordingly.
 */
static size_t account(struct entropy_store *r, size_t nbytes, int min,
		      int reserved)
{
	int entropy_count, orig, have_bytes;
	size_t ibytes, nfrac;

	BUG_ON(r->entropy_count > r->poolinfo->poolfracbits);

	/* Can we pull enough? */
retry:
	entropy_count = orig = READ_ONCE(r->entropy_count);
	ibytes = nbytes;
	/* never pull more than available */
	have_bytes = entropy_count >> (ENTROPY_SHIFT + 3);

	if ((have_bytes -= reserved) < 0)
		have_bytes = 0;
	ibytes = min_t(size_t, ibytes, have_bytes);
	if (ibytes < min)
		ibytes = 0;

	if (unlikely(entropy_count < 0)) {
		pr_warn("random: negative entropy count: pool %s count %d\n",
			r->name, entropy_count);
		WARN_ON(1);
		entropy_count = 0;
	}
	nfrac = ibytes << (ENTROPY_SHIFT + 3);
	if ((size_t) entropy_count > nfrac)
		entropy_count -= nfrac;
	else
		entropy_count = 0;

	if (cmpxchg(&r->entropy_count, orig, entropy_count) != orig)
		goto retry;

	trace_debit_entropy(r->name, 8 * ibytes);
	if (ibytes &&
	    (r->entropy_count >> ENTROPY_SHIFT) < random_write_wakeup_bits) {
		wake_up_interruptible(&random_write_wait);
		kill_fasync(&fasync, SIGIO, POLL_OUT);
	}

	return ibytes;
}

/*
 * This function does the actual extraction for extract_entropy and
 * extract_entropy_user.
 *
 * Note: we assume that .poolwords is a multiple of 16 words.
 */
static void extract_buf(struct entropy_store *r, __u8 *out)
{
	int i;
	union {
		__u32 w[5];
		unsigned long l[LONGS(20)];
	} hash;
	__u32 workspace[SHA_WORKSPACE_WORDS];
	unsigned long flags;

	/*
	 * If we have an architectural hardware random number
	 * generator, use it for SHA's initial vector
	 */
	sha_init(hash.w);
	for (i = 0; i < LONGS(20); i++) {
		unsigned long v;
		if (!arch_get_random_long(&v))
			break;
		hash.l[i] = v;
	}

	/* Generate a hash across the pool, 16 words (512 bits) at a time */
	spin_lock_irqsave(&r->lock, flags);
	for (i = 0; i < r->poolinfo->poolwords; i += 16)
		sha_transform(hash.w, (__u8 *)(r->pool + i), workspace);

	/*
	 * We mix the hash back into the pool to prevent backtracking
	 * attacks (where the attacker knows the state of the pool
	 * plus the current outputs, and attempts to find previous
	 * ouputs), unless the hash function can be inverted. By
	 * mixing at least a SHA1 worth of hash data back, we make
	 * brute-forcing the feedback as hard as brute-forcing the
	 * hash.
	 */
	__mix_pool_bytes(r, hash.w, sizeof(hash.w));
	spin_unlock_irqrestore(&r->lock, flags);

	memzero_explicit(workspace, sizeof(workspace));

	/*
	 * In case the hash function has some recognizable output
	 * pattern, we fold it in half. Thus, we always feed back
	 * twice as much data as we output.
	 */
	hash.w[0] ^= hash.w[3];
	hash.w[1] ^= hash.w[4];
	hash.w[2] ^= rol32(hash.w[2], 16);

	memcpy(out, &hash, EXTRACT_SIZE);
	memzero_explicit(&hash, sizeof(hash));
}

static ssize_t _extract_entropy(struct entropy_store *r, void *buf,
				size_t nbytes, int fips)
{
	ssize_t ret = 0, i;
	__u8 tmp[EXTRACT_SIZE];
	unsigned long flags;

	while (nbytes) {
		extract_buf(r, tmp);

		if (fips) {
			spin_lock_irqsave(&r->lock, flags);
			if (!memcmp(tmp, r->last_data, EXTRACT_SIZE))
				panic("Hardware RNG duplicated output!\n");
			memcpy(r->last_data, tmp, EXTRACT_SIZE);
			spin_unlock_irqrestore(&r->lock, flags);
		}
		i = min_t(int, nbytes, EXTRACT_SIZE);
		memcpy(buf, tmp, i);
		nbytes -= i;
		buf += i;
		ret += i;
	}

	/* Wipe data just returned from memory */
	memzero_explicit(tmp, sizeof(tmp));

	return ret;
}

/*
 * This function extracts randomness from the "entropy pool", and
 * returns it in a buffer.
 *
 * The min parameter specifies the minimum amount we can pull before
 * failing to avoid races that defeat catastrophic reseeding while the
 * reserved parameter indicates how much entropy we must leave in the
 * pool after each pull to avoid starving other readers.
 */
static ssize_t extract_entropy(struct entropy_store *r, void *buf,
				 size_t nbytes, int min, int reserved)
{
	__u8 tmp[EXTRACT_SIZE];
	unsigned long flags;

	/* if last_data isn't primed, we need EXTRACT_SIZE extra bytes */
	if (fips_enabled) {
		spin_lock_irqsave(&r->lock, flags);
		if (!r->last_data_init) {
			r->last_data_init = 1;
			spin_unlock_irqrestore(&r->lock, flags);
			trace_extract_entropy(r->name, EXTRACT_SIZE,
					      ENTROPY_BITS(r), _RET_IP_);
			xfer_secondary_pool(r, EXTRACT_SIZE);
			extract_buf(r, tmp);
			spin_lock_irqsave(&r->lock, flags);
			memcpy(r->last_data, tmp, EXTRACT_SIZE);
		}
		spin_unlock_irqrestore(&r->lock, flags);
	}

	trace_extract_entropy(r->name, nbytes, ENTROPY_BITS(r), _RET_IP_);
	xfer_secondary_pool(r, nbytes);
	nbytes = account(r, nbytes, min, reserved);

	return _extract_entropy(r, buf, nbytes, fips_enabled);
}

/*
 * This function extracts randomness from the "entropy pool", and
 * returns it in a userspace buffer.
 */
static ssize_t extract_entropy_user(struct entropy_store *r, void __user *buf,
				    size_t nbytes)
{
	ssize_t ret = 0, i;
	__u8 tmp[EXTRACT_SIZE];
	int large_request = (nbytes > 256);

	trace_extract_entropy_user(r->name, nbytes, ENTROPY_BITS(r), _RET_IP_);
	xfer_secondary_pool(r, nbytes);
	nbytes = account(r, nbytes, 0, 0);

	while (nbytes) {
		if (large_request && need_resched()) {
			if (signal_pending(current)) {
				if (ret == 0)
					ret = -ERESTARTSYS;
				break;
			}
			schedule();
		}

		extract_buf(r, tmp);
		i = min_t(int, nbytes, EXTRACT_SIZE);
		if (copy_to_user(buf, tmp, i)) {
			ret = -EFAULT;
			break;
		}

		nbytes -= i;
		buf += i;
		ret += i;
	}

	/* Wipe data just returned from memory */
	memzero_explicit(tmp, sizeof(tmp));

	return ret;
}

#define warn_unseeded_randomness(previous) \
	_warn_unseeded_randomness(__func__, (void *) _RET_IP_, (previous))

static void _warn_unseeded_randomness(const char *func_name, void *caller,
				      void **previous)
{
#ifdef CONFIG_WARN_ALL_UNSEEDED_RANDOM
	const bool print_once = false;
#else
	static bool print_once __read_mostly;
#endif

	if (print_once ||
	    crng_ready() ||
	    (previous && (caller == READ_ONCE(*previous))))
		return;
	WRITE_ONCE(*previous, caller);
#ifndef CONFIG_WARN_ALL_UNSEEDED_RANDOM
	print_once = true;
#endif
	pr_notice("random: %s called from %pS with crng_init=%d\n",
		  func_name, caller, crng_init);
}

/*
 * This function is the exported kernel interface.  It returns some
 * number of good random numbers, suitable for key generation, seeding
 * TCP sequence numbers, etc.  It does not rely on the hardware random
 * number generator.  For random bytes direct from the hardware RNG
 * (when available), use get_random_bytes_arch(). In order to ensure
 * that the randomness provided by this function is okay, the function
 * wait_for_random_bytes() should be called and return 0 at least once
 * at any point prior.
 */
static void _get_random_bytes(void *buf, int nbytes)
{
	__u32 tmp[CHACHA20_BLOCK_WORDS];

	trace_get_random_bytes(nbytes, _RET_IP_);

	while (nbytes >= CHACHA20_BLOCK_SIZE) {
		extract_crng(buf);
		buf += CHACHA20_BLOCK_SIZE;
		nbytes -= CHACHA20_BLOCK_SIZE;
	}

	if (nbytes > 0) {
		extract_crng(tmp);
		memcpy(buf, tmp, nbytes);
		crng_backtrack_protect(tmp, nbytes);
	} else
		crng_backtrack_protect(tmp, CHACHA20_BLOCK_SIZE);
	memzero_explicit(tmp, sizeof(tmp));
}

void get_random_bytes(void *buf, int nbytes)
{
	static void *previous;

	warn_unseeded_randomness(&previous);
	_get_random_bytes(buf, nbytes);
}
EXPORT_SYMBOL(get_random_bytes);

/*
 * Wait for the urandom pool to be seeded and thus guaranteed to supply
 * cryptographically secure random numbers. This applies to: the /dev/urandom
 * device, the get_random_bytes function, and the get_random_{u32,u64,int,long}
 * family of functions. Using any of these functions without first calling
 * this function forfeits the guarantee of security.
 *
 * Returns: 0 if the urandom pool has been seeded.
 *          -ERESTARTSYS if the function was interrupted by a signal.
 */
int wait_for_random_bytes(void)
{
	if (likely(crng_ready()))
		return 0;
	return wait_event_interruptible(crng_init_wait, crng_ready());
}
EXPORT_SYMBOL(wait_for_random_bytes);

/*
 * Add a callback function that will be invoked when the nonblocking
 * pool is initialised.
 *
 * returns: 0 if callback is successfully added
 *	    -EALREADY if pool is already initialised (callback not called)
 *	    -ENOENT if module for callback is not alive
 */
int add_random_ready_callback(struct random_ready_callback *rdy)
{
	struct module *owner;
	unsigned long flags;
	int err = -EALREADY;

	if (crng_ready())
		return err;

	owner = rdy->owner;
	if (!try_module_get(owner))
		return -ENOENT;

	spin_lock_irqsave(&random_ready_list_lock, flags);
	if (crng_ready())
		goto out;

	owner = NULL;

	list_add(&rdy->list, &random_ready_list);
	err = 0;

out:
	spin_unlock_irqrestore(&random_ready_list_lock, flags);

	module_put(owner);

	return err;
}
EXPORT_SYMBOL(add_random_ready_callback);

/*
 * Delete a previously registered readiness callback function.
 */
void del_random_ready_callback(struct random_ready_callback *rdy)
{
	unsigned long flags;
	struct module *owner = NULL;

	spin_lock_irqsave(&random_ready_list_lock, flags);
	if (!list_empty(&rdy->list)) {
		list_del_init(&rdy->list);
		owner = rdy->owner;
	}
	spin_unlock_irqrestore(&random_ready_list_lock, flags);

	module_put(owner);
}
EXPORT_SYMBOL(del_random_ready_callback);

/*
 * This function will use the architecture-specific hardware random
 * number generator if it is available.  The arch-specific hw RNG will
 * almost certainly be faster than what we can do in software, but it
 * is impossible to verify that it is implemented securely (as
 * opposed, to, say, the AES encryption of a sequence number using a
 * key known by the NSA).  So it's useful if we need the speed, but
 * only if we're willing to trust the hardware manufacturer not to
 * have put in a back door.
 */
void get_random_bytes_arch(void *buf, int nbytes)
{
	char *p = buf;

	trace_get_random_bytes_arch(nbytes, _RET_IP_);
	while (nbytes) {
		unsigned long v;
		int chunk = min(nbytes, (int)sizeof(unsigned long));

		if (!arch_get_random_long(&v))
			break;
		
		memcpy(p, &v, chunk);
		p += chunk;
		nbytes -= chunk;
	}

	if (nbytes)
		get_random_bytes(p, nbytes);
}
EXPORT_SYMBOL(get_random_bytes_arch);


/*
 * init_std_data - initialize pool with system data
 *
 * @r: pool to initialize
 *
 * This function clears the pool's entropy count and mixes some system
 * data into the pool to prepare it for use. The pool is not cleared
 * as that can only decrease the entropy in the pool.
 */
static void init_std_data(struct entropy_store *r)
{
	int i;
	ktime_t now = ktime_get_real();
	unsigned long rv;

	r->last_pulled = jiffies;
	mix_pool_bytes(r, &now, sizeof(now));
	for (i = r->poolinfo->poolbytes; i > 0; i -= sizeof(rv)) {
		if (!arch_get_random_seed_long(&rv) &&
		    !arch_get_random_long(&rv))
			rv = random_get_entropy();
		mix_pool_bytes(r, &rv, sizeof(rv));
	}
	mix_pool_bytes(r, utsname(), sizeof(*(utsname())));
}

/*
 * Note that setup_arch() may call add_device_randomness()
 * long before we get here. This allows seeding of the pools
 * with some platform dependent data very early in the boot
 * process. But it limits our options here. We must use
 * statically allocated structures that already have all
 * initializations complete at compile time. We should also
 * take care not to overwrite the precious per platform data
 * we were given.
 */
static int rand_initialize(void)
{
#ifdef CONFIG_NUMA
	int i;
	struct crng_state *crng;
	struct crng_state **pool;
#endif

	init_std_data(&input_pool);
	init_std_data(&blocking_pool);
	crng_initialize(&primary_crng);

#ifdef CONFIG_NUMA
	pool = kcalloc(nr_node_ids, sizeof(*pool), GFP_KERNEL|__GFP_NOFAIL);
	for_each_online_node(i) {
		crng = kmalloc_node(sizeof(struct crng_state),
				    GFP_KERNEL | __GFP_NOFAIL, i);
		spin_lock_init(&crng->lock);
		crng_initialize(crng);
		pool[i] = crng;
	}
	mb();
	crng_node_pool = pool;
#endif
	return 0;
}
early_initcall(rand_initialize);

#ifdef CONFIG_BLOCK
void rand_initialize_disk(struct gendisk *disk)
{
	struct timer_rand_state *state;

	/*
	 * If kzalloc returns null, we just won't use that entropy
	 * source.
	 */
	state = kzalloc(sizeof(struct timer_rand_state), GFP_KERNEL);
	if (state) {
		state->last_time = INITIAL_JIFFIES;
		disk->random = state;
	}
}
#endif

static ssize_t
_random_read(int nonblock, char __user *buf, size_t nbytes)
{
	ssize_t n;

	if (nbytes == 0)
		return 0;

	nbytes = min_t(size_t, nbytes, SEC_XFER_SIZE);
	while (1) {
		n = extract_entropy_user(&blocking_pool, buf, nbytes);
		if (n < 0)
			return n;
		trace_random_read(n*8, (nbytes-n)*8,
				  ENTROPY_BITS(&blocking_pool),
				  ENTROPY_BITS(&input_pool));
		if (n > 0)
			return n;

		/* Pool is (near) empty.  Maybe wait and retry. */
		if (nonblock)
			return -EAGAIN;

		wait_event_interruptible(random_read_wait,
			ENTROPY_BITS(&input_pool) >=
			random_read_wakeup_bits);
		if (signal_pending(current))
			return -ERESTARTSYS;
	}
}

static ssize_t
random_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	return _random_read(file->f_flags & O_NONBLOCK, buf, nbytes);
}

static ssize_t
urandom_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	unsigned long flags;
	static int maxwarn = 10;
	int ret;

	if (!crng_ready() && maxwarn > 0) {
		maxwarn--;
		printk(KERN_NOTICE "random: %s: uninitialized urandom read "
		       "(%zd bytes read)\n",
		       current->comm, nbytes);
		spin_lock_irqsave(&primary_crng.lock, flags);
		crng_init_cnt = 0;
		spin_unlock_irqrestore(&primary_crng.lock, flags);
	}
	nbytes = min_t(size_t, nbytes, INT_MAX >> (ENTROPY_SHIFT + 3));
	ret = extract_crng_user(buf, nbytes);
	trace_urandom_read(8 * nbytes, 0, ENTROPY_BITS(&input_pool));
	return ret;
}

static __poll_t
random_poll(struct file *file, poll_table * wait)
{
	__poll_t mask;

	poll_wait(file, &random_read_wait, wait);
	poll_wait(file, &random_write_wait, wait);
	mask = 0;
	if (ENTROPY_BITS(&input_pool) >= random_read_wakeup_bits)
		mask |= EPOLLIN | EPOLLRDNORM;
	if (ENTROPY_BITS(&input_pool) < random_write_wakeup_bits)
		mask |= EPOLLOUT | EPOLLWRNORM;
	return mask;
}

static int
write_pool(struct entropy_store *r, const char __user *buffer, size_t count)
{
	size_t bytes;
	__u32 buf[16];
	const char __user *p = buffer;

	while (count > 0) {
		bytes = min(count, sizeof(buf));
		if (copy_from_user(&buf, p, bytes))
			return -EFAULT;

		count -= bytes;
		p += bytes;

		mix_pool_bytes(r, buf, bytes);
		cond_resched();
	}

	return 0;
}

static ssize_t random_write(struct file *file, const char __user *buffer,
			    size_t count, loff_t *ppos)
{
	size_t ret;

	ret = write_pool(&input_pool, buffer, count);
	if (ret)
		return ret;

	return (ssize_t)count;
}

static long random_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	int size, ent_count;
	int __user *p = (int __user *)arg;
	int retval;

	switch (cmd) {
	case RNDGETENTCNT:
		/* inherently racy, no point locking */
		ent_count = ENTROPY_BITS(&input_pool);
		if (put_user(ent_count, p))
			return -EFAULT;
		return 0;
	case RNDADDTOENTCNT:
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		if (get_user(ent_count, p))
			return -EFAULT;
		return credit_entropy_bits_safe(&input_pool, ent_count);
	case RNDADDENTROPY:
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		if (get_user(ent_count, p++))
			return -EFAULT;
		if (ent_count < 0)
			return -EINVAL;
		if (get_user(size, p++))
			return -EFAULT;
		retval = write_pool(&input_pool, (const char __user *)p,
				    size);
		if (retval < 0)
			return retval;
		return credit_entropy_bits_safe(&input_pool, ent_count);
	case RNDZAPENTCNT:
	case RNDCLEARPOOL:
		/*
		 * Clear the entropy pool counters. We no longer clear
		 * the entropy pool, as that's silly.
		 */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		input_pool.entropy_count = 0;
		blocking_pool.entropy_count = 0;
		return 0;
	default:
		return -EINVAL;
	}
}

static int random_fasync(int fd, struct file *filp, int on)
{
	return fasync_helper(fd, filp, on, &fasync);
}

const struct file_operations random_fops = {
	.read  = random_read,
	.write = random_write,
	.poll  = random_poll,
	.unlocked_ioctl = random_ioctl,
	.fasync = random_fasync,
	.llseek = noop_llseek,
};

const struct file_operations urandom_fops = {
	.read  = urandom_read,
	.write = random_write,
	.unlocked_ioctl = random_ioctl,
	.fasync = random_fasync,
	.llseek = noop_llseek,
};

SYSCALL_DEFINE3(getrandom, char __user *, buf, size_t, count,
		unsigned int, flags)
{
	int ret;

	if (flags & ~(GRND_NONBLOCK|GRND_RANDOM))
		return -EINVAL;

	if (count > INT_MAX)
		count = INT_MAX;

	if (flags & GRND_RANDOM)
		return _random_read(flags & GRND_NONBLOCK, buf, count);

	if (!crng_ready()) {
		if (flags & GRND_NONBLOCK)
			return -EAGAIN;
		ret = wait_for_random_bytes();
		if (unlikely(ret))
			return ret;
	}
	return urandom_read(NULL, buf, count, NULL);
}

/********************************************************************
 *
 * Sysctl interface
 *
 ********************************************************************/

#ifdef CONFIG_SYSCTL

#include <linux/sysctl.h>

static int min_read_thresh = 8, min_write_thresh;
static int max_read_thresh = OUTPUT_POOL_WORDS * 32;
static int max_write_thresh = INPUT_POOL_WORDS * 32;
static int random_min_urandom_seed = 60;
static char sysctl_bootid[16];

/*
 * This function is used to return both the bootid UUID, and random
 * UUID.  The difference is in whether table->data is NULL; if it is,
 * then a new UUID is generated and returned to the user.
 *
 * If the user accesses this via the proc interface, the UUID will be
 * returned as an ASCII string in the standard UUID format; if via the
 * sysctl system call, as 16 bytes of binary data.
 */
static int proc_do_uuid(struct ctl_table *table, int write,
			void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table fake_table;
	unsigned char buf[64], tmp_uuid[16], *uuid;

	uuid = table->data;
	if (!uuid) {
		uuid = tmp_uuid;
		generate_random_uuid(uuid);
	} else {
		static DEFINE_SPINLOCK(bootid_spinlock);

		spin_lock(&bootid_spinlock);
		if (!uuid[8])
			generate_random_uuid(uuid);
		spin_unlock(&bootid_spinlock);
	}

	sprintf(buf, "%pU", uuid);

	fake_table.data = buf;
	fake_table.maxlen = sizeof(buf);

	return proc_dostring(&fake_table, write, buffer, lenp, ppos);
}

/*
 * Return entropy available scaled to integral bits
 */
static int proc_do_entropy(struct ctl_table *table, int write,
			   void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table fake_table;
	int entropy_count;

	entropy_count = *(int *)table->data >> ENTROPY_SHIFT;

	fake_table.data = &entropy_count;
	fake_table.maxlen = sizeof(entropy_count);

	return proc_dointvec(&fake_table, write, buffer, lenp, ppos);
}

static int sysctl_poolsize = INPUT_POOL_WORDS * 32;
extern struct ctl_table random_table[];
struct ctl_table random_table[] = {
	{
		.procname	= "poolsize",
		.data		= &sysctl_poolsize,
		.maxlen		= sizeof(int),
		.mode		= 0444,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "entropy_avail",
		.maxlen		= sizeof(int),
		.mode		= 0444,
		.proc_handler	= proc_do_entropy,
		.data		= &input_pool.entropy_count,
	},
	{
		.procname	= "read_wakeup_threshold",
		.data		= &random_read_wakeup_bits,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &min_read_thresh,
		.extra2		= &max_read_thresh,
	},
	{
		.procname	= "write_wakeup_threshold",
		.data		= &random_write_wakeup_bits,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &min_write_thresh,
		.extra2		= &max_write_thresh,
	},
	{
		.procname	= "urandom_min_reseed_secs",
		.data		= &random_min_urandom_seed,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "boot_id",
		.data		= &sysctl_bootid,
		.maxlen		= 16,
		.mode		= 0444,
		.proc_handler	= proc_do_uuid,
	},
	{
		.procname	= "uuid",
		.maxlen		= 16,
		.mode		= 0444,
		.proc_handler	= proc_do_uuid,
	},
#ifdef ADD_INTERRUPT_BENCH
	{
		.procname	= "add_interrupt_avg_cycles",
		.data		= &avg_cycles,
		.maxlen		= sizeof(avg_cycles),
		.mode		= 0444,
		.proc_handler	= proc_doulongvec_minmax,
	},
	{
		.procname	= "add_interrupt_avg_deviation",
		.data		= &avg_deviation,
		.maxlen		= sizeof(avg_deviation),
		.mode		= 0444,
		.proc_handler	= proc_doulongvec_minmax,
	},
#endif
	{ }
};
#endif 	/* CONFIG_SYSCTL */

struct batched_entropy {
	union {
		u64 entropy_u64[CHACHA20_BLOCK_SIZE / sizeof(u64)];
		u32 entropy_u32[CHACHA20_BLOCK_SIZE / sizeof(u32)];
	};
	unsigned int position;
};
static rwlock_t batched_entropy_reset_lock = __RW_LOCK_UNLOCKED(batched_entropy_reset_lock);

/*
 * Get a random word for internal kernel use only. The quality of the random
 * number is either as good as RDRAND or as good as /dev/urandom, with the
 * goal of being quite fast and not depleting entropy. In order to ensure
 * that the randomness provided by this function is okay, the function
 * wait_for_random_bytes() should be called and return 0 at least once
 * at any point prior.
 */
static DEFINE_PER_CPU(struct batched_entropy, batched_entropy_u64);
u64 get_random_u64(void)
{
	u64 ret;
	bool use_lock;
	unsigned long flags = 0;
	struct batched_entropy *batch;
	static void *previous;

#if BITS_PER_LONG == 64
	if (arch_get_random_long((unsigned long *)&ret))
		return ret;
#else
	if (arch_get_random_long((unsigned long *)&ret) &&
	    arch_get_random_long((unsigned long *)&ret + 1))
	    return ret;
#endif

	warn_unseeded_randomness(&previous);

	use_lock = READ_ONCE(crng_init) < 2;
	batch = &get_cpu_var(batched_entropy_u64);
	if (use_lock)
		read_lock_irqsave(&batched_entropy_reset_lock, flags);
	if (batch->position % ARRAY_SIZE(batch->entropy_u64) == 0) {
		extract_crng((__u32 *)batch->entropy_u64);
		batch->position = 0;
	}
	ret = batch->entropy_u64[batch->position++];
	if (use_lock)
		read_unlock_irqrestore(&batched_entropy_reset_lock, flags);
	put_cpu_var(batched_entropy_u64);
	return ret;
}
EXPORT_SYMBOL(get_random_u64);

static DEFINE_PER_CPU(struct batched_entropy, batched_entropy_u32);
u32 get_random_u32(void)
{
	u32 ret;
	bool use_lock;
	unsigned long flags = 0;
	struct batched_entropy *batch;
	static void *previous;

	if (arch_get_random_int(&ret))
		return ret;

	warn_unseeded_randomness(&previous);

	use_lock = READ_ONCE(crng_init) < 2;
	batch = &get_cpu_var(batched_entropy_u32);
	if (use_lock)
		read_lock_irqsave(&batched_entropy_reset_lock, flags);
	if (batch->position % ARRAY_SIZE(batch->entropy_u32) == 0) {
		extract_crng(batch->entropy_u32);
		batch->position = 0;
	}
	ret = batch->entropy_u32[batch->position++];
	if (use_lock)
		read_unlock_irqrestore(&batched_entropy_reset_lock, flags);
	put_cpu_var(batched_entropy_u32);
	return ret;
}
EXPORT_SYMBOL(get_random_u32);

/* It's important to invalidate all potential batched entropy that might
 * be stored before the crng is initialized, which we can do lazily by
 * simply resetting the counter to zero so that it's re-extracted on the
 * next usage. */
static void invalidate_batched_entropy(void)
{
	int cpu;
	unsigned long flags;

	write_lock_irqsave(&batched_entropy_reset_lock, flags);
	for_each_possible_cpu (cpu) {
		per_cpu_ptr(&batched_entropy_u32, cpu)->position = 0;
		per_cpu_ptr(&batched_entropy_u64, cpu)->position = 0;
	}
	write_unlock_irqrestore(&batched_entropy_reset_lock, flags);
}

/**
 * randomize_page - Generate a random, page aligned address
 * @start:	The smallest acceptable address the caller will take.
 * @range:	The size of the area, starting at @start, within which the
 *		random address must fall.
 *
 * If @start + @range would overflow, @range is capped.
 *
 * NOTE: Historical use of randomize_range, which this replaces, presumed that
 * @start was already page aligned.  We now align it regardless.
 *
 * Return: A page aligned address within [start, start + range).  On error,
 * @start is returned.
 */
unsigned long
randomize_page(unsigned long start, unsigned long range)
{
	if (!PAGE_ALIGNED(start)) {
		range -= PAGE_ALIGN(start) - start;
		start = PAGE_ALIGN(start);
	}

	if (start > ULONG_MAX - range)
		range = ULONG_MAX - start;

	range >>= PAGE_SHIFT;

	if (range == 0)
		return start;

	return start + (get_random_long() % range << PAGE_SHIFT);
}

/* Interface for in-kernel drivers of true hardware RNGs.
 * Those devices may produce endless random bits and will be throttled
 * when our pool is full.
 */
void add_hwgenerator_randomness(const char *buffer, size_t count,
				size_t entropy)
{
	struct entropy_store *poolp = &input_pool;

	if (unlikely(crng_init == 0)) {
		crng_fast_load(buffer, count);
		return;
	}

	/* Suspend writing if we're above the trickle threshold.
	 * We'll be woken up again once below random_write_wakeup_thresh,
	 * or when the calling thread is about to terminate.
	 */
	wait_event_interruptible(random_write_wait, kthread_should_stop() ||
			ENTROPY_BITS(&input_pool) <= random_write_wakeup_bits);
	mix_pool_bytes(poolp, buffer, count);
	credit_entropy_bits(poolp, entropy);
}
EXPORT_SYMBOL_GPL(add_hwgenerator_randomness);
{
	unsigned long time = random_get_entropy() ^ jiffies;
	unsigned long flags;

	if (!crng_ready()) {
		crng_fast_load(buf, size);
		return;
	}