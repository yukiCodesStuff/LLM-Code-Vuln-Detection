				struct ethtool_wolinfo *wol)
{
	struct bcm_sysport_priv *priv = netdev_priv(dev);
	u32 reg;

	wol->supported = WAKE_MAGIC | WAKE_MAGICSECURE | WAKE_FILTER;
	wol->wolopts = priv->wolopts;

	if (!(priv->wolopts & WAKE_MAGICSECURE))
		return;

	/* Return the programmed SecureOn password */
	reg = umac_readl(priv, UMAC_PSW_MS);
	put_unaligned_be16(reg, &wol->sopass[0]);
	reg = umac_readl(priv, UMAC_PSW_LS);
	put_unaligned_be32(reg, &wol->sopass[2]);
}

static int bcm_sysport_set_wol(struct net_device *dev,
			       struct ethtool_wolinfo *wol)
	if (wol->wolopts & ~supported)
		return -EINVAL;

	/* Program the SecureOn password */
	if (wol->wolopts & WAKE_MAGICSECURE) {
		umac_writel(priv, get_unaligned_be16(&wol->sopass[0]),
			    UMAC_PSW_MS);
		umac_writel(priv, get_unaligned_be32(&wol->sopass[2]),
			    UMAC_PSW_LS);
	}

	/* Flag the device and relevant IRQ as wakeup capable */
	if (wol->wolopts) {
		device_set_wakeup_enable(kdev, 1);
	unsigned int index, i = 0;
	u32 reg;

	/* Password has already been programmed */
	reg = umac_readl(priv, UMAC_MPD_CTRL);
	if (priv->wolopts & (WAKE_MAGIC | WAKE_MAGICSECURE))
		reg |= MPD_EN;
	reg &= ~PSW_EN;
	if (priv->wolopts & WAKE_MAGICSECURE)
		reg |= PSW_EN;
	umac_writel(priv, reg, UMAC_MPD_CTRL);

	if (priv->wolopts & WAKE_FILTER) {
		/* Turn on ACPI matching to steal packets from RBUF */