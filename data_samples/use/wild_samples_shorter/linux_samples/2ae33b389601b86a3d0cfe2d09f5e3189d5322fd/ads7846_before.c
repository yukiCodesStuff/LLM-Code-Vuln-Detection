/* Must be called with ts->lock held */
static void __ads7846_enable(struct ads7846 *ts)
{
	regulator_enable(ts->reg);
	ads7846_restart(ts);
}

static void ads7846_disable(struct ads7846 *ts)