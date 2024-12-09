	return data & (1 << bit) ? 1 : 0;
}

static int ichx_gpio_check_available(struct gpio_chip *gpio, unsigned nr)
{
	return (ichx_priv.use_gpio & (1 << (nr / 32))) ? 0 : -ENXIO;
}

static int ichx_gpio_direction_input(struct gpio_chip *gpio, unsigned nr)
{