MODULE_PARM_DESC(max_queues,
		 "Maximum number of queues per virtual interface");

static bool __read_mostly xennet_trusted = true;
module_param_named(trusted, xennet_trusted, bool, 0644);
MODULE_PARM_DESC(trusted, "Is the backend trusted");

#define XENNET_TIMEOUT  (5 * HZ)

static const struct ethtool_ops xennet_ethtool_ops;

	/* Is device behaving sane? */
	bool broken;

	/* Should skbs be bounced into a zeroed buffer? */
	bool bounce;

	atomic_t rx_gso_checksum_fixup;
};

struct netfront_rx_info {
	return nxmit;
}

struct sk_buff *bounce_skb(const struct sk_buff *skb)
{
	unsigned int headerlen = skb_headroom(skb);
	/* Align size to allocate full pages and avoid contiguous data leaks */
	unsigned int size = ALIGN(skb_end_offset(skb) + skb->data_len,
				  XEN_PAGE_SIZE);
	struct sk_buff *n = alloc_skb(size, GFP_ATOMIC | __GFP_ZERO);

	if (!n)
		return NULL;

	if (!IS_ALIGNED((uintptr_t)n->head, XEN_PAGE_SIZE)) {
		WARN_ONCE(1, "misaligned skb allocated\n");
		kfree_skb(n);
		return NULL;
	}

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	BUG_ON(skb_copy_bits(skb, -headerlen, n->head, headerlen + skb->len));

	skb_copy_header(n, skb);
	return n;
}

#define MAX_XEN_SKB_FRAGS (65536 / XEN_PAGE_SIZE + 1)

static netdev_tx_t xennet_start_xmit(struct sk_buff *skb, struct net_device *dev)

	/* The first req should be at least ETH_HLEN size or the packet will be
	 * dropped by netback.
	 *
	 * If the backend is not trusted bounce all data to zeroed pages to
	 * avoid exposing contiguous data on the granted page not belonging to
	 * the skb.
	 */
	if (np->bounce || unlikely(PAGE_SIZE - offset < ETH_HLEN)) {
		nskb = bounce_skb(skb);
		if (!nskb)
			goto drop;
		dev_consume_skb_any(skb);
		skb = nskb;

	info->netdev->irq = 0;

	/* Check if backend is trusted. */
	info->bounce = !xennet_trusted ||
		       !xenbus_read_unsigned(dev->nodename, "trusted", 1);

	/* Check if backend supports multiple queues */
	max_queues = xenbus_read_unsigned(info->xbdev->otherend,
					  "multi-queue-max-queues", 1);
	num_queues = min(max_queues, xennet_max_queues);
		return err;
	if (np->netback_has_xdp_headroom)
		pr_info("backend supports XDP headroom\n");
	if (np->bounce)
		dev_info(&np->xbdev->dev,
			 "bouncing transmitted data to zeroed pages\n");

	/* talk_to_netback() sets the correct number of queues */
	num_queues = dev->real_num_tx_queues;
