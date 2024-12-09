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
{
	struct bcm_sysport_priv *priv = netdev_priv(dev);
	struct device *kdev = &priv->pdev->dev;
	u32 supported = WAKE_MAGIC | WAKE_MAGICSECURE | WAKE_FILTER;

	if (!device_can_wakeup(kdev))
		return -ENOTSUPP;

	if (wol->wolopts & ~supported)
		return -EINVAL;

	/* Program the SecureOn password */
	if (wol->wolopts & WAKE_MAGICSECURE) {
		umac_writel(priv, get_unaligned_be16(&wol->sopass[0]),
			    UMAC_PSW_MS);
		umac_writel(priv, get_unaligned_be32(&wol->sopass[2]),
			    UMAC_PSW_LS);
	}
{
	struct net_device *ndev = priv->netdev;
	unsigned int timeout = 1000;
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
		reg = rbuf_readl(priv, RBUF_CONTROL);
		if (priv->is_lite)
			reg |= RBUF_ACPI_EN_LITE;
		else
			reg |= RBUF_ACPI_EN;
		rbuf_writel(priv, reg, RBUF_CONTROL);

		/* Enable RXCHK, active filters and Broadcom tag matching */
		reg = rxchk_readl(priv, RXCHK_CONTROL);
		reg &= ~(RXCHK_BRCM_TAG_MATCH_MASK <<
			 RXCHK_BRCM_TAG_MATCH_SHIFT);
		for_each_set_bit(index, priv->filters, RXCHK_BRCM_TAG_MAX) {
			reg |= BIT(RXCHK_BRCM_TAG_MATCH_SHIFT + i);
			i++;
		}
		reg |= RXCHK_EN | RXCHK_BRCM_TAG_EN;
		rxchk_writel(priv, reg, RXCHK_CONTROL);
	}