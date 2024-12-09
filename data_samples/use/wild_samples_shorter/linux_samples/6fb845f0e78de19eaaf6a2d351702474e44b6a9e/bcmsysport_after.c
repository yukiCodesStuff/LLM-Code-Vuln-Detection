				struct ethtool_wolinfo *wol)
{
	struct bcm_sysport_priv *priv = netdev_priv(dev);

	wol->supported = WAKE_MAGIC | WAKE_MAGICSECURE | WAKE_FILTER;
	wol->wolopts = priv->wolopts;

	if (!(priv->wolopts & WAKE_MAGICSECURE))
		return;

	memcpy(wol->sopass, priv->sopass, sizeof(priv->sopass));
}

static int bcm_sysport_set_wol(struct net_device *dev,
			       struct ethtool_wolinfo *wol)
	if (wol->wolopts & ~supported)
		return -EINVAL;

	if (wol->wolopts & WAKE_MAGICSECURE)
		memcpy(priv->sopass, wol->sopass, sizeof(priv->sopass));

	/* Flag the device and relevant IRQ as wakeup capable */
	if (wol->wolopts) {
		device_set_wakeup_enable(kdev, 1);
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
	umac_writel(priv, reg, UMAC_MPD_CTRL);

	if (priv->wolopts & WAKE_FILTER) {
		/* Turn on ACPI matching to steal packets from RBUF */