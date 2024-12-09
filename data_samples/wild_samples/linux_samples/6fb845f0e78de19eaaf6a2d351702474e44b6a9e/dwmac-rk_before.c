	switch (bsp_priv->phy_iface) {
	case PHY_INTERFACE_MODE_RGMII:
		dev_info(dev, "init for RGMII\n");
		bsp_priv->ops->set_to_rgmii(bsp_priv, bsp_priv->tx_delay,
					    bsp_priv->rx_delay);
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		dev_info(dev, "init for RGMII_ID\n");
		bsp_priv->ops->set_to_rgmii(bsp_priv, 0, 0);
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		dev_info(dev, "init for RGMII_RXID\n");
		bsp_priv->ops->set_to_rgmii(bsp_priv, bsp_priv->tx_delay, 0);
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		dev_info(dev, "init for RGMII_TXID\n");
		bsp_priv->ops->set_to_rgmii(bsp_priv, 0, bsp_priv->rx_delay);
		break;
	case PHY_INTERFACE_MODE_RMII:
		dev_info(dev, "init for RMII\n");
		bsp_priv->ops->set_to_rmii(bsp_priv);
		break;
	default:
		dev_err(dev, "NO interface defined!\n");
	}

	ret = phy_power_on(bsp_priv, true);
	if (ret)
		return ret;

	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);

	if (bsp_priv->integrated_phy)
		rk_gmac_integrated_phy_powerup(bsp_priv);

	return 0;
}

static void rk_gmac_powerdown(struct rk_priv_data *gmac)
{
	struct device *dev = &gmac->pdev->dev;

	if (gmac->integrated_phy)
		rk_gmac_integrated_phy_powerdown(gmac);

	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	phy_power_on(gmac, false);
	gmac_clk_enable(gmac, false);
}

static void rk_fix_speed(void *priv, unsigned int speed)
{
	struct rk_priv_data *bsp_priv = priv;
	struct device *dev = &bsp_priv->pdev->dev;

	switch (bsp_priv->phy_iface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		bsp_priv->ops->set_rgmii_speed(bsp_priv, speed);
		break;
	case PHY_INTERFACE_MODE_RMII:
		bsp_priv->ops->set_rmii_speed(bsp_priv, speed);
		break;
	default:
		dev_err(dev, "unsupported interface %d", bsp_priv->phy_iface);
	}