	struct vf610_gpio_port *port;
	struct resource *iores;
	struct gpio_chip *gc;
	int i;
	int ret;

	port = devm_kzalloc(&pdev->dev, sizeof(*port), GFP_KERNEL);
	if (!port)
	if (ret < 0)
		return ret;

	/* Mask all GPIO interrupts */
	for (i = 0; i < gc->ngpio; i++)
		vf610_gpio_writel(0, port->base + PORT_PCR(i));

	/* Clear the interrupt status register for all GPIO's */
	vf610_gpio_writel(~0, port->base + PORT_ISFR);

	ret = gpiochip_irqchip_add(gc, &vf610_gpio_irq_chip, 0,