        return IRQ_HANDLED;
};

static int gpio_halt_probe(struct platform_device *pdev)
{
	enum of_gpio_flags flags;
	struct device_node *node = pdev->dev.of_node;
	int gpio, err, irq;
	return 0;
}

static int gpio_halt_remove(struct platform_device *pdev)
{
	if (halt_node) {
		int gpio = of_get_gpio(halt_node, 0);
		int irq = irq_of_parse_and_map(halt_node, 0);
		.of_match_table = gpio_halt_match,
	},
	.probe		= gpio_halt_probe,
	.remove		= gpio_halt_remove,
};

module_platform_driver(gpio_halt_driver);
