		goto fail2;
	}

	efx->rxq_entries = efx->txq_entries = EFX_DEFAULT_DMAQ_SIZE;

	rc = efx_probe_filters(efx);
	if (rc) {
	net_dev->irq = efx->pci_dev->irq;
	net_dev->netdev_ops = &efx_netdev_ops;
	SET_ETHTOOL_OPS(net_dev, &efx_ethtool_ops);

	rtnl_lock();

	rc = dev_alloc_name(net_dev, net_dev->name);