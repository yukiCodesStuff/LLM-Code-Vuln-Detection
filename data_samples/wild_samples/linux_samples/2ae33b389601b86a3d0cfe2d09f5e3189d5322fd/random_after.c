{
	unsigned long flags;
	int wakeup_write = 0;

	/* Hold lock while accounting */
	spin_lock_irqsave(&r->lock, flags);

	BUG_ON(r->entropy_count > r->poolinfo->POOLBITS);
	DEBUG_ENT("trying to extract %zu bits from %s\n",
		  nbytes * 8, r->name);

	/* Can we pull enough? */
	if (r->entropy_count / 8 < min + reserved) {
		nbytes = 0;
	} else {
		/* If limited, never pull more than available */
		if (r->limit && nbytes + reserved >= r->entropy_count / 8)
			nbytes = r->entropy_count/8 - reserved;

		if (r->entropy_count / 8 >= nbytes + reserved)
			r->entropy_count -= nbytes*8;
		else
			r->entropy_count = reserved;

		if (r->entropy_count < random_write_wakeup_thresh)
			wakeup_write = 1;
	}

	DEBUG_ENT("debiting %zu entropy credits from %s%s\n",
		  nbytes * 8, r->name, r->limit ? "" : " (unlimited)");

	spin_unlock_irqrestore(&r->lock, flags);

	if (wakeup_write) {
		wake_up_interruptible(&random_write_wait);
		kill_fasync(&fasync, SIGIO, POLL_OUT);
	}

	return nbytes;
}

static void extract_buf(struct entropy_store *r, __u8 *out)
{
	int i;
	union {
		__u32 w[5];
		unsigned long l[LONGS(EXTRACT_SIZE)];
	} hash;
	__u32 workspace[SHA_WORKSPACE_WORDS];
	__u8 extract[64];
	unsigned long flags;

	/* Generate a hash across the pool, 16 words (512 bits) at a time */
	sha_init(hash.w);
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
	__mix_pool_bytes(r, hash.w, sizeof(hash.w), extract);
	spin_unlock_irqrestore(&r->lock, flags);

	/*
	 * To avoid duplicates, we atomically extract a portion of the
	 * pool while mixing, and hash one final time.
	 */
	sha_transform(hash.w, extract, workspace);
	memset(extract, 0, sizeof(extract));
	memset(workspace, 0, sizeof(workspace));

	/*
	 * In case the hash function has some recognizable output
	 * pattern, we fold it in half. Thus, we always feed back
	 * twice as much data as we output.
	 */
	hash.w[0] ^= hash.w[3];
	hash.w[1] ^= hash.w[4];
	hash.w[2] ^= rol32(hash.w[2], 16);

	/*
	 * If we have a architectural hardware random number
	 * generator, mix that in, too.
	 */
	for (i = 0; i < LONGS(EXTRACT_SIZE); i++) {
		unsigned long v;
		if (!arch_get_random_long(&v))
			break;
		hash.l[i] ^= v;
	}

	memcpy(out, &hash, EXTRACT_SIZE);
	memset(&hash, 0, sizeof(hash));
}

