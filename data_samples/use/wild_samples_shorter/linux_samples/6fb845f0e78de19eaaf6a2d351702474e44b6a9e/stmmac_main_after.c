
	tx_q = &priv->tx_queue[queue];

	if (priv->tx_path_in_lpi_mode)
		stmmac_disable_eee_mode(priv);

	/* Manage oversized TCP frames for GMAC4 device */
	if (skb_is_gso(skb) && priv->tso) {
		if (skb_shinfo(skb)->gso_type & (SKB_GSO_TCPV4 | SKB_GSO_TCPV6)) {
			/*
			 * There is no way to determine the number of TSO
			 * capable Queues. Let's use always the Queue 0
			 * because if TSO is supported then at least this
			 * one will be capable.
			 */
			skb_set_queue_mapping(skb, 0);

			return stmmac_tso_xmit(skb, dev);
		}
	}

	if (unlikely(stmmac_tx_avail(priv, queue) < nfrags + 1)) {
		if (!netif_tx_queue_stopped(netdev_get_tx_queue(dev, queue))) {
		return NETDEV_TX_BUSY;
	}

	entry = tx_q->cur_tx;
	first_entry = entry;
	WARN_ON(tx_q->tx_skbuff[first_entry]);
