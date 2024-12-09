static struct w1_gpio_platform_data w1_gpio_pdata = {
	.pin		= AT91_PIN_PA29,
	.is_open_drain	= 1,
};

static struct platform_device w1_device = {
	.name			= "w1-gpio",