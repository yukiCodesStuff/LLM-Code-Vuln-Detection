
static struct w1_gpio_platform_data vulcan_w1_gpio_pdata = {
	.pin			= 14,
	.ext_pullup_enable_pin	= -EINVAL,
};

static struct platform_device vulcan_w1_gpio = {
	.name			= "w1-gpio",