static ssize_t extract_entropy(struct entropy_store *r, void *buf,
				 size_t nbytes, int min, int reserved)
{
	ssize_t ret = 0, i;
	__u8 tmp[EXTRACT_SIZE];

	/* if last_data isn't primed, we need EXTRACT_SIZE extra bytes */
	if (fips_enabled && !r->last_data_init)
		nbytes += EXTRACT_SIZE;

	trace_extract_entropy(r->name, nbytes, r->entropy_count, _RET_IP_);
	xfer_secondary_pool(r, nbytes);
	nbytes = account(r, nbytes, min, reserved);

	while (nbytes) {
		extract_buf(r, tmp);

		if (fips_enabled) {
			unsigned long flags;


			/* prime last_data value if need be, per fips 140-2 */
			if (!r->last_data_init) {
				spin_lock_irqsave(&r->lock, flags);
				memcpy(r->last_data, tmp, EXTRACT_SIZE);
				r->last_data_init = true;
				nbytes -= EXTRACT_SIZE;
				spin_unlock_irqrestore(&r->lock, flags);
				extract_buf(r, tmp);
			}

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
	memset(tmp, 0, sizeof(tmp));

	return ret;
}

static ssize_t extract_entropy_user(struct entropy_store *r, void __user *buf,
				    size_t nbytes)
{
	ssize_t ret = 0, i;
	__u8 tmp[EXTRACT_SIZE];

	trace_extract_entropy_user(r->name, nbytes, r->entropy_count, _RET_IP_);
	xfer_secondary_pool(r, nbytes);
	nbytes = account(r, nbytes, 0, 0);

	while (nbytes) {
		if (need_resched()) {
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
	memset(tmp, 0, sizeof(tmp));

	return ret;
}

/*
 * This function is the exported kernel interface.  It returns some
 * number of good random numbers, suitable for key generation, seeding
 * TCP sequence numbers, etc.  It does not use the hw random number
 * generator, if available; use get_random_bytes_arch() for that.
 */
void get_random_bytes(void *buf, int nbytes)
{
	extract_entropy(&nonblocking_pool, buf, nbytes, 0, 0);
}
EXPORT_SYMBOL(get_random_bytes);

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

	trace_get_random_bytes(nbytes, _RET_IP_);
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
		extract_entropy(&nonblocking_pool, p, nbytes, 0, 0);
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

	r->entropy_count = 0;
	r->entropy_total = 0;
	r->last_data_init = false;
	mix_pool_bytes(r, &now, sizeof(now), NULL);
	for (i = r->poolinfo->POOLBYTES; i > 0; i -= sizeof(rv)) {
		if (!arch_get_random_long(&rv))
			break;
		mix_pool_bytes(r, &rv, sizeof(rv), NULL);
	}
	mix_pool_bytes(r, utsname(), sizeof(*(utsname())), NULL);
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
	init_std_data(&input_pool);
	init_std_data(&blocking_pool);
	init_std_data(&nonblocking_pool);
	return 0;
}
module_init(rand_initialize);

#ifdef CONFIG_BLOCK
void rand_initialize_disk(struct gendisk *disk)
{
	struct timer_rand_state *state;

	/*
	 * If kzalloc returns null, we just won't use that entropy
	 * source.
	 */
	state = kzalloc(sizeof(struct timer_rand_state), GFP_KERNEL);
	if (state)
		disk->random = state;
}
#endif

static ssize_t
random_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t n, retval = 0, count = 0;

	if (nbytes == 0)
		return 0;

	while (nbytes > 0) {
		n = nbytes;
		if (n > SEC_XFER_SIZE)
			n = SEC_XFER_SIZE;

		DEBUG_ENT("reading %zu bits\n", n*8);

		n = extract_entropy_user(&blocking_pool, buf, n);

		if (n < 0) {
			retval = n;
			break;
		}

		DEBUG_ENT("read got %zd bits (%zd still needed)\n",
			  n*8, (nbytes-n)*8);

		if (n == 0) {
			if (file->f_flags & O_NONBLOCK) {
				retval = -EAGAIN;
				break;
			}

			DEBUG_ENT("sleeping?\n");

			wait_event_interruptible(random_read_wait,
				input_pool.entropy_count >=
						 random_read_wakeup_thresh);

			DEBUG_ENT("awake\n");

			if (signal_pending(current)) {
				retval = -ERESTARTSYS;
				break;
			}

			continue;
		}

		count += n;
		buf += n;
		nbytes -= n;
		break;		/* This break makes the device work */
				/* like a named pipe */
	}

	return (count ? count : retval);
}

static ssize_t
urandom_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	return extract_entropy_user(&nonblocking_pool, buf, nbytes);
}

static unsigned int
random_poll(struct file *file, poll_table * wait)
{
	unsigned int mask;

	poll_wait(file, &random_read_wait, wait);
	poll_wait(file, &random_write_wait, wait);
	mask = 0;
	if (input_pool.entropy_count >= random_read_wakeup_thresh)
		mask |= POLLIN | POLLRDNORM;
	if (input_pool.entropy_count < random_write_wakeup_thresh)
		mask |= POLLOUT | POLLWRNORM;
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

		mix_pool_bytes(r, buf, bytes, NULL);
		cond_resched();
	}

	return 0;
}

