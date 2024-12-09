
	err = -EIO;

	netdev->hw_features = NETIF_F_SG | NETIF_F_HW_VLAN_CTAG_RX;
	netdev->features |= (NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX);

	/* Init PHY as early as possible due to power saving issue  */
	atl2_phy_init(&adapter->hw);