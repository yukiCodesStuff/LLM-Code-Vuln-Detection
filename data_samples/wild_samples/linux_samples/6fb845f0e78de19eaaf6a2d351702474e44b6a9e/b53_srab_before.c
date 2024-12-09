{
	struct b53_device *dev = platform_get_drvdata(pdev);
	struct b53_srab_priv *priv = dev->priv;
	struct b53_srab_port_priv *port;
	unsigned int i;
	char *name;

	/* Clear all pending interrupts */
	writel(0xffffffff, priv->regs + B53_SRAB_INTR);

	if (dev->pdata && dev->pdata->chip_id != BCM58XX_DEVICE_ID)
		return;

	for (i = 0; i < B53_N_PORTS; i++) {
		port = &priv->port_intrs[i];

		/* There is no port 6 */
		if (i == 6)
			continue;

		name = kasprintf(GFP_KERNEL, "link_state_p%d", i);
		if (!name)
			return;

		port->num = i;
		port->dev = dev;
		port->irq = platform_get_irq_byname(pdev, name);
		kfree(name);
	}