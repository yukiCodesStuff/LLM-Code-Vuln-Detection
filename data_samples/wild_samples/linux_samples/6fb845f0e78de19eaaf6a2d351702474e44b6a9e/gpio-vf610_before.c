{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct vf610_gpio_port *port;
	struct resource *iores;
	struct gpio_chip *gc;
	int ret;

	port = devm_kzalloc(&pdev->dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	port->sdata = of_device_get_match_data(dev);
	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	port->base = devm_ioremap_resource(dev, iores);
	if (IS_ERR(port->base))
		return PTR_ERR(port->base);

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	port->gpio_base = devm_ioremap_resource(dev, iores);
	if (IS_ERR(port->gpio_base))
		return PTR_ERR(port->gpio_base);

	port->irq = platform_get_irq(pdev, 0);
	if (port->irq < 0)
		return port->irq;

	port->clk_port = devm_clk_get(&pdev->dev, "port");
	if (!IS_ERR(port->clk_port)) {
		ret = clk_prepare_enable(port->clk_port);
		if (ret)
			return ret;
	} else if (port->clk_port == ERR_PTR(-EPROBE_DEFER)) {
		/*
		 * Percolate deferrals, for anything else,
		 * just live without the clocking.
		 */
		return PTR_ERR(port->clk_port);
	}

	port->clk_gpio = devm_clk_get(&pdev->dev, "gpio");
	if (!IS_ERR(port->clk_gpio)) {
		ret = clk_prepare_enable(port->clk_gpio);
		if (ret) {
			clk_disable_unprepare(port->clk_port);
			return ret;
		}
	} else if (port->clk_gpio == ERR_PTR(-EPROBE_DEFER)) {
		clk_disable_unprepare(port->clk_port);
		return PTR_ERR(port->clk_gpio);
	}

	platform_set_drvdata(pdev, port);

	gc = &port->gc;
	gc->of_node = np;
	gc->parent = dev;
	gc->label = "vf610-gpio";
	gc->ngpio = VF610_GPIO_PER_PORT;
	gc->base = of_alias_get_id(np, "gpio") * VF610_GPIO_PER_PORT;

	gc->request = gpiochip_generic_request;
	gc->free = gpiochip_generic_free;
	gc->direction_input = vf610_gpio_direction_input;
	gc->get = vf610_gpio_get;
	gc->direction_output = vf610_gpio_direction_output;
	gc->set = vf610_gpio_set;

	ret = gpiochip_add_data(gc, port);
	if (ret < 0)
		return ret;

	/* Clear the interrupt status register for all GPIO's */
	vf610_gpio_writel(~0, port->base + PORT_ISFR);

	ret = gpiochip_irqchip_add(gc, &vf610_gpio_irq_chip, 0,
				   handle_edge_irq, IRQ_TYPE_NONE);
	if (ret) {
		dev_err(dev, "failed to add irqchip\n");
		gpiochip_remove(gc);
		return ret;
	}
	gpiochip_set_chained_irqchip(gc, &vf610_gpio_irq_chip, port->irq,
				     vf610_gpio_irq_handler);

	return 0;
}

static int vf610_gpio_remove(struct platform_device *pdev)
{
	struct vf610_gpio_port *port = platform_get_drvdata(pdev);

	gpiochip_remove(&port->gc);
	if (!IS_ERR(port->clk_port))
		clk_disable_unprepare(port->clk_port);
	if (!IS_ERR(port->clk_gpio))
		clk_disable_unprepare(port->clk_gpio);

	return 0;
}

static struct platform_driver vf610_gpio_driver = {
	.driver		= {
		.name	= "gpio-vf610",
		.of_match_table = vf610_gpio_dt_ids,
	},
	.probe		= vf610_gpio_probe,
	.remove		= vf610_gpio_remove,
};

builtin_platform_driver(vf610_gpio_driver);
	} else if (port->clk_gpio == ERR_PTR(-EPROBE_DEFER)) {
		clk_disable_unprepare(port->clk_port);
		return PTR_ERR(port->clk_gpio);
	}

	platform_set_drvdata(pdev, port);

	gc = &port->gc;
	gc->of_node = np;
	gc->parent = dev;
	gc->label = "vf610-gpio";
	gc->ngpio = VF610_GPIO_PER_PORT;
	gc->base = of_alias_get_id(np, "gpio") * VF610_GPIO_PER_PORT;

	gc->request = gpiochip_generic_request;
	gc->free = gpiochip_generic_free;
	gc->direction_input = vf610_gpio_direction_input;
	gc->get = vf610_gpio_get;
	gc->direction_output = vf610_gpio_direction_output;
	gc->set = vf610_gpio_set;

	ret = gpiochip_add_data(gc, port);
	if (ret < 0)
		return ret;

	/* Clear the interrupt status register for all GPIO's */
	vf610_gpio_writel(~0, port->base + PORT_ISFR);

	ret = gpiochip_irqchip_add(gc, &vf610_gpio_irq_chip, 0,
				   handle_edge_irq, IRQ_TYPE_NONE);
	if (ret) {
		dev_err(dev, "failed to add irqchip\n");
		gpiochip_remove(gc);
		return ret;
	}