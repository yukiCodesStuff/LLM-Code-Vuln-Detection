	} else {
		adapter->rss = false;
	}
#endif

	vmxnet3_read_mac_addr(adapter, mac);
	memcpy(netdev->dev_addr,  mac, netdev->addr_len);

	netdev->netdev_ops = &vmxnet3_netdev_ops;
	vmxnet3_set_ethtool_ops(netdev);
	netdev->watchdog_timeo = 5 * HZ;

	INIT_WORK(&adapter->work, vmxnet3_reset_work);
	set_bit(VMXNET3_STATE_BIT_QUIESCED, &adapter->state);

	if (adapter->intr.type == VMXNET3_IT_MSIX) {
		int i;
		for (i = 0; i < adapter->num_rx_queues; i++) {
			netif_napi_add(adapter->netdev,
				       &adapter->rx_queue[i].napi,
				       vmxnet3_poll_rx_only, 64);
		}
	} else {
		netif_napi_add(adapter->netdev, &adapter->rx_queue[0].napi,
			       vmxnet3_poll, 64);
	}

	netif_set_real_num_tx_queues(adapter->netdev, adapter->num_tx_queues);
	netif_set_real_num_rx_queues(adapter->netdev, adapter->num_rx_queues);

	err = register_netdev(netdev);

	if (err) {
		printk(KERN_ERR "Failed to register adapter %s\n",
			pci_name(pdev));
		goto err_register;
	}

	vmxnet3_check_link(adapter, false);
	atomic_inc(&devices_found);
	return 0;

err_register:
	vmxnet3_free_intr_resources(adapter);
err_ver:
	vmxnet3_free_pci_resources(adapter);
err_alloc_pci:
#ifdef VMXNET3_RSS
	kfree(adapter->rss_conf);
err_alloc_rss:
#endif
	kfree(adapter->pm_conf);
err_alloc_pm:
	pci_free_consistent(adapter->pdev, size, adapter->tqd_start,
			    adapter->queue_desc_pa);
err_alloc_queue_desc:
	pci_free_consistent(adapter->pdev, sizeof(struct Vmxnet3_DriverShared),
			    adapter->shared, adapter->shared_pa);
err_alloc_shared:
	pci_set_drvdata(pdev, NULL);
	free_netdev(netdev);
	return err;
}
	if (err) {
		printk(KERN_ERR "Failed to register adapter %s\n",
			pci_name(pdev));
		goto err_register;
	}

	vmxnet3_check_link(adapter, false);
	atomic_inc(&devices_found);
	return 0;

err_register:
	vmxnet3_free_intr_resources(adapter);
err_ver:
	vmxnet3_free_pci_resources(adapter);
err_alloc_pci:
#ifdef VMXNET3_RSS
	kfree(adapter->rss_conf);
err_alloc_rss:
#endif
	kfree(adapter->pm_conf);
err_alloc_pm:
	pci_free_consistent(adapter->pdev, size, adapter->tqd_start,
			    adapter->queue_desc_pa);
err_alloc_queue_desc:
	pci_free_consistent(adapter->pdev, sizeof(struct Vmxnet3_DriverShared),
			    adapter->shared, adapter->shared_pa);
err_alloc_shared:
	pci_set_drvdata(pdev, NULL);
	free_netdev(netdev);
	return err;
}


