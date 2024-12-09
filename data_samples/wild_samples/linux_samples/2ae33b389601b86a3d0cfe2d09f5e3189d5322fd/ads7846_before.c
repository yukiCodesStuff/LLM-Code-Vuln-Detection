{
	ads7846_stop(ts);
	regulator_disable(ts->reg);

	/*
	 * We know the chip's in low power mode since we always
	 * leave it that way after every request
	 */
}

/* Must be called with ts->lock held */
static void __ads7846_enable(struct ads7846 *ts)
{
	regulator_enable(ts->reg);
	ads7846_restart(ts);
}