}

static void
kona_timer_get_counter(void *timer_base, uint32_t *msw, uint32_t *lsw)
{
	void __iomem *base = IOMEM(timer_base);
	int loop_limit = 4;

	/*
	 * Read 64-bit free running counter
	 */

	while (--loop_limit) {
		*msw = readl(base + KONA_GPTIMER_STCHI_OFFSET);
		*lsw = readl(base + KONA_GPTIMER_STCLO_OFFSET);
		if (*msw == readl(base + KONA_GPTIMER_STCHI_OFFSET))
			break;
	}
	if (!loop_limit) {
		pr_err("bcm_kona_timer: getting counter failed.\n");