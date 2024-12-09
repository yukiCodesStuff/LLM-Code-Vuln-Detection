{
	struct xen_netif_rx_response *rx = &rinfo->rx, rx_local;
	int max = XEN_NETIF_NR_SLOTS_MIN + (rx->status <= RX_COPY_THRESHOLD);
	RING_IDX cons = queue->rx.rsp_cons;
	struct sk_buff *skb = xennet_get_rx_skb(queue, cons);
	struct xen_netif_extra_info *extras = rinfo->extras;
	grant_ref_t ref = xennet_get_rx_ref(queue, cons);
	struct device *dev = &queue->info->netdev->dev;
	struct bpf_prog *xdp_prog;
	struct xdp_buff xdp;
	unsigned long ret;
	int slots = 1;
	int err = 0;
	u32 verdict;

	if (rx->flags & XEN_NETRXF_extra_info) {
		err = xennet_get_extras(queue, extras, rp);
		if (!err) {
			if (extras[XEN_NETIF_EXTRA_TYPE_XDP - 1].type) {
				struct xen_netif_extra_info *xdp;

				xdp = &extras[XEN_NETIF_EXTRA_TYPE_XDP - 1];
				rx->offset = xdp->u.xdp.headroom;
			}
		}
		cons = queue->rx.rsp_cons;
	}
		if (ref == GRANT_INVALID_REF) {
			if (net_ratelimit())
				dev_warn(dev, "Bad rx response id %d.\n",
					 rx->id);
			err = -EINVAL;
			goto next;
		}

		ret = gnttab_end_foreign_access_ref(ref, 0);
		BUG_ON(!ret);

		gnttab_release_grant_reference(&queue->gref_rx_head, ref);

		rcu_read_lock();
		xdp_prog = rcu_dereference(queue->xdp_prog);
		if (xdp_prog) {
			if (!(rx->flags & XEN_NETRXF_more_data)) {
				/* currently only a single page contains data */
				verdict = xennet_run_xdp(queue,
							 skb_frag_page(&skb_shinfo(skb)->frags[0]),
							 rx, xdp_prog, &xdp, need_xdp_flush);
				if (verdict != XDP_PASS)
					err = -EINVAL;
			} else {
				/* drop the frame */
				err = -EINVAL;
			}
		}
	while ((i != rp) && (work_done < budget)) {
		RING_COPY_RESPONSE(&queue->rx, i, rx);
		memset(extras, 0, sizeof(rinfo.extras));

		err = xennet_get_responses(queue, &rinfo, rp, &tmpq,
					   &need_xdp_flush);

		if (unlikely(err)) {
err:
			while ((skb = __skb_dequeue(&tmpq)))
				__skb_queue_tail(&errq, skb);
			dev->stats.rx_errors++;
			i = queue->rx.rsp_cons;
			continue;
		}
{
	struct xen_netif_tx_sring *txs;
	struct xen_netif_rx_sring *rxs;
	grant_ref_t gref;
	int err;

	queue->tx_ring_ref = GRANT_INVALID_REF;
	queue->rx_ring_ref = GRANT_INVALID_REF;
	queue->rx.sring = NULL;
	queue->tx.sring = NULL;

	txs = (struct xen_netif_tx_sring *)get_zeroed_page(GFP_NOIO | __GFP_HIGH);
	if (!txs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating tx ring page");
		goto fail;
	}
	SHARED_RING_INIT(txs);
	FRONT_RING_INIT(&queue->tx, txs, XEN_PAGE_SIZE);

	err = xenbus_grant_ring(dev, txs, 1, &gref);
	if (err < 0)
		goto grant_tx_ring_fail;
	queue->tx_ring_ref = gref;

	rxs = (struct xen_netif_rx_sring *)get_zeroed_page(GFP_NOIO | __GFP_HIGH);
	if (!rxs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating rx ring page");
		goto alloc_rx_ring_fail;
	}
	SHARED_RING_INIT(rxs);
	FRONT_RING_INIT(&queue->rx, rxs, XEN_PAGE_SIZE);

	err = xenbus_grant_ring(dev, rxs, 1, &gref);
	if (err < 0)
		goto grant_rx_ring_fail;
	queue->rx_ring_ref = gref;

	if (feature_split_evtchn)
		err = setup_netfront_split(queue);
	/* setup single event channel if
	 *  a) feature-split-event-channels == 0
	 *  b) feature-split-event-channels == 1 but failed to setup
	 */
	if (!feature_split_evtchn || err)
		err = setup_netfront_single(queue);

	if (err)
		goto alloc_evtchn_fail;

	return 0;

	/* If we fail to setup netfront, it is safe to just revoke access to
	 * granted pages because backend is not accessing it at this point.
	 */
alloc_evtchn_fail:
	gnttab_end_foreign_access_ref(queue->rx_ring_ref, 0);
grant_rx_ring_fail:
	free_page((unsigned long)rxs);
alloc_rx_ring_fail:
	gnttab_end_foreign_access_ref(queue->tx_ring_ref, 0);
grant_tx_ring_fail:
	free_page((unsigned long)txs);
fail:
	return err;
}
	if (!txs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating tx ring page");
		goto fail;
	}
	SHARED_RING_INIT(txs);
	FRONT_RING_INIT(&queue->tx, txs, XEN_PAGE_SIZE);

	err = xenbus_grant_ring(dev, txs, 1, &gref);
	if (err < 0)
		goto grant_tx_ring_fail;
	queue->tx_ring_ref = gref;

	rxs = (struct xen_netif_rx_sring *)get_zeroed_page(GFP_NOIO | __GFP_HIGH);
	if (!rxs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating rx ring page");
		goto alloc_rx_ring_fail;
	}
	if (!rxs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating rx ring page");
		goto alloc_rx_ring_fail;
	}
	SHARED_RING_INIT(rxs);
	FRONT_RING_INIT(&queue->rx, rxs, XEN_PAGE_SIZE);

	err = xenbus_grant_ring(dev, rxs, 1, &gref);
	if (err < 0)
		goto grant_rx_ring_fail;
	queue->rx_ring_ref = gref;

	if (feature_split_evtchn)
		err = setup_netfront_split(queue);
	/* setup single event channel if
	 *  a) feature-split-event-channels == 0
	 *  b) feature-split-event-channels == 1 but failed to setup
	 */
	if (!feature_split_evtchn || err)
		err = setup_netfront_single(queue);

	if (err)
		goto alloc_evtchn_fail;

	return 0;

	/* If we fail to setup netfront, it is safe to just revoke access to
	 * granted pages because backend is not accessing it at this point.
	 */
alloc_evtchn_fail:
	gnttab_end_foreign_access_ref(queue->rx_ring_ref, 0);
grant_rx_ring_fail:
	free_page((unsigned long)rxs);
alloc_rx_ring_fail:
	gnttab_end_foreign_access_ref(queue->tx_ring_ref, 0);
grant_tx_ring_fail:
	free_page((unsigned long)txs);
fail:
	return err;
}

/* Queue-specific initialisation
 * This used to be done in xennet_create_dev() but must now
 * be run per-queue.
 */
static int xennet_init_queue(struct netfront_queue *queue)
{
	unsigned short i;
	int err = 0;
	char *devid;

	spin_lock_init(&queue->tx_lock);
	spin_lock_init(&queue->rx_lock);
	spin_lock_init(&queue->rx_cons_lock);

	timer_setup(&queue->rx_refill_timer, rx_refill_timeout, 0);

	devid = strrchr(queue->info->xbdev->nodename, '/') + 1;
	snprintf(queue->name, sizeof(queue->name), "vif%s-q%u",
		 devid, queue->id);

	/* Initialise tx_skb_freelist as a free chain containing every entry. */
	queue->tx_skb_freelist = 0;
	queue->tx_pend_queue = TX_LINK_NONE;
	for (i = 0; i < NET_TX_RING_SIZE; i++) {
		queue->tx_link[i] = i + 1;
		queue->grant_tx_ref[i] = GRANT_INVALID_REF;
		queue->grant_tx_page[i] = NULL;
	}