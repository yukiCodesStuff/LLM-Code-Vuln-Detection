	/* If you choose to use a pin other than PB16 it needs to be 3.3V */
	.pin		= AT91_PIN_PB16,
	.is_open_drain  = 1,
};

static struct platform_device w1_device = {
	.name			= "w1-gpio",