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

static u32
hns_get_rss_key_size(struct net_device *netdev)