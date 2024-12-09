{
	struct bcm_sysport_priv *priv = netdev_priv(dev);

	wol->supported = WAKE_MAGIC | WAKE_MAGICSECURE | WAKE_FILTER;
	wol->wolopts = priv->wolopts;

	if (!(priv->wolopts & WAKE_MAGICSECURE))
		return;

	memcpy(wol->sopass, priv->sopass, sizeof(priv->sopass));
}
{
	struct bcm_sysport_priv *priv = netdev_priv(dev);
	struct device *kdev = &priv->pdev->dev;
	u32 supported = WAKE_MAGIC | WAKE_MAGICSECURE | WAKE_FILTER;

	if (!device_can_wakeup(kdev))
		return -ENOTSUPP;

	if (wol->wolopts & ~supported)
		return -EINVAL;

	if (wol->wolopts & WAKE_MAGICSECURE)
		memcpy(priv->sopass, wol->sopass, sizeof(priv->sopass));

	/* Flag the device and relevant IRQ as wakeup capable */
	if (wol->wolopts) {
		device_set_wakeup_enable(kdev, 1);
		if (priv->wol_irq_disabled)
			enable_irq_wake(priv->wol_irq);
		priv->wol_irq_disabled = 0;
	} else {
		device_set_wakeup_enable(kdev, 0);
		/* Avoid unbalanced disable_irq_wake calls */
		if (!priv->wol_irq_disabled)
			disable_irq_wake(priv->wol_irq);
		priv->wol_irq_disabled = 1;
	}

	priv->wolopts = wol->wolopts;

	return 0;
}
{
	struct net_device *ndev = priv->netdev;
	unsigned int timeout = 1000;
	unsigned int index, i = 0;
	u32 reg;

	reg = umac_readl(priv, UMAC_MPD_CTRL);
	if (priv->wolopts & (WAKE_MAGIC | WAKE_MAGICSECURE))
		reg |= MPD_EN;
	reg &= ~PSW_EN;
	if (priv->wolopts & WAKE_MAGICSECURE) {
		/* Program the SecureOn password */
		umac_writel(priv, get_unaligned_be16(&priv->sopass[0]),
			    UMAC_PSW_MS);
		umac_writel(priv, get_unaligned_be32(&priv->sopass[2]),
			    UMAC_PSW_LS);
		reg |= PSW_EN;
	}