	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);
	dma_addr_t txbd_paddr, skb_paddr;
	struct qtnf_tx_bd *txbd;
	int len, i;
	u32 info;
	int ret = 0;

	if (!qtnf_tx_queue_ready(priv)) {
		if (skb->dev)
			netif_stop_queue(skb->dev);

		return NETDEV_TX_BUSY;
	}

	i = priv->tx_bd_w_index;
		dev_kfree_skb_any(skb);
	}

	qtnf_pcie_data_tx_reclaim(priv);
	priv->tx_done_count++;

	return NETDEV_TX_OK;
}

	strcpy(bus->fwname, QTN_PCI_PEARL_FW_NAME);
	init_completion(&bus->request_firmware_complete);
	mutex_init(&bus->bus_lock);
	spin_lock_init(&pcie_priv->irq_lock);
	spin_lock_init(&pcie_priv->tx_reclaim_lock);

	/* init stats */