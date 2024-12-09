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

static u32
hns_get_rss_key_size(struct net_device *netdev)