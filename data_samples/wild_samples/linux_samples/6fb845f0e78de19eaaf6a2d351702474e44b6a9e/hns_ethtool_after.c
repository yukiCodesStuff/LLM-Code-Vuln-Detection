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
	struct phy_device *phy = netdev->phydev;

	if (!netif_running(netdev))
		return 0;

	if (!phy)
		return -EOPNOTSUPP;

	if (phy->autoneg != AUTONEG_ENABLE)
		return -EINVAL;

	return genphy_restart_aneg(phy);
}