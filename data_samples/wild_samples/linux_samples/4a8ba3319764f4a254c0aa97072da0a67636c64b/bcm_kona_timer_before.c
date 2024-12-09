{
	uint32_t reg;

	/*
	 * clear and disable interrupts
	 * We are using compare/match register 0 for our system interrupts
	 */
	reg = readl(base + KONA_GPTIMER_STCS_OFFSET);

	/* Clear compare (0) interrupt */
	reg |= 1 << KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT;
	/* disable compare */
	reg &= ~(1 << KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT);

	writel(reg, base + KONA_GPTIMER_STCS_OFFSET);

}

static void
kona_timer_get_counter(void *timer_base, uint32_t *msw, uint32_t *lsw)
{
	void __iomem *base = IOMEM(timer_base);
	int loop_limit = 4;

	/*
	 * Read 64-bit free running counter
	 * 1. Read hi-word
	 * 2. Read low-word
	 * 3. Read hi-word again
	 * 4.1
	 *      if new hi-word is not equal to previously read hi-word, then
	 *      start from #1
	 * 4.2
	 *      if new hi-word is equal to previously read hi-word then stop.
	 */

	while (--loop_limit) {
		*msw = readl(base + KONA_GPTIMER_STCHI_OFFSET);
		*lsw = readl(base + KONA_GPTIMER_STCLO_OFFSET);
		if (*msw == readl(base + KONA_GPTIMER_STCHI_OFFSET))
			break;
	}
	if (!loop_limit) {
		pr_err("bcm_kona_timer: getting counter failed.\n");
		pr_err(" Timer will be impacted\n");
	}

	return;
}
{
	void __iomem *base = IOMEM(timer_base);
	int loop_limit = 4;

	/*
	 * Read 64-bit free running counter
	 * 1. Read hi-word
	 * 2. Read low-word
	 * 3. Read hi-word again
	 * 4.1
	 *      if new hi-word is not equal to previously read hi-word, then
	 *      start from #1
	 * 4.2
	 *      if new hi-word is equal to previously read hi-word then stop.
	 */

	while (--loop_limit) {
		*msw = readl(base + KONA_GPTIMER_STCHI_OFFSET);
		*lsw = readl(base + KONA_GPTIMER_STCLO_OFFSET);
		if (*msw == readl(base + KONA_GPTIMER_STCHI_OFFSET))
			break;
	}