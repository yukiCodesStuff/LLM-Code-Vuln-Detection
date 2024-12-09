 * its value (from 0->1->2).
 */
static int crng_init = 0;
#define crng_ready() (likely(crng_init > 0))
static int crng_init_cnt = 0;
#define CRNG_INIT_CNT_THRESH (2*CHACHA20_KEY_SIZE)
static void _extract_crng(struct crng_state *crng,
			  __u32 out[CHACHA20_BLOCK_WORDS]);

	if (!spin_trylock_irqsave(&primary_crng.lock, flags))
		return 0;
	if (crng_ready()) {
		spin_unlock_irqrestore(&primary_crng.lock, flags);
		return 0;
	}
	p = (unsigned char *) &primary_crng.state[4];
{
	unsigned long v, flags;

	if (crng_init > 1 &&
	    time_after(jiffies, crng->init_time + CRNG_RESEED_INTERVAL))
		crng_reseed(crng, crng == &primary_crng ? &input_pool : NULL);
	spin_lock_irqsave(&crng->lock, flags);
	if (arch_get_random_long(&v))
	fast_mix(fast_pool);
	add_interrupt_bench(cycles);

	if (!crng_ready()) {
		if ((fast_pool->count >= 64) &&
		    crng_fast_load((char *) fast_pool->pool,
				   sizeof(fast_pool->pool))) {
			fast_pool->count = 0;
{
	struct entropy_store *poolp = &input_pool;

	if (!crng_ready()) {
		crng_fast_load(buffer, count);
		return;
	}
