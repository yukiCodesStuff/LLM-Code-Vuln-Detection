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
	int error;

	error = regulator_enable(ts->reg);
	if (error != 0)
		dev_err(&ts->spi->dev, "Failed to enable supply: %d\n", error);

	ads7846_restart(ts);
}