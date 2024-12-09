			} else {
				/* drop the frame */
				err = -EINVAL;
			}
		}
		rcu_read_unlock();

		__skb_queue_tail(list, skb);

next:
		if (!(rx->flags & XEN_NETRXF_more_data))
			break;

		if (cons + slots == rp) {
			if (net_ratelimit())
				dev_warn(dev, "Need more slots\n");
			err = -ENOENT;
			break;
		}

		RING_COPY_RESPONSE(&queue->rx, cons + slots, &rx_local);
		rx = &rx_local;
		skb = xennet_get_rx_skb(queue, cons + slots);
		ref = xennet_get_rx_ref(queue, cons + slots);
		slots++;
	}

	if (unlikely(slots > max)) {
		if (net_ratelimit())
			dev_warn(dev, "Too many slots\n");
		err = -E2BIG;
	}

	if (unlikely(err))
		xennet_set_rx_rsp_cons(queue, cons + slots);

	return err;
}

static int xennet_set_skb_gso(struct sk_buff *skb,
			      struct xen_netif_extra_info *gso)
{
	if (!gso->u.gso.size) {
		if (net_ratelimit())
			pr_warn("GSO size must not be zero\n");
		return -EINVAL;
	}

	if (gso->u.gso.type != XEN_NETIF_GSO_TYPE_TCPV4 &&
	    gso->u.gso.type != XEN_NETIF_GSO_TYPE_TCPV6) {
		if (net_ratelimit())
			pr_warn("Bad GSO type %d\n", gso->u.gso.type);
		return -EINVAL;
	}

	skb_shinfo(skb)->gso_size = gso->u.gso.size;
	skb_shinfo(skb)->gso_type =
		(gso->u.gso.type == XEN_NETIF_GSO_TYPE_TCPV4) ?
		SKB_GSO_TCPV4 :
		SKB_GSO_TCPV6;

	/* Header must be checked, and gso_segs computed. */
	skb_shinfo(skb)->gso_type |= SKB_GSO_DODGY;
	skb_shinfo(skb)->gso_segs = 0;

	return 0;
}

static int xennet_fill_frags(struct netfront_queue *queue,
			     struct sk_buff *skb,
			     struct sk_buff_head *list)
{
	RING_IDX cons = queue->rx.rsp_cons;
	struct sk_buff *nskb;

	while ((nskb = __skb_dequeue(list))) {
		struct xen_netif_rx_response rx;
		skb_frag_t *nfrag = &skb_shinfo(nskb)->frags[0];

		RING_COPY_RESPONSE(&queue->rx, ++cons, &rx);

		if (skb_shinfo(skb)->nr_frags == MAX_SKB_FRAGS) {
			unsigned int pull_to = NETFRONT_SKB_CB(skb)->pull_to;

			BUG_ON(pull_to < skb_headlen(skb));
			__pskb_pull_tail(skb, pull_to - skb_headlen(skb));
		}
		if (unlikely(skb_shinfo(skb)->nr_frags >= MAX_SKB_FRAGS)) {
			xennet_set_rx_rsp_cons(queue,
					       ++cons + skb_queue_len(list));
			kfree_skb(nskb);
			return -ENOENT;
		}

		skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags,
				skb_frag_page(nfrag),
				rx.offset, rx.status, PAGE_SIZE);

		skb_shinfo(nskb)->nr_frags = 0;
		kfree_skb(nskb);
	}

	xennet_set_rx_rsp_cons(queue, cons);

	return 0;
}

static int checksum_setup(struct net_device *dev, struct sk_buff *skb)
{
	bool recalculate_partial_csum = false;

	/*
	 * A GSO SKB must be CHECKSUM_PARTIAL. However some buggy
	 * peers can fail to set NETRXF_csum_blank when sending a GSO
	 * frame. In this case force the SKB to CHECKSUM_PARTIAL and
	 * recalculate the partial checksum.
	 */
	if (skb->ip_summed != CHECKSUM_PARTIAL && skb_is_gso(skb)) {
		struct netfront_info *np = netdev_priv(dev);
		atomic_inc(&np->rx_gso_checksum_fixup);
		skb->ip_summed = CHECKSUM_PARTIAL;
		recalculate_partial_csum = true;
	}

	/* A non-CHECKSUM_PARTIAL SKB does not require setup. */
	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return 0;

	return skb_checksum_setup(skb, recalculate_partial_csum);
}

static int handle_incoming_queue(struct netfront_queue *queue,
				 struct sk_buff_head *rxq)
{
	struct netfront_stats *rx_stats = this_cpu_ptr(queue->info->rx_stats);
	int packets_dropped = 0;
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(rxq)) != NULL) {
		int pull_to = NETFRONT_SKB_CB(skb)->pull_to;

		if (pull_to > skb_headlen(skb))
			__pskb_pull_tail(skb, pull_to - skb_headlen(skb));

		/* Ethernet work: Delayed to here as it peeks the header. */
		skb->protocol = eth_type_trans(skb, queue->info->netdev);
		skb_reset_network_header(skb);

		if (checksum_setup(queue->info->netdev, skb)) {
			kfree_skb(skb);
			packets_dropped++;
			queue->info->netdev->stats.rx_errors++;
			continue;
		}

		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->packets++;
		rx_stats->bytes += skb->len;
		u64_stats_update_end(&rx_stats->syncp);

		/* Pass it up. */
		napi_gro_receive(&queue->napi, skb);
	}

	return packets_dropped;
}

static int xennet_poll(struct napi_struct *napi, int budget)
{
	struct netfront_queue *queue = container_of(napi, struct netfront_queue, napi);
	struct net_device *dev = queue->info->netdev;
	struct sk_buff *skb;
	struct netfront_rx_info rinfo;
	struct xen_netif_rx_response *rx = &rinfo.rx;
	struct xen_netif_extra_info *extras = rinfo.extras;
	RING_IDX i, rp;
	int work_done;
	struct sk_buff_head rxq;
	struct sk_buff_head errq;
	struct sk_buff_head tmpq;
	int err;
	bool need_xdp_flush = false;

	spin_lock(&queue->rx_lock);

	skb_queue_head_init(&rxq);
	skb_queue_head_init(&errq);
	skb_queue_head_init(&tmpq);

	rp = queue->rx.sring->rsp_prod;
	if (RING_RESPONSE_PROD_OVERFLOW(&queue->rx, rp)) {
		dev_alert(&dev->dev, "Illegal number of responses %u\n",
			  rp - queue->rx.rsp_cons);
		queue->info->broken = true;
		spin_unlock(&queue->rx_lock);
		return 0;
	}
	rmb(); /* Ensure we see queued responses up to 'rp'. */

	i = queue->rx.rsp_cons;
	work_done = 0;
	while ((i != rp) && (work_done < budget)) {
		RING_COPY_RESPONSE(&queue->rx, i, rx);
		memset(extras, 0, sizeof(rinfo.extras));

		err = xennet_get_responses(queue, &rinfo, rp, &tmpq,
					   &need_xdp_flush);

		if (unlikely(err)) {
			if (queue->info->broken) {
				spin_unlock(&queue->rx_lock);
				return 0;
			}