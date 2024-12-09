	if (vif->multicast_control && skb->pkt_type == PACKET_MULTICAST) {
		struct ethhdr *eth = (struct ethhdr *)skb->data;

		if (!xenvif_mcast_match(vif, eth->h_dest))
			goto drop;
	}

	cb = XENVIF_RX_CB(skb);
	cb->expires = jiffies + vif->drain_timeout;

	/* If there is no hash algorithm configured then make sure there
	 * is no hash information in the socket buffer otherwise it
	 * would be incorrectly forwarded to the frontend.
	 */
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
{
	struct xenvif *vif = netdev_priv(dev);
	struct xenvif_queue *queue = NULL;
	unsigned int num_queues;
	u64 rx_bytes = 0;
	u64 rx_packets = 0;
	u64 tx_bytes = 0;
	u64 tx_packets = 0;
	unsigned int index;

	rcu_read_lock();
	num_queues = READ_ONCE(vif->num_queues);

	/* Aggregate tx and rx stats from each queue */
	for (index = 0; index < num_queues; ++index) {
		queue = &vif->queues[index];
		rx_bytes += queue->stats.rx_bytes;
		rx_packets += queue->stats.rx_packets;
		tx_bytes += queue->stats.tx_bytes;
		tx_packets += queue->stats.tx_packets;
	}