	if (!ops->get_regs_len) {
		netdev_err(net_dev, "ops->get_regs_len is null!\n");
		return -EOPNOTSUPP;
	}

	reg_num = ops->get_regs_len(priv->ae_handle);
	if (reg_num > 0)
		return reg_num * sizeof(u32);
	else
		return reg_num;	/* error code */
}

/**
 * hns_nic_nway_reset - nway reset
 * @dev: net device
 *
 * Return 0 on success, negative on failure
 */
static int hns_nic_nway_reset(struct net_device *netdev)
{
	int ret = 0;
	struct phy_device *phy = netdev->phydev;

	if (netif_running(netdev)) {
		/* if autoneg is disabled, don't restart auto-negotiation */
		if (phy && phy->autoneg == AUTONEG_ENABLE)
			ret = genphy_restart_aneg(phy);
	}

	return ret;
}