static void __devexit
vmxnet3_remove_device(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct vmxnet3_adapter *adapter = netdev_priv(netdev);
	int size = 0;
	int num_rx_queues;

#ifdef VMXNET3_RSS
	if (enable_mq)
		num_rx_queues = min(VMXNET3_DEVICE_MAX_RX_QUEUES,
				    (int)num_online_cpus());
	else
#endif
		num_rx_queues = 1;
	num_rx_queues = rounddown_pow_of_two(num_rx_queues);

	cancel_work_sync(&adapter->work);

	unregister_netdev(netdev);

	vmxnet3_free_intr_resources(adapter);
	vmxnet3_free_pci_resources(adapter);
#ifdef VMXNET3_RSS
	kfree(adapter->rss_conf);
#endif
	kfree(adapter->pm_conf);

	size = sizeof(struct Vmxnet3_TxQueueDesc) * adapter->num_tx_queues;
	size += sizeof(struct Vmxnet3_RxQueueDesc) * num_rx_queues;
	pci_free_consistent(adapter->pdev, size, adapter->tqd_start,
			    adapter->queue_desc_pa);
	pci_free_consistent(adapter->pdev, sizeof(struct Vmxnet3_DriverShared),
			    adapter->shared, adapter->shared_pa);
	free_netdev(netdev);
}


#ifdef CONFIG_PM

static int
vmxnet3_suspend(struct device *device)
{
	struct pci_dev *pdev = to_pci_dev(device);
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct vmxnet3_adapter *adapter = netdev_priv(netdev);
	struct Vmxnet3_PMConf *pmConf;
	struct ethhdr *ehdr;
	struct arphdr *ahdr;
	u8 *arpreq;
	struct in_device *in_dev;
	struct in_ifaddr *ifa;
	unsigned long flags;
	int i = 0;

	if (!netif_running(netdev))
		return 0;

	for (i = 0; i < adapter->num_rx_queues; i++)
		napi_disable(&adapter->rx_queue[i].napi);

	vmxnet3_disable_all_intrs(adapter);
	vmxnet3_free_irqs(adapter);
	vmxnet3_free_intr_resources(adapter);

	netif_device_detach(netdev);
	netif_tx_stop_all_queues(netdev);

	/* Create wake-up filters. */
	pmConf = adapter->pm_conf;
	memset(pmConf, 0, sizeof(*pmConf));

	if (adapter->wol & WAKE_UCAST) {
		pmConf->filters[i].patternSize = ETH_ALEN;
		pmConf->filters[i].maskSize = 1;
		memcpy(pmConf->filters[i].pattern, netdev->dev_addr, ETH_ALEN);
		pmConf->filters[i].mask[0] = 0x3F; /* LSB ETH_ALEN bits */

		pmConf->wakeUpEvents |= VMXNET3_PM_WAKEUP_FILTER;
		i++;
	}

	if (adapter->wol & WAKE_ARP) {
		in_dev = in_dev_get(netdev);
		if (!in_dev)
			goto skip_arp;

		ifa = (struct in_ifaddr *)in_dev->ifa_list;
		if (!ifa)
			goto skip_arp;

		pmConf->filters[i].patternSize = ETH_HLEN + /* Ethernet header*/
			sizeof(struct arphdr) +		/* ARP header */
			2 * ETH_ALEN +		/* 2 Ethernet addresses*/
			2 * sizeof(u32);	/*2 IPv4 addresses */
		pmConf->filters[i].maskSize =
			(pmConf->filters[i].patternSize - 1) / 8 + 1;

		/* ETH_P_ARP in Ethernet header. */
		ehdr = (struct ethhdr *)pmConf->filters[i].pattern;
		ehdr->h_proto = htons(ETH_P_ARP);

		/* ARPOP_REQUEST in ARP header. */
		ahdr = (struct arphdr *)&pmConf->filters[i].pattern[ETH_HLEN];
		ahdr->ar_op = htons(ARPOP_REQUEST);
		arpreq = (u8 *)(ahdr + 1);

		/* The Unicast IPv4 address in 'tip' field. */
		arpreq += 2 * ETH_ALEN + sizeof(u32);
		*(u32 *)arpreq = ifa->ifa_address;

		/* The mask for the relevant bits. */
		pmConf->filters[i].mask[0] = 0x00;
		pmConf->filters[i].mask[1] = 0x30; /* ETH_P_ARP */
		pmConf->filters[i].mask[2] = 0x30; /* ARPOP_REQUEST */
		pmConf->filters[i].mask[3] = 0x00;
		pmConf->filters[i].mask[4] = 0xC0; /* IPv4 TIP */
		pmConf->filters[i].mask[5] = 0x03; /* IPv4 TIP */
		in_dev_put(in_dev);

		pmConf->wakeUpEvents |= VMXNET3_PM_WAKEUP_FILTER;
		i++;
	}

skip_arp:
	if (adapter->wol & WAKE_MAGIC)
		pmConf->wakeUpEvents |= VMXNET3_PM_WAKEUP_MAGIC;

	pmConf->numFilters = i;

	adapter->shared->devRead.pmConfDesc.confVer = cpu_to_le32(1);
	adapter->shared->devRead.pmConfDesc.confLen = cpu_to_le32(sizeof(
								  *pmConf));
	adapter->shared->devRead.pmConfDesc.confPA = cpu_to_le64(virt_to_phys(
								 pmConf));

	spin_lock_irqsave(&adapter->cmd_lock, flags);
	VMXNET3_WRITE_BAR1_REG(adapter, VMXNET3_REG_CMD,
			       VMXNET3_CMD_UPDATE_PMCFG);
	spin_unlock_irqrestore(&adapter->cmd_lock, flags);

	pci_save_state(pdev);
	pci_enable_wake(pdev, pci_choose_state(pdev, PMSG_SUSPEND),
			adapter->wol);
	pci_disable_device(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, PMSG_SUSPEND));

	return 0;
}


