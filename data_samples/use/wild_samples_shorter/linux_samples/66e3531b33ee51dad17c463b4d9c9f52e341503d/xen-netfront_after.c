	struct device *dev = &queue->info->netdev->dev;
	struct bpf_prog *xdp_prog;
	struct xdp_buff xdp;
	int slots = 1;
	int err = 0;
	u32 verdict;

			goto next;
		}

		if (!gnttab_end_foreign_access_ref(ref, 0)) {
			dev_alert(dev,
				  "Grant still in use by backend domain\n");
			queue->info->broken = true;
			dev_alert(dev, "Disabled for further use\n");
			return -EINVAL;
		}

		gnttab_release_grant_reference(&queue->gref_rx_head, ref);

		rcu_read_lock();
					   &need_xdp_flush);

		if (unlikely(err)) {
			if (queue->info->broken) {
				spin_unlock(&queue->rx_lock);
				return 0;
			}
err:
			while ((skb = __skb_dequeue(&tmpq)))
				__skb_queue_tail(&errq, skb);
			dev->stats.rx_errors++;
			struct netfront_queue *queue, unsigned int feature_split_evtchn)
{
	struct xen_netif_tx_sring *txs;
	struct xen_netif_rx_sring *rxs = NULL;
	grant_ref_t gref;
	int err;

	queue->tx_ring_ref = GRANT_INVALID_REF;

	err = xenbus_grant_ring(dev, txs, 1, &gref);
	if (err < 0)
		goto fail;
	queue->tx_ring_ref = gref;

	rxs = (struct xen_netif_rx_sring *)get_zeroed_page(GFP_NOIO | __GFP_HIGH);
	if (!rxs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating rx ring page");
		goto fail;
	}
	SHARED_RING_INIT(rxs);
	FRONT_RING_INIT(&queue->rx, rxs, XEN_PAGE_SIZE);

	err = xenbus_grant_ring(dev, rxs, 1, &gref);
	if (err < 0)
		goto fail;
	queue->rx_ring_ref = gref;

	if (feature_split_evtchn)
		err = setup_netfront_split(queue);
		err = setup_netfront_single(queue);

	if (err)
		goto fail;

	return 0;

	/* If we fail to setup netfront, it is safe to just revoke access to
	 * granted pages because backend is not accessing it at this point.
	 */
 fail:
	if (queue->rx_ring_ref != GRANT_INVALID_REF) {
		gnttab_end_foreign_access(queue->rx_ring_ref, 0,
					  (unsigned long)rxs);
		queue->rx_ring_ref = GRANT_INVALID_REF;
	} else {
		free_page((unsigned long)rxs);
	}
	if (queue->tx_ring_ref != GRANT_INVALID_REF) {
		gnttab_end_foreign_access(queue->tx_ring_ref, 0,
					  (unsigned long)txs);
		queue->tx_ring_ref = GRANT_INVALID_REF;
	} else {
		free_page((unsigned long)txs);
	}
	return err;
}

/* Queue-specific initialisation