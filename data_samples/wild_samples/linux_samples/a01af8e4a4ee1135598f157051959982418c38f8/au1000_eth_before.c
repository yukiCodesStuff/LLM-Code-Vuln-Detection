{
	unsigned long flags;
	struct au1000_private *aup = netdev_priv(dev);

	spin_lock_irqsave(&aup->lock, flags);

	if (force_reset || (!aup->mac_enabled)) {
		writel(MAC_EN_CLOCK_ENABLE, &aup->enable);
		au_sync_delay(2);
		writel((MAC_EN_RESET0 | MAC_EN_RESET1 | MAC_EN_RESET2
				| MAC_EN_CLOCK_ENABLE), &aup->enable);
		au_sync_delay(2);

		aup->mac_enabled = 1;
	}
{
	struct au1000_private *const aup = netdev_priv(dev);
	int i;

	au1000_hard_stop(dev);

	writel(MAC_EN_CLOCK_ENABLE, &aup->enable);
	au_sync_delay(2);
	writel(0, &aup->enable);
	au_sync_delay(2);

	aup->tx_full = 0;
	for (i = 0; i < NUM_RX_DMA; i++) {
		/* reset control bits */
		aup->rx_dma_ring[i]->buff_stat &= ~0xf;
	}
	if (!aup->enable) {
		dev_err(&pdev->dev, "failed to ioremap MAC enable register\n");
		err = -ENXIO;
		goto err_remap2;
	}
	aup->mac_id = pdev->id;

	if (pdev->id == 0)
		au1000_setup_hw_rings(aup, MAC0_RX_DMA_ADDR, MAC0_TX_DMA_ADDR);
	else if (pdev->id == 1)
		au1000_setup_hw_rings(aup, MAC1_RX_DMA_ADDR, MAC1_TX_DMA_ADDR);

	/* set a random MAC now in case platform_data doesn't provide one */
	random_ether_addr(dev->dev_addr);

	writel(0, &aup->enable);
	aup->mac_enabled = 0;

	pd = pdev->dev.platform_data;
	if (!pd) {
		dev_info(&pdev->dev, "no platform_data passed,"
					" PHY search on MAC0\n");
		aup->phy1_search_mac0 = 1;
	} else {
		if (is_valid_ether_addr(pd->mac))
			memcpy(dev->dev_addr, pd->mac, 6);

		aup->phy_static_config = pd->phy_static_config;
		aup->phy_search_highest_addr = pd->phy_search_highest_addr;
		aup->phy1_search_mac0 = pd->phy1_search_mac0;
		aup->phy_addr = pd->phy_addr;
		aup->phy_busid = pd->phy_busid;
		aup->phy_irq = pd->phy_irq;
	}

	if (aup->phy_busid && aup->phy_busid > 0) {
		dev_err(&pdev->dev, "MAC0-associated PHY attached 2nd MACs MII bus not supported yet\n");
		err = -ENODEV;
		goto err_mdiobus_alloc;
	}

	aup->mii_bus = mdiobus_alloc();
	if (aup->mii_bus == NULL) {
		dev_err(&pdev->dev, "failed to allocate mdiobus structure\n");
		err = -ENOMEM;
		goto err_mdiobus_alloc;
	}

	aup->mii_bus->priv = dev;
	aup->mii_bus->read = au1000_mdiobus_read;
	aup->mii_bus->write = au1000_mdiobus_write;
	aup->mii_bus->reset = au1000_mdiobus_reset;
	aup->mii_bus->name = "au1000_eth_mii";
	snprintf(aup->mii_bus->id, MII_BUS_ID_SIZE, "%x", aup->mac_id);
	aup->mii_bus->irq = kmalloc(sizeof(int)*PHY_MAX_ADDR, GFP_KERNEL);
	if (aup->mii_bus->irq == NULL)
		goto err_out;

	for (i = 0; i < PHY_MAX_ADDR; ++i)
		aup->mii_bus->irq[i] = PHY_POLL;
	/* if known, set corresponding PHY IRQs */
	if (aup->phy_static_config)
		if (aup->phy_irq && aup->phy_busid == aup->mac_id)
			aup->mii_bus->irq[aup->phy_addr] = aup->phy_irq;

	err = mdiobus_register(aup->mii_bus);
	if (err) {
		dev_err(&pdev->dev, "failed to register MDIO bus\n");
		goto err_mdiobus_reg;
	}

	if (au1000_mii_probe(dev) != 0)
		goto err_out;

	pDBfree = NULL;
	/* setup the data buffer descriptors and attach a buffer to each one */
	pDB = aup->db;
	for (i = 0; i < (NUM_TX_BUFFS+NUM_RX_BUFFS); i++) {
		pDB->pnext = pDBfree;
		pDBfree = pDB;
		pDB->vaddr = (u32 *)((unsigned)aup->vaddr + MAX_BUF_SIZE*i);
		pDB->dma_addr = (dma_addr_t)virt_to_bus(pDB->vaddr);
		pDB++;
	}
	aup->pDBfree = pDBfree;

	for (i = 0; i < NUM_RX_DMA; i++) {
		pDB = au1000_GetFreeDB(aup);
		if (!pDB)
			goto err_out;

		aup->rx_dma_ring[i]->buff_stat = (unsigned)pDB->dma_addr;
		aup->rx_db_inuse[i] = pDB;
	}
	for (i = 0; i < NUM_TX_DMA; i++) {
		pDB = au1000_GetFreeDB(aup);
		if (!pDB)
			goto err_out;

		aup->tx_dma_ring[i]->buff_stat = (unsigned)pDB->dma_addr;
		aup->tx_dma_ring[i]->len = 0;
		aup->tx_db_inuse[i] = pDB;
	}

	dev->base_addr = base->start;
	dev->irq = irq;
	dev->netdev_ops = &au1000_netdev_ops;
	SET_ETHTOOL_OPS(dev, &au1000_ethtool_ops);
	dev->watchdog_timeo = ETH_TX_TIMEOUT;

	/*
	 * The boot code uses the ethernet controller, so reset it to start
	 * fresh.  au1000_init() expects that the device is in reset state.
	 */
	au1000_reset_mac(dev);

	err = register_netdev(dev);
	if (err) {
		netdev_err(dev, "Cannot register net device, aborting.\n");
		goto err_out;
	}

	netdev_info(dev, "Au1xx0 Ethernet found at 0x%lx, irq %d\n",
			(unsigned long)base->start, irq);
	if (version_printed++ == 0)
		pr_info("%s version %s %s\n",
					DRV_NAME, DRV_VERSION, DRV_AUTHOR);

	return 0;

err_out:
	if (aup->mii_bus != NULL)
		mdiobus_unregister(aup->mii_bus);

	/* here we should have a valid dev plus aup-> register addresses
	 * so we can reset the mac properly.
	 */
	au1000_reset_mac(dev);

	for (i = 0; i < NUM_RX_DMA; i++) {
		if (aup->rx_db_inuse[i])
			au1000_ReleaseDB(aup, aup->rx_db_inuse[i]);
	}
	for (i = 0; i < NUM_TX_DMA; i++) {
		if (aup->tx_db_inuse[i])
			au1000_ReleaseDB(aup, aup->tx_db_inuse[i]);
	}
err_mdiobus_reg:
	mdiobus_free(aup->mii_bus);
err_mdiobus_alloc:
	iounmap(aup->enable);
err_remap2:
	iounmap(aup->mac);
err_remap1:
	dma_free_noncoherent(NULL, MAX_BUF_SIZE * (NUM_TX_BUFFS + NUM_RX_BUFFS),
			     (void *)aup->vaddr, aup->dma_addr);
err_vaddr:
	free_netdev(dev);
err_alloc:
	release_mem_region(macen->start, resource_size(macen));
err_request:
	release_mem_region(base->start, resource_size(base));
out:
	return err;
}