static int
vmxnet3_resume(struct device *device)
{
	int err, i = 0;
	unsigned long flags;
	struct pci_dev *pdev = to_pci_dev(device);
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct vmxnet3_adapter *adapter = netdev_priv(netdev);
	struct Vmxnet3_PMConf *pmConf;

	if (!netif_running(netdev))
		return 0;

	/* Destroy wake-up filters. */
	pmConf = adapter->pm_conf;
	memset(pmConf, 0, sizeof(*pmConf));

	adapter->shared->devRead.pmConfDesc.confVer = cpu_to_le32(1);
	adapter->shared->devRead.pmConfDesc.confLen = cpu_to_le32(sizeof(
								  *pmConf));
	adapter->shared->devRead.pmConfDesc.confPA = cpu_to_le64(virt_to_phys(
								 pmConf));

	netif_device_attach(netdev);
	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	err = pci_enable_device_mem(pdev);
	if (err != 0)
		return err;

	pci_enable_wake(pdev, PCI_D0, 0);

	spin_lock_irqsave(&adapter->cmd_lock, flags);
	VMXNET3_WRITE_BAR1_REG(adapter, VMXNET3_REG_CMD,
			       VMXNET3_CMD_UPDATE_PMCFG);
	spin_unlock_irqrestore(&adapter->cmd_lock, flags);
	vmxnet3_alloc_intr_resources(adapter);
	vmxnet3_request_irqs(adapter);
	for (i = 0; i < adapter->num_rx_queues; i++)
		napi_enable(&adapter->rx_queue[i].napi);
	vmxnet3_enable_all_intrs(adapter);

	return 0;
}

static const struct dev_pm_ops vmxnet3_pm_ops = {
	.suspend = vmxnet3_suspend,
	.resume = vmxnet3_resume,
};
#endif

static struct pci_driver vmxnet3_driver = {
	.name		= vmxnet3_driver_name,
	.id_table	= vmxnet3_pciid_table,
	.probe		= vmxnet3_probe_device,
	.remove		= __devexit_p(vmxnet3_remove_device),
#ifdef CONFIG_PM
	.driver.pm	= &vmxnet3_pm_ops,
#endif
};


static int __init
vmxnet3_init_module(void)
{
	printk(KERN_INFO "%s - version %s\n", VMXNET3_DRIVER_DESC,
		VMXNET3_DRIVER_VERSION_REPORT);
	return pci_register_driver(&vmxnet3_driver);
}