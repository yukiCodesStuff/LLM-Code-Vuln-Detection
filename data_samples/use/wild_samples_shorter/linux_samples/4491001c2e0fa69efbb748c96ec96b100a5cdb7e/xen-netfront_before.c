MODULE_PARM_DESC(max_queues,
		 "Maximum number of queues per virtual interface");

#define XENNET_TIMEOUT  (5 * HZ)

static const struct ethtool_ops xennet_ethtool_ops;

	/* Is device behaving sane? */
	bool broken;

	atomic_t rx_gso_checksum_fixup;
};

struct netfront_rx_info {
	return nxmit;
}


#define MAX_XEN_SKB_FRAGS (65536 / XEN_PAGE_SIZE + 1)

static netdev_tx_t xennet_start_xmit(struct sk_buff *skb, struct net_device *dev)

	/* The first req should be at least ETH_HLEN size or the packet will be
	 * dropped by netback.
	 */
	if (unlikely(PAGE_SIZE - offset < ETH_HLEN)) {
		nskb = skb_copy(skb, GFP_ATOMIC);
		if (!nskb)
			goto drop;
		dev_consume_skb_any(skb);
		skb = nskb;

	info->netdev->irq = 0;

	/* Check if backend supports multiple queues */
	max_queues = xenbus_read_unsigned(info->xbdev->otherend,
					  "multi-queue-max-queues", 1);
	num_queues = min(max_queues, xennet_max_queues);
		return err;
	if (np->netback_has_xdp_headroom)
		pr_info("backend supports XDP headroom\n");

	/* talk_to_netback() sets the correct number of queues */
	num_queues = dev->real_num_tx_queues;
