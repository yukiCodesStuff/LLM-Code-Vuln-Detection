	for (i = 4; i < 16; i++) {
		if (!arch_get_random_seed_long(&rv) &&
		    !arch_get_random_long(&rv))
			rv = random_get_entropy();
		crng->state[i] ^= rv;
	}
	crng->init_time = jiffies - CRNG_RESEED_INTERVAL - 1;
}

#ifdef CONFIG_NUMA
static void numa_crng_init(void)
{
	int i;
	struct crng_state *crng;
	struct crng_state **pool;

	pool = kcalloc(nr_node_ids, sizeof(*pool), GFP_KERNEL|__GFP_NOFAIL);
	for_each_online_node(i) {
		crng = kmalloc_node(sizeof(struct crng_state),
				    GFP_KERNEL | __GFP_NOFAIL, i);
		spin_lock_init(&crng->lock);
		crng_initialize(crng);
		pool[i] = crng;
	}
	if (crng == &primary_crng && crng_init < 2) {
		invalidate_batched_entropy();
		numa_crng_init();
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
	init_std_data(&input_pool);
	init_std_data(&blocking_pool);
	crng_initialize(&primary_crng);
	return 0;
}