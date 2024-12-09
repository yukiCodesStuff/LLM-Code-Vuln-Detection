	spin_lock_irqsave(&aup->lock, flags);

	if (force_reset || (!aup->mac_enabled)) {
		writel(MAC_EN_CLOCK_ENABLE, aup->enable);
		au_sync_delay(2);
		writel((MAC_EN_RESET0 | MAC_EN_RESET1 | MAC_EN_RESET2
				| MAC_EN_CLOCK_ENABLE), aup->enable);
		au_sync_delay(2);

		aup->mac_enabled = 1;
	}

	au1000_hard_stop(dev);

	writel(MAC_EN_CLOCK_ENABLE, aup->enable);
	au_sync_delay(2);
	writel(0, aup->enable);
	au_sync_delay(2);

	aup->tx_full = 0;
	for (i = 0; i < NUM_RX_DMA; i++) {
	/* set a random MAC now in case platform_data doesn't provide one */
	random_ether_addr(dev->dev_addr);

	writel(0, aup->enable);
	aup->mac_enabled = 0;

	pd = pdev->dev.platform_data;
	if (!pd) {