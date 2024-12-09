	if (!adapter->hw.hw_addr) {
		err = -EIO;
		goto err_ioremap;
	}

	atl2_setup_pcicmd(pdev);

	netdev->netdev_ops = &atl2_netdev_ops;
	netdev->ethtool_ops = &atl2_ethtool_ops;
	netdev->watchdog_timeo = 5 * HZ;
	strncpy(netdev->name, pci_name(pdev), sizeof(netdev->name) - 1);

	netdev->mem_start = mmio_start;
	netdev->mem_end = mmio_start + mmio_len;
	adapter->bd_number = cards_found;
	adapter->pci_using_64 = false;

	/* setup the private structure */
	err = atl2_sw_init(adapter);
	if (err)
		goto err_sw_init;

	err = -EIO;

	netdev->hw_features = NETIF_F_SG | NETIF_F_HW_VLAN_CTAG_RX;
	netdev->features |= (NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX);

	/* Init PHY as early as possible due to power saving issue  */
	atl2_phy_init(&adapter->hw);

	/* reset the controller to
	 * put the device in a known good starting state */

	if (atl2_reset_hw(&adapter->hw)) {
		err = -EIO;
		goto err_reset;
	}