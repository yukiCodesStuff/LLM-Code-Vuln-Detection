static int altr_a10sr_gpio_direction_output(struct gpio_chip *gc,
					    unsigned int nr, int value)
{
	if (nr <= (ALTR_A10SR_OUT_VALID_RANGE_HI - ALTR_A10SR_LED_VALID_SHIFT))
		return 0;
	return -EINVAL;
}

static const struct gpio_chip altr_a10sr_gc = {