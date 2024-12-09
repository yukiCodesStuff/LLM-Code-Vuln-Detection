	/* Clear all pending interrupts */
	writel(0xffffffff, priv->regs + B53_SRAB_INTR);

	if (dev->pdata && dev->pdata->chip_id != BCM58XX_DEVICE_ID)
		return;

	for (i = 0; i < B53_N_PORTS; i++) {
		port = &priv->port_intrs[i];

		/* There is no port 6 */