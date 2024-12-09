	if (vif->hash.alg == XEN_NETIF_CTRL_HASH_ALGORITHM_NONE)
		skb_clear_hash(skb);

	if (!xenvif_rx_queue_tail(queue, skb))
		goto drop;

	xenvif_kick_thread(queue);

	return NETDEV_TX_OK;

 drop:
	vif->dev->stats.tx_dropped++;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

static struct net_device_stats *xenvif_get_stats(struct net_device *dev)