static ssize_t random_write(struct file *file, const char __user *buffer,
			    size_t count, loff_t *ppos)
{
	size_t ret;

	ret = write_pool(&blocking_pool, buffer, count);
	if (ret)
		return ret;
	ret = write_pool(&nonblocking_pool, buffer, count);
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
		if (put_user(input_pool.entropy_count, p))
			return -EFAULT;
		return 0;
	case RNDADDTOENTCNT:
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		if (get_user(ent_count, p))
			return -EFAULT;
		credit_entropy_bits(&input_pool, ent_count);
		return 0;
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
		credit_entropy_bits(&input_pool, ent_count);
		return 0;
	case RNDZAPENTCNT:
	case RNDCLEARPOOL:
		/* Clear the entropy pool counters. */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		rand_initialize();
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

/***************************************************************
 * Random UUID interface
 *
 * Used here for a Boot ID, but can be useful for other kernel
 * drivers.
 ***************************************************************/

/*
 * Generate random UUID
 */
void generate_random_uuid(unsigned char uuid_out[16])
{
	get_random_bytes(uuid_out, 16);
	/* Set UUID version to 4 --- truly random generation */
	uuid_out[6] = (uuid_out[6] & 0x0F) | 0x40;
	/* Set the UUID variant to DCE */
	uuid_out[8] = (uuid_out[8] & 0x3F) | 0x80;
}
EXPORT_SYMBOL(generate_random_uuid);

/********************************************************************
 *
 * Sysctl interface
 *
 ********************************************************************/

#ifdef CONFIG_SYSCTL

#include <linux/sysctl.h>

static int min_read_thresh = 8, min_write_thresh;
static int max_read_thresh = INPUT_POOL_WORDS * 32;
static int max_write_thresh = INPUT_POOL_WORDS * 32;
static char sysctl_bootid[16];

/*
 * These functions is used to return both the bootid UUID, and random
 * UUID.  The difference is in whether table->data is NULL; if it is,
 * then a new UUID is generated and returned to the user.
 *
 * If the user accesses this via the proc interface, it will be returned
 * as an ASCII string in the standard UUID format.  If accesses via the
 * sysctl system call, it is returned as 16 bytes of binary data.
 */
static int proc_do_uuid(ctl_table *table, int write,
			void __user *buffer, size_t *lenp, loff_t *ppos)
{
	ctl_table fake_table;
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

static int sysctl_poolsize = INPUT_POOL_WORDS * 32;
extern ctl_table random_table[];
ctl_table random_table[] = {
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
		.proc_handler	= proc_dointvec,
		.data		= &input_pool.entropy_count,
	},
	{
		.procname	= "read_wakeup_threshold",
		.data		= &random_read_wakeup_thresh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &min_read_thresh,
		.extra2		= &max_read_thresh,
	},
	{
		.procname	= "write_wakeup_threshold",
		.data		= &random_write_wakeup_thresh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &min_write_thresh,
		.extra2		= &max_write_thresh,
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
	{ }
};
#endif 	/* CONFIG_SYSCTL */

static u32 random_int_secret[MD5_MESSAGE_BYTES / 4] ____cacheline_aligned;

static int __init random_int_secret_init(void)
{
	get_random_bytes(random_int_secret, sizeof(random_int_secret));
	return 0;
}
late_initcall(random_int_secret_init);

/*
 * Get a random word for internal kernel use only. Similar to urandom but
 * with the goal of minimal entropy pool depletion. As a result, the random
 * value is not cryptographically secure but for several uses the cost of
 * depleting entropy is too high
 */
static DEFINE_PER_CPU(__u32 [MD5_DIGEST_WORDS], get_random_int_hash);
unsigned int get_random_int(void)
{
	__u32 *hash;
	unsigned int ret;

	if (arch_get_random_int(&ret))
		return ret;

	hash = get_cpu_var(get_random_int_hash);

	hash[0] += current->pid + jiffies + get_cycles();
	md5_transform(hash, random_int_secret);
	ret = hash[0];
	put_cpu_var(get_random_int_hash);

	return ret;
}

/*
 * randomize_range() returns a start address such that
 *
 *    [...... <range> .....]
 *  start                  end
 *
 * a <range> with size "len" starting at the return value is inside in the
 * area defined by [start, end], but is otherwise randomized.
 */
unsigned long
randomize_range(unsigned long start, unsigned long end, unsigned long len)
{
	unsigned long range = end - len - start;

	if (end <= start + len)
		return 0;
	return PAGE_ALIGN(get_random_int() % range + start);
}
	} else {
		/* If limited, never pull more than available */
		if (r->limit && nbytes + reserved >= r->entropy_count / 8)
			nbytes = r->entropy_count/8 - reserved;

		if (r->entropy_count / 8 >= nbytes + reserved)
			r->entropy_count -= nbytes*8;
		else
			r->entropy_count = reserved;

		if (r->entropy_count < random_write_wakeup_thresh)
			wakeup_write = 1;
	}

	DEBUG_ENT("debiting %zu entropy credits from %s%s\n",
		  nbytes * 8, r->name, r->limit ? "" : " (unlimited)");

	spin_unlock_irqrestore(&r->lock, flags);

	if (wakeup_write) {
		wake_up_interruptible(&random_write_wait);
		kill_fasync(&fasync, SIGIO, POLL_OUT);
	}

	return nbytes;
}

static void extract_buf(struct entropy_store *r, __u8 *out)
{
	int i;
	union {
		__u32 w[5];
		unsigned long l[LONGS(EXTRACT_SIZE)];
	} hash;
	__u32 workspace[SHA_WORKSPACE_WORDS];
	__u8 extract[64];
	unsigned long flags;

	/* Generate a hash across the pool, 16 words (512 bits) at a time */
	sha_init(hash.w);
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
	__mix_pool_bytes(r, hash.w, sizeof(hash.w), extract);
	spin_unlock_irqrestore(&r->lock, flags);

	/*
	 * To avoid duplicates, we atomically extract a portion of the
	 * pool while mixing, and hash one final time.
	 */
	sha_transform(hash.w, extract, workspace);
	memset(extract, 0, sizeof(extract));
	memset(workspace, 0, sizeof(workspace));

	/*
	 * In case the hash function has some recognizable output
	 * pattern, we fold it in half. Thus, we always feed back
	 * twice as much data as we output.
	 */
	hash.w[0] ^= hash.w[3];
	hash.w[1] ^= hash.w[4];
	hash.w[2] ^= rol32(hash.w[2], 16);

	/*
	 * If we have a architectural hardware random number
	 * generator, mix that in, too.
	 */
	for (i = 0; i < LONGS(EXTRACT_SIZE); i++) {
		unsigned long v;
		if (!arch_get_random_long(&v))
			break;
		hash.l[i] ^= v;
	}