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
	mb();
	if (cmpxchg(&crng_node_pool, NULL, pool)) {
		for_each_node(i)
			kfree(pool[i]);
		kfree(pool);
	}
}
#else
static void numa_crng_init(void) {}
#endif

/*
 * crng_fast_load() can be called by code in the interrupt service
 * path.  So we can't afford to dilly-dally.
 */
	spin_unlock_irqrestore(&primary_crng.lock, flags);
	if (crng == &primary_crng && crng_init < 2) {
		invalidate_batched_entropy();
		numa_crng_init();
		crng_init = 2;
		process_random_ready_list();
		wake_up_interruptible(&crng_init_wait);
		pr_notice("random: crng init done\n");
 */
static int rand_initialize(void)
{
	init_std_data(&input_pool);
	init_std_data(&blocking_pool);
	crng_initialize(&primary_crng);
	return 0;
}
early_initcall(rand_initialize);
