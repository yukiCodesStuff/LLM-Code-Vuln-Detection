	.pin			= GPIO_ONE_WIRE,
	.is_open_drain		= 0,
	.enable_external_pullup	= w1_enable_external_pullup,
	.ext_pullup_enable_pin	= -EINVAL,
};

struct platform_device raumfeld_w1_gpio_device = {
	.name	= "w1-gpio",