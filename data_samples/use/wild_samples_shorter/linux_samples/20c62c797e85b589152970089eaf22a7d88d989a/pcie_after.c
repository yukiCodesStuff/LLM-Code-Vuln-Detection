	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);
	dma_addr_t txbd_paddr, skb_paddr;
	struct qtnf_tx_bd *txbd;
	unsigned long flags;
	int len, i;
	u32 info;
	int ret = 0;

	spin_lock_irqsave(&priv->tx0_lock, flags);

	if (!qtnf_tx_queue_ready(priv)) {
		if (skb->dev)
			netif_stop_queue(skb->dev);

		spin_unlock_irqrestore(&priv->tx0_lock, flags);
		return NETDEV_TX_BUSY;
	}

	i = priv->tx_bd_w_index;
		dev_kfree_skb_any(skb);
	}

	priv->tx_done_count++;
	spin_unlock_irqrestore(&priv->tx0_lock, flags);

	qtnf_pcie_data_tx_reclaim(priv);

	return NETDEV_TX_OK;
}

	strcpy(bus->fwname, QTN_PCI_PEARL_FW_NAME);
	init_completion(&bus->request_firmware_complete);
	mutex_init(&bus->bus_lock);
	spin_lock_init(&pcie_priv->tx0_lock);
	spin_lock_init(&pcie_priv->irq_lock);
	spin_lock_init(&pcie_priv->tx_reclaim_lock);

	/* init stats */