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
	if (crng == &primary_crng && crng_init < 2) {
		invalidate_batched_entropy();
		crng_init = 2;
		process_random_ready_list();
		wake_up_interruptible(&crng_init_wait);
		pr_notice("random: crng init done\n");
	}
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