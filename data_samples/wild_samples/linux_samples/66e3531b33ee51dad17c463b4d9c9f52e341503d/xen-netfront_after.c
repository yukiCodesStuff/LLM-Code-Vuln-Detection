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

		if (!gnttab_end_foreign_access_ref(ref, 0)) {
			dev_alert(dev,
				  "Grant still in use by backend domain\n");
			queue->info->broken = true;
			dev_alert(dev, "Disabled for further use\n");
			return -EINVAL;
		}

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
			if (queue->info->broken) {
				spin_unlock(&queue->rx_lock);
				return 0;
			}
err:
			while ((skb = __skb_dequeue(&tmpq)))
				__skb_queue_tail(&errq, skb);
			dev->stats.rx_errors++;
			i = queue->rx.rsp_cons;
			continue;
		}
{
	struct xen_netif_tx_sring *txs;
	struct xen_netif_rx_sring *rxs = NULL;
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
	/* setup single event channel if
	 *  a) feature-split-event-channels == 0
	 *  b) feature-split-event-channels == 1 but failed to setup
	 */
	if (!feature_split_evtchn || err)
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
	queue->tx_link[NET_TX_RING_SIZE - 1] = TX_LINK_NONE;

	/* Clear out rx_skbs */
	for (i = 0; i < NET_RX_RING_SIZE; i++) {
		queue->rx_skbs[i] = NULL;
		queue->grant_rx_ref[i] = GRANT_INVALID_REF;
	}

	/* A grant for every tx ring slot */
	if (gnttab_alloc_grant_references(NET_TX_RING_SIZE,
					  &queue->gref_tx_head) < 0) {
		pr_alert("can't alloc tx grant refs\n");
		err = -ENOMEM;
		goto exit;
	}

	/* A grant for every rx ring slot */
	if (gnttab_alloc_grant_references(NET_RX_RING_SIZE,
					  &queue->gref_rx_head) < 0) {
		pr_alert("can't alloc rx grant refs\n");
		err = -ENOMEM;
		goto exit_free_tx;
	}

	return 0;

 exit_free_tx:
	gnttab_free_grant_references(queue->gref_tx_head);
 exit:
	return err;
}

static int write_queue_xenstore_keys(struct netfront_queue *queue,
			   struct xenbus_transaction *xbt, int write_hierarchical)
{
	/* Write the queue-specific keys into XenStore in the traditional
	 * way for a single queue, or in a queue subkeys for multiple
	 * queues.
	 */
	struct xenbus_device *dev = queue->info->xbdev;
	int err;
	const char *message;
	char *path;
	size_t pathsize;

	/* Choose the correct place to write the keys */
	if (write_hierarchical) {
		pathsize = strlen(dev->nodename) + 10;
		path = kzalloc(pathsize, GFP_KERNEL);
		if (!path) {
			err = -ENOMEM;
			message = "out of memory while writing ring references";
			goto error;
		}
		snprintf(path, pathsize, "%s/queue-%u",
				dev->nodename, queue->id);
	} else {
		path = (char *)dev->nodename;
	}

	/* Write ring references */
	err = xenbus_printf(*xbt, path, "tx-ring-ref", "%u",
			queue->tx_ring_ref);
	if (err) {
		message = "writing tx-ring-ref";
		goto error;
	}

	err = xenbus_printf(*xbt, path, "rx-ring-ref", "%u",
			queue->rx_ring_ref);
	if (err) {
		message = "writing rx-ring-ref";
		goto error;
	}

	/* Write event channels; taking into account both shared
	 * and split event channel scenarios.
	 */
	if (queue->tx_evtchn == queue->rx_evtchn) {
		/* Shared event channel */
		err = xenbus_printf(*xbt, path,
				"event-channel", "%u", queue->tx_evtchn);
		if (err) {
			message = "writing event-channel";
			goto error;
		}
	} else {
		/* Split event channels */
		err = xenbus_printf(*xbt, path,
				"event-channel-tx", "%u", queue->tx_evtchn);
		if (err) {
			message = "writing event-channel-tx";
			goto error;
		}

		err = xenbus_printf(*xbt, path,
				"event-channel-rx", "%u", queue->rx_evtchn);
		if (err) {
			message = "writing event-channel-rx";
			goto error;
		}
	}

	if (write_hierarchical)
		kfree(path);
	return 0;

error:
	if (write_hierarchical)
		kfree(path);
	xenbus_dev_fatal(dev, err, "%s", message);
	return err;
}



static int xennet_create_page_pool(struct netfront_queue *queue)
{
	int err;
	struct page_pool_params pp_params = {
		.order = 0,
		.flags = 0,
		.pool_size = NET_RX_RING_SIZE,
		.nid = NUMA_NO_NODE,
		.dev = &queue->info->netdev->dev,
		.offset = XDP_PACKET_HEADROOM,
		.max_len = XEN_PAGE_SIZE - XDP_PACKET_HEADROOM,
	};

	queue->page_pool = page_pool_create(&pp_params);
	if (IS_ERR(queue->page_pool)) {
		err = PTR_ERR(queue->page_pool);
		queue->page_pool = NULL;
		return err;
	}

	err = xdp_rxq_info_reg(&queue->xdp_rxq, queue->info->netdev,
			       queue->id, 0);
	if (err) {
		netdev_err(queue->info->netdev, "xdp_rxq_info_reg failed\n");
		goto err_free_pp;
	}

	err = xdp_rxq_info_reg_mem_model(&queue->xdp_rxq,
					 MEM_TYPE_PAGE_POOL, queue->page_pool);
	if (err) {
		netdev_err(queue->info->netdev, "xdp_rxq_info_reg_mem_model failed\n");
		goto err_unregister_rxq;
	}
	return 0;

err_unregister_rxq:
	xdp_rxq_info_unreg(&queue->xdp_rxq);
err_free_pp:
	page_pool_destroy(queue->page_pool);
	queue->page_pool = NULL;
	return err;
}

static int xennet_create_queues(struct netfront_info *info,
				unsigned int *num_queues)
{
	unsigned int i;
	int ret;

	info->queues = kcalloc(*num_queues, sizeof(struct netfront_queue),
			       GFP_KERNEL);
	if (!info->queues)
		return -ENOMEM;

	for (i = 0; i < *num_queues; i++) {
		struct netfront_queue *queue = &info->queues[i];

		queue->id = i;
		queue->info = info;

		ret = xennet_init_queue(queue);
		if (ret < 0) {
			dev_warn(&info->xbdev->dev,
				 "only created %d queues\n", i);
			*num_queues = i;
			break;
		}

		/* use page pool recycling instead of buddy allocator */
		ret = xennet_create_page_pool(queue);
		if (ret < 0) {
			dev_err(&info->xbdev->dev, "can't allocate page pool\n");
			*num_queues = i;
			return ret;
		}

		netif_napi_add(queue->info->netdev, &queue->napi,
			       xennet_poll, 64);
		if (netif_running(info->netdev))
			napi_enable(&queue->napi);
	}

	netif_set_real_num_tx_queues(info->netdev, *num_queues);

	if (*num_queues == 0) {
		dev_err(&info->xbdev->dev, "no queues\n");
		return -EINVAL;
	}
	return 0;
}

/* Common code used when first setting up, and when resuming. */
static int talk_to_netback(struct xenbus_device *dev,
			   struct netfront_info *info)
{
	const char *message;
	struct xenbus_transaction xbt;
	int err;
	unsigned int feature_split_evtchn;
	unsigned int i = 0;
	unsigned int max_queues = 0;
	struct netfront_queue *queue = NULL;
	unsigned int num_queues = 1;
	u8 addr[ETH_ALEN];

	info->netdev->irq = 0;

	/* Check if backend supports multiple queues */
	max_queues = xenbus_read_unsigned(info->xbdev->otherend,
					  "multi-queue-max-queues", 1);
	num_queues = min(max_queues, xennet_max_queues);

	/* Check feature-split-event-channels */
	feature_split_evtchn = xenbus_read_unsigned(info->xbdev->otherend,
					"feature-split-event-channels", 0);

	/* Read mac addr. */
	err = xen_net_read_mac(dev, addr);
	if (err) {
		xenbus_dev_fatal(dev, err, "parsing %s/mac", dev->nodename);
		goto out_unlocked;
	}
	eth_hw_addr_set(info->netdev, addr);

	info->netback_has_xdp_headroom = xenbus_read_unsigned(info->xbdev->otherend,
							      "feature-xdp-headroom", 0);
	if (info->netback_has_xdp_headroom) {
		/* set the current xen-netfront xdp state */
		err = talk_to_netback_xdp(info, info->netfront_xdp_enabled ?
					  NETBACK_XDP_HEADROOM_ENABLE :
					  NETBACK_XDP_HEADROOM_DISABLE);
		if (err)
			goto out_unlocked;
	}

	rtnl_lock();
	if (info->queues)
		xennet_destroy_queues(info);

	/* For the case of a reconnect reset the "broken" indicator. */
	info->broken = false;

	err = xennet_create_queues(info, &num_queues);
	if (err < 0) {
		xenbus_dev_fatal(dev, err, "creating queues");
		kfree(info->queues);
		info->queues = NULL;
		goto out;
	}
	rtnl_unlock();

	/* Create shared ring, alloc event channel -- for each queue */
	for (i = 0; i < num_queues; ++i) {
		queue = &info->queues[i];
		err = setup_netfront(dev, queue, feature_split_evtchn);
		if (err)
			goto destroy_ring;
	}

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		xenbus_dev_fatal(dev, err, "starting transaction");
		goto destroy_ring;
	}

	if (xenbus_exists(XBT_NIL,
			  info->xbdev->otherend, "multi-queue-max-queues")) {
		/* Write the number of queues */
		err = xenbus_printf(xbt, dev->nodename,
				    "multi-queue-num-queues", "%u", num_queues);
		if (err) {
			message = "writing multi-queue-num-queues";
			goto abort_transaction_no_dev_fatal;
		}
	}

	if (num_queues == 1) {
		err = write_queue_xenstore_keys(&info->queues[0], &xbt, 0); /* flat */
		if (err)
			goto abort_transaction_no_dev_fatal;
	} else {
		/* Write the keys for each queue */
		for (i = 0; i < num_queues; ++i) {
			queue = &info->queues[i];
			err = write_queue_xenstore_keys(queue, &xbt, 1); /* hierarchical */
			if (err)
				goto abort_transaction_no_dev_fatal;
		}
	}

	/* The remaining keys are not queue-specific */
	err = xenbus_printf(xbt, dev->nodename, "request-rx-copy", "%u",
			    1);
	if (err) {
		message = "writing request-rx-copy";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-rx-notify", "%d", 1);
	if (err) {
		message = "writing feature-rx-notify";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-sg", "%d", 1);
	if (err) {
		message = "writing feature-sg";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-gso-tcpv4", "%d", 1);
	if (err) {
		message = "writing feature-gso-tcpv4";
		goto abort_transaction;
	}

	err = xenbus_write(xbt, dev->nodename, "feature-gso-tcpv6", "1");
	if (err) {
		message = "writing feature-gso-tcpv6";
		goto abort_transaction;
	}

	err = xenbus_write(xbt, dev->nodename, "feature-ipv6-csum-offload",
			   "1");
	if (err) {
		message = "writing feature-ipv6-csum-offload";
		goto abort_transaction;
	}

	err = xenbus_transaction_end(xbt, 0);
	if (err) {
		if (err == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, err, "completing transaction");
		goto destroy_ring;
	}

	return 0;

 abort_transaction:
	xenbus_dev_fatal(dev, err, "%s", message);
abort_transaction_no_dev_fatal:
	xenbus_transaction_end(xbt, 1);
 destroy_ring:
	xennet_disconnect_backend(info);
	rtnl_lock();
	xennet_destroy_queues(info);
 out:
	rtnl_unlock();
out_unlocked:
	device_unregister(&dev->dev);
	return err;
}

static int xennet_connect(struct net_device *dev)
{
	struct netfront_info *np = netdev_priv(dev);
	unsigned int num_queues = 0;
	int err;
	unsigned int j = 0;
	struct netfront_queue *queue = NULL;

	if (!xenbus_read_unsigned(np->xbdev->otherend, "feature-rx-copy", 0)) {
		dev_info(&dev->dev,
			 "backend does not support copying receive path\n");
		return -ENODEV;
	}

	err = talk_to_netback(np->xbdev, np);
	if (err)
		return err;
	if (np->netback_has_xdp_headroom)
		pr_info("backend supports XDP headroom\n");

	/* talk_to_netback() sets the correct number of queues */
	num_queues = dev->real_num_tx_queues;

	if (dev->reg_state == NETREG_UNINITIALIZED) {
		err = register_netdev(dev);
		if (err) {
			pr_warn("%s: register_netdev err=%d\n", __func__, err);
			device_unregister(&np->xbdev->dev);
			return err;
		}
	}

	rtnl_lock();
	netdev_update_features(dev);
	rtnl_unlock();

	/*
	 * All public and private state should now be sane.  Get
	 * ready to start sending and receiving packets and give the driver
	 * domain a kick because we've probably just requeued some
	 * packets.
	 */
	netif_tx_lock_bh(np->netdev);
	netif_device_attach(np->netdev);
	netif_tx_unlock_bh(np->netdev);

	netif_carrier_on(np->netdev);
	for (j = 0; j < num_queues; ++j) {
		queue = &np->queues[j];

		notify_remote_via_irq(queue->tx_irq);
		if (queue->tx_irq != queue->rx_irq)
			notify_remote_via_irq(queue->rx_irq);

		spin_lock_irq(&queue->tx_lock);
		xennet_tx_buf_gc(queue);
		spin_unlock_irq(&queue->tx_lock);

		spin_lock_bh(&queue->rx_lock);
		xennet_alloc_rx_buffers(queue);
		spin_unlock_bh(&queue->rx_lock);
	}

	return 0;
}

/*
 * Callback received when the backend's state changes.
 */
static void netback_changed(struct xenbus_device *dev,
			    enum xenbus_state backend_state)
{
	struct netfront_info *np = dev_get_drvdata(&dev->dev);
	struct net_device *netdev = np->netdev;

	dev_dbg(&dev->dev, "%s\n", xenbus_strstate(backend_state));

	wake_up_all(&module_wq);

	switch (backend_state) {
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateUnknown:
		break;

	case XenbusStateInitWait:
		if (dev->state != XenbusStateInitialising)
			break;
		if (xennet_connect(netdev) != 0)
			break;
		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateConnected:
		netdev_notify_peers(netdev);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;	/* Missed the backend's CLOSING state */
	case XenbusStateClosing:
		xenbus_frontend_closed(dev);
		break;
	}
}

static const struct xennet_stat {
	char name[ETH_GSTRING_LEN];
	u16 offset;
} xennet_stats[] = {
	{
		"rx_gso_checksum_fixup",
		offsetof(struct netfront_info, rx_gso_checksum_fixup)
	},
};

static int xennet_get_sset_count(struct net_device *dev, int string_set)
{
	switch (string_set) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(xennet_stats);
	default:
		return -EINVAL;
	}
}

static void xennet_get_ethtool_stats(struct net_device *dev,
				     struct ethtool_stats *stats, u64 * data)
{
	void *np = netdev_priv(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(xennet_stats); i++)
		data[i] = atomic_read((atomic_t *)(np + xennet_stats[i].offset));
}

static void xennet_get_strings(struct net_device *dev, u32 stringset, u8 * data)
{
	int i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < ARRAY_SIZE(xennet_stats); i++)
			memcpy(data + i * ETH_GSTRING_LEN,
			       xennet_stats[i].name, ETH_GSTRING_LEN);
		break;
	}
}

static const struct ethtool_ops xennet_ethtool_ops =
{
	.get_link = ethtool_op_get_link,

	.get_sset_count = xennet_get_sset_count,
	.get_ethtool_stats = xennet_get_ethtool_stats,
	.get_strings = xennet_get_strings,
	.get_ts_info = ethtool_op_get_ts_info,
};

#ifdef CONFIG_SYSFS
static ssize_t show_rxbuf(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", NET_RX_RING_SIZE);
}

static ssize_t store_rxbuf(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t len)
{
	char *endp;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	simple_strtoul(buf, &endp, 0);
	if (endp == buf)
		return -EBADMSG;

	/* rxbuf_min and rxbuf_max are no longer configurable. */

	return len;
}

static DEVICE_ATTR(rxbuf_min, 0644, show_rxbuf, store_rxbuf);
static DEVICE_ATTR(rxbuf_max, 0644, show_rxbuf, store_rxbuf);
static DEVICE_ATTR(rxbuf_cur, 0444, show_rxbuf, NULL);

static struct attribute *xennet_dev_attrs[] = {
	&dev_attr_rxbuf_min.attr,
	&dev_attr_rxbuf_max.attr,
	&dev_attr_rxbuf_cur.attr,
	NULL
};

static const struct attribute_group xennet_dev_group = {
	.attrs = xennet_dev_attrs
};
#endif /* CONFIG_SYSFS */

static void xennet_bus_close(struct xenbus_device *dev)
{
	int ret;

	if (xenbus_read_driver_state(dev->otherend) == XenbusStateClosed)
		return;
	do {
		xenbus_switch_state(dev, XenbusStateClosing);
		ret = wait_event_timeout(module_wq,
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosing ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosed ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateUnknown,
				   XENNET_TIMEOUT);
	} while (!ret);

	if (xenbus_read_driver_state(dev->otherend) == XenbusStateClosed)
		return;

	do {
		xenbus_switch_state(dev, XenbusStateClosed);
		ret = wait_event_timeout(module_wq,
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosed ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateUnknown,
				   XENNET_TIMEOUT);
	} while (!ret);
}

static int xennet_remove(struct xenbus_device *dev)
{
	struct netfront_info *info = dev_get_drvdata(&dev->dev);

	xennet_bus_close(dev);
	xennet_disconnect_backend(info);

	if (info->netdev->reg_state == NETREG_REGISTERED)
		unregister_netdev(info->netdev);

	if (info->queues) {
		rtnl_lock();
		xennet_destroy_queues(info);
		rtnl_unlock();
	}
	xennet_free_netdev(info->netdev);

	return 0;
}

static const struct xenbus_device_id netfront_ids[] = {
	{ "vif" },
	{ "" }
};

static struct xenbus_driver netfront_driver = {
	.ids = netfront_ids,
	.probe = netfront_probe,
	.remove = xennet_remove,
	.resume = netfront_resume,
	.otherend_changed = netback_changed,
};

static int __init netif_init(void)
{
	if (!xen_domain())
		return -ENODEV;

	if (!xen_has_pv_nic_devices())
		return -ENODEV;

	pr_info("Initialising Xen virtual ethernet driver\n");

	/* Allow as many queues as there are CPUs inut max. 8 if user has not
	 * specified a value.
	 */
	if (xennet_max_queues == 0)
		xennet_max_queues = min_t(unsigned int, MAX_QUEUES_DEFAULT,
					  num_online_cpus());

	return xenbus_register_frontend(&netfront_driver);
}
module_init(netif_init);


static void __exit netif_exit(void)
{
	xenbus_unregister_driver(&netfront_driver);
}
module_exit(netif_exit);

MODULE_DESCRIPTION("Xen virtual network device frontend");
MODULE_LICENSE("GPL");
MODULE_ALIAS("xen:vif");
MODULE_ALIAS("xennet");
	if (!txs) {
		err = -ENOMEM;
		xenbus_dev_fatal(dev, err, "allocating tx ring page");
		goto fail;
	}
	SHARED_RING_INIT(txs);
	FRONT_RING_INIT(&queue->tx, txs, XEN_PAGE_SIZE);

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
	/* setup single event channel if
	 *  a) feature-split-event-channels == 0
	 *  b) feature-split-event-channels == 1 but failed to setup
	 */
	if (!feature_split_evtchn || err)
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
	queue->tx_link[NET_TX_RING_SIZE - 1] = TX_LINK_NONE;

	/* Clear out rx_skbs */
	for (i = 0; i < NET_RX_RING_SIZE; i++) {
		queue->rx_skbs[i] = NULL;
		queue->grant_rx_ref[i] = GRANT_INVALID_REF;
	}

	/* A grant for every tx ring slot */
	if (gnttab_alloc_grant_references(NET_TX_RING_SIZE,
					  &queue->gref_tx_head) < 0) {
		pr_alert("can't alloc tx grant refs\n");
		err = -ENOMEM;
		goto exit;
	}

	/* A grant for every rx ring slot */
	if (gnttab_alloc_grant_references(NET_RX_RING_SIZE,
					  &queue->gref_rx_head) < 0) {
		pr_alert("can't alloc rx grant refs\n");
		err = -ENOMEM;
		goto exit_free_tx;
	}

	return 0;

 exit_free_tx:
	gnttab_free_grant_references(queue->gref_tx_head);
 exit:
	return err;
}

static int write_queue_xenstore_keys(struct netfront_queue *queue,
			   struct xenbus_transaction *xbt, int write_hierarchical)
{
	/* Write the queue-specific keys into XenStore in the traditional
	 * way for a single queue, or in a queue subkeys for multiple
	 * queues.
	 */
	struct xenbus_device *dev = queue->info->xbdev;
	int err;
	const char *message;
	char *path;
	size_t pathsize;

	/* Choose the correct place to write the keys */
	if (write_hierarchical) {
		pathsize = strlen(dev->nodename) + 10;
		path = kzalloc(pathsize, GFP_KERNEL);
		if (!path) {
			err = -ENOMEM;
			message = "out of memory while writing ring references";
			goto error;
		}
		snprintf(path, pathsize, "%s/queue-%u",
				dev->nodename, queue->id);
	} else {
		path = (char *)dev->nodename;
	}

	/* Write ring references */
	err = xenbus_printf(*xbt, path, "tx-ring-ref", "%u",
			queue->tx_ring_ref);
	if (err) {
		message = "writing tx-ring-ref";
		goto error;
	}

	err = xenbus_printf(*xbt, path, "rx-ring-ref", "%u",
			queue->rx_ring_ref);
	if (err) {
		message = "writing rx-ring-ref";
		goto error;
	}

	/* Write event channels; taking into account both shared
	 * and split event channel scenarios.
	 */
	if (queue->tx_evtchn == queue->rx_evtchn) {
		/* Shared event channel */
		err = xenbus_printf(*xbt, path,
				"event-channel", "%u", queue->tx_evtchn);
		if (err) {
			message = "writing event-channel";
			goto error;
		}
	} else {
		/* Split event channels */
		err = xenbus_printf(*xbt, path,
				"event-channel-tx", "%u", queue->tx_evtchn);
		if (err) {
			message = "writing event-channel-tx";
			goto error;
		}

		err = xenbus_printf(*xbt, path,
				"event-channel-rx", "%u", queue->rx_evtchn);
		if (err) {
			message = "writing event-channel-rx";
			goto error;
		}
	}

	if (write_hierarchical)
		kfree(path);
	return 0;

error:
	if (write_hierarchical)
		kfree(path);
	xenbus_dev_fatal(dev, err, "%s", message);
	return err;
}



static int xennet_create_page_pool(struct netfront_queue *queue)
{
	int err;
	struct page_pool_params pp_params = {
		.order = 0,
		.flags = 0,
		.pool_size = NET_RX_RING_SIZE,
		.nid = NUMA_NO_NODE,
		.dev = &queue->info->netdev->dev,
		.offset = XDP_PACKET_HEADROOM,
		.max_len = XEN_PAGE_SIZE - XDP_PACKET_HEADROOM,
	};

	queue->page_pool = page_pool_create(&pp_params);
	if (IS_ERR(queue->page_pool)) {
		err = PTR_ERR(queue->page_pool);
		queue->page_pool = NULL;
		return err;
	}

	err = xdp_rxq_info_reg(&queue->xdp_rxq, queue->info->netdev,
			       queue->id, 0);
	if (err) {
		netdev_err(queue->info->netdev, "xdp_rxq_info_reg failed\n");
		goto err_free_pp;
	}

	err = xdp_rxq_info_reg_mem_model(&queue->xdp_rxq,
					 MEM_TYPE_PAGE_POOL, queue->page_pool);
	if (err) {
		netdev_err(queue->info->netdev, "xdp_rxq_info_reg_mem_model failed\n");
		goto err_unregister_rxq;
	}
	return 0;

err_unregister_rxq:
	xdp_rxq_info_unreg(&queue->xdp_rxq);
err_free_pp:
	page_pool_destroy(queue->page_pool);
	queue->page_pool = NULL;
	return err;
}

static int xennet_create_queues(struct netfront_info *info,
				unsigned int *num_queues)
{
	unsigned int i;
	int ret;

	info->queues = kcalloc(*num_queues, sizeof(struct netfront_queue),
			       GFP_KERNEL);
	if (!info->queues)
		return -ENOMEM;

	for (i = 0; i < *num_queues; i++) {
		struct netfront_queue *queue = &info->queues[i];

		queue->id = i;
		queue->info = info;

		ret = xennet_init_queue(queue);
		if (ret < 0) {
			dev_warn(&info->xbdev->dev,
				 "only created %d queues\n", i);
			*num_queues = i;
			break;
		}

		/* use page pool recycling instead of buddy allocator */
		ret = xennet_create_page_pool(queue);
		if (ret < 0) {
			dev_err(&info->xbdev->dev, "can't allocate page pool\n");
			*num_queues = i;
			return ret;
		}

		netif_napi_add(queue->info->netdev, &queue->napi,
			       xennet_poll, 64);
		if (netif_running(info->netdev))
			napi_enable(&queue->napi);
	}

	netif_set_real_num_tx_queues(info->netdev, *num_queues);

	if (*num_queues == 0) {
		dev_err(&info->xbdev->dev, "no queues\n");
		return -EINVAL;
	}
	return 0;
}

/* Common code used when first setting up, and when resuming. */
static int talk_to_netback(struct xenbus_device *dev,
			   struct netfront_info *info)
{
	const char *message;
	struct xenbus_transaction xbt;
	int err;
	unsigned int feature_split_evtchn;
	unsigned int i = 0;
	unsigned int max_queues = 0;
	struct netfront_queue *queue = NULL;
	unsigned int num_queues = 1;
	u8 addr[ETH_ALEN];

	info->netdev->irq = 0;

	/* Check if backend supports multiple queues */
	max_queues = xenbus_read_unsigned(info->xbdev->otherend,
					  "multi-queue-max-queues", 1);
	num_queues = min(max_queues, xennet_max_queues);

	/* Check feature-split-event-channels */
	feature_split_evtchn = xenbus_read_unsigned(info->xbdev->otherend,
					"feature-split-event-channels", 0);

	/* Read mac addr. */
	err = xen_net_read_mac(dev, addr);
	if (err) {
		xenbus_dev_fatal(dev, err, "parsing %s/mac", dev->nodename);
		goto out_unlocked;
	}
	eth_hw_addr_set(info->netdev, addr);

	info->netback_has_xdp_headroom = xenbus_read_unsigned(info->xbdev->otherend,
							      "feature-xdp-headroom", 0);
	if (info->netback_has_xdp_headroom) {
		/* set the current xen-netfront xdp state */
		err = talk_to_netback_xdp(info, info->netfront_xdp_enabled ?
					  NETBACK_XDP_HEADROOM_ENABLE :
					  NETBACK_XDP_HEADROOM_DISABLE);
		if (err)
			goto out_unlocked;
	}

	rtnl_lock();
	if (info->queues)
		xennet_destroy_queues(info);

	/* For the case of a reconnect reset the "broken" indicator. */
	info->broken = false;

	err = xennet_create_queues(info, &num_queues);
	if (err < 0) {
		xenbus_dev_fatal(dev, err, "creating queues");
		kfree(info->queues);
		info->queues = NULL;
		goto out;
	}
	rtnl_unlock();

	/* Create shared ring, alloc event channel -- for each queue */
	for (i = 0; i < num_queues; ++i) {
		queue = &info->queues[i];
		err = setup_netfront(dev, queue, feature_split_evtchn);
		if (err)
			goto destroy_ring;
	}

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		xenbus_dev_fatal(dev, err, "starting transaction");
		goto destroy_ring;
	}

	if (xenbus_exists(XBT_NIL,
			  info->xbdev->otherend, "multi-queue-max-queues")) {
		/* Write the number of queues */
		err = xenbus_printf(xbt, dev->nodename,
				    "multi-queue-num-queues", "%u", num_queues);
		if (err) {
			message = "writing multi-queue-num-queues";
			goto abort_transaction_no_dev_fatal;
		}
	}

	if (num_queues == 1) {
		err = write_queue_xenstore_keys(&info->queues[0], &xbt, 0); /* flat */
		if (err)
			goto abort_transaction_no_dev_fatal;
	} else {
		/* Write the keys for each queue */
		for (i = 0; i < num_queues; ++i) {
			queue = &info->queues[i];
			err = write_queue_xenstore_keys(queue, &xbt, 1); /* hierarchical */
			if (err)
				goto abort_transaction_no_dev_fatal;
		}
	}

	/* The remaining keys are not queue-specific */
	err = xenbus_printf(xbt, dev->nodename, "request-rx-copy", "%u",
			    1);
	if (err) {
		message = "writing request-rx-copy";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-rx-notify", "%d", 1);
	if (err) {
		message = "writing feature-rx-notify";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-sg", "%d", 1);
	if (err) {
		message = "writing feature-sg";
		goto abort_transaction;
	}

	err = xenbus_printf(xbt, dev->nodename, "feature-gso-tcpv4", "%d", 1);
	if (err) {
		message = "writing feature-gso-tcpv4";
		goto abort_transaction;
	}

	err = xenbus_write(xbt, dev->nodename, "feature-gso-tcpv6", "1");
	if (err) {
		message = "writing feature-gso-tcpv6";
		goto abort_transaction;
	}

	err = xenbus_write(xbt, dev->nodename, "feature-ipv6-csum-offload",
			   "1");
	if (err) {
		message = "writing feature-ipv6-csum-offload";
		goto abort_transaction;
	}

	err = xenbus_transaction_end(xbt, 0);
	if (err) {
		if (err == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, err, "completing transaction");
		goto destroy_ring;
	}

	return 0;

 abort_transaction:
	xenbus_dev_fatal(dev, err, "%s", message);
abort_transaction_no_dev_fatal:
	xenbus_transaction_end(xbt, 1);
 destroy_ring:
	xennet_disconnect_backend(info);
	rtnl_lock();
	xennet_destroy_queues(info);
 out:
	rtnl_unlock();
out_unlocked:
	device_unregister(&dev->dev);
	return err;
}

static int xennet_connect(struct net_device *dev)
{
	struct netfront_info *np = netdev_priv(dev);
	unsigned int num_queues = 0;
	int err;
	unsigned int j = 0;
	struct netfront_queue *queue = NULL;

	if (!xenbus_read_unsigned(np->xbdev->otherend, "feature-rx-copy", 0)) {
		dev_info(&dev->dev,
			 "backend does not support copying receive path\n");
		return -ENODEV;
	}

	err = talk_to_netback(np->xbdev, np);
	if (err)
		return err;
	if (np->netback_has_xdp_headroom)
		pr_info("backend supports XDP headroom\n");

	/* talk_to_netback() sets the correct number of queues */
	num_queues = dev->real_num_tx_queues;

	if (dev->reg_state == NETREG_UNINITIALIZED) {
		err = register_netdev(dev);
		if (err) {
			pr_warn("%s: register_netdev err=%d\n", __func__, err);
			device_unregister(&np->xbdev->dev);
			return err;
		}
	}

	rtnl_lock();
	netdev_update_features(dev);
	rtnl_unlock();

	/*
	 * All public and private state should now be sane.  Get
	 * ready to start sending and receiving packets and give the driver
	 * domain a kick because we've probably just requeued some
	 * packets.
	 */
	netif_tx_lock_bh(np->netdev);
	netif_device_attach(np->netdev);
	netif_tx_unlock_bh(np->netdev);

	netif_carrier_on(np->netdev);
	for (j = 0; j < num_queues; ++j) {
		queue = &np->queues[j];

		notify_remote_via_irq(queue->tx_irq);
		if (queue->tx_irq != queue->rx_irq)
			notify_remote_via_irq(queue->rx_irq);

		spin_lock_irq(&queue->tx_lock);
		xennet_tx_buf_gc(queue);
		spin_unlock_irq(&queue->tx_lock);

		spin_lock_bh(&queue->rx_lock);
		xennet_alloc_rx_buffers(queue);
		spin_unlock_bh(&queue->rx_lock);
	}

	return 0;
}

/*
 * Callback received when the backend's state changes.
 */
static void netback_changed(struct xenbus_device *dev,
			    enum xenbus_state backend_state)
{
	struct netfront_info *np = dev_get_drvdata(&dev->dev);
	struct net_device *netdev = np->netdev;

	dev_dbg(&dev->dev, "%s\n", xenbus_strstate(backend_state));

	wake_up_all(&module_wq);

	switch (backend_state) {
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateUnknown:
		break;

	case XenbusStateInitWait:
		if (dev->state != XenbusStateInitialising)
			break;
		if (xennet_connect(netdev) != 0)
			break;
		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateConnected:
		netdev_notify_peers(netdev);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;	/* Missed the backend's CLOSING state */
	case XenbusStateClosing:
		xenbus_frontend_closed(dev);
		break;
	}
}

static const struct xennet_stat {
	char name[ETH_GSTRING_LEN];
	u16 offset;
} xennet_stats[] = {
	{
		"rx_gso_checksum_fixup",
		offsetof(struct netfront_info, rx_gso_checksum_fixup)
	},
};

static int xennet_get_sset_count(struct net_device *dev, int string_set)
{
	switch (string_set) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(xennet_stats);
	default:
		return -EINVAL;
	}
}

static void xennet_get_ethtool_stats(struct net_device *dev,
				     struct ethtool_stats *stats, u64 * data)
{
	void *np = netdev_priv(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(xennet_stats); i++)
		data[i] = atomic_read((atomic_t *)(np + xennet_stats[i].offset));
}

static void xennet_get_strings(struct net_device *dev, u32 stringset, u8 * data)
{
	int i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < ARRAY_SIZE(xennet_stats); i++)
			memcpy(data + i * ETH_GSTRING_LEN,
			       xennet_stats[i].name, ETH_GSTRING_LEN);
		break;
	}
}

static const struct ethtool_ops xennet_ethtool_ops =
{
	.get_link = ethtool_op_get_link,

	.get_sset_count = xennet_get_sset_count,
	.get_ethtool_stats = xennet_get_ethtool_stats,
	.get_strings = xennet_get_strings,
	.get_ts_info = ethtool_op_get_ts_info,
};

#ifdef CONFIG_SYSFS
static ssize_t show_rxbuf(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", NET_RX_RING_SIZE);
}

static ssize_t store_rxbuf(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t len)
{
	char *endp;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	simple_strtoul(buf, &endp, 0);
	if (endp == buf)
		return -EBADMSG;

	/* rxbuf_min and rxbuf_max are no longer configurable. */

	return len;
}

static DEVICE_ATTR(rxbuf_min, 0644, show_rxbuf, store_rxbuf);
static DEVICE_ATTR(rxbuf_max, 0644, show_rxbuf, store_rxbuf);
static DEVICE_ATTR(rxbuf_cur, 0444, show_rxbuf, NULL);

static struct attribute *xennet_dev_attrs[] = {
	&dev_attr_rxbuf_min.attr,
	&dev_attr_rxbuf_max.attr,
	&dev_attr_rxbuf_cur.attr,
	NULL
};

static const struct attribute_group xennet_dev_group = {
	.attrs = xennet_dev_attrs
};
#endif /* CONFIG_SYSFS */

static void xennet_bus_close(struct xenbus_device *dev)
{
	int ret;

	if (xenbus_read_driver_state(dev->otherend) == XenbusStateClosed)
		return;
	do {
		xenbus_switch_state(dev, XenbusStateClosing);
		ret = wait_event_timeout(module_wq,
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosing ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosed ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateUnknown,
				   XENNET_TIMEOUT);
	} while (!ret);

	if (xenbus_read_driver_state(dev->otherend) == XenbusStateClosed)
		return;

	do {
		xenbus_switch_state(dev, XenbusStateClosed);
		ret = wait_event_timeout(module_wq,
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateClosed ||
				   xenbus_read_driver_state(dev->otherend) ==
				   XenbusStateUnknown,
				   XENNET_TIMEOUT);
	} while (!ret);
}

static int xennet_remove(struct xenbus_device *dev)
{
	struct netfront_info *info = dev_get_drvdata(&dev->dev);

	xennet_bus_close(dev);
	xennet_disconnect_backend(info);

	if (info->netdev->reg_state == NETREG_REGISTERED)
		unregister_netdev(info->netdev);

	if (info->queues) {
		rtnl_lock();
		xennet_destroy_queues(info);
		rtnl_unlock();
	}
	xennet_free_netdev(info->netdev);

	return 0;
}

static const struct xenbus_device_id netfront_ids[] = {
	{ "vif" },
	{ "" }
};

static struct xenbus_driver netfront_driver = {
	.ids = netfront_ids,
	.probe = netfront_probe,
	.remove = xennet_remove,
	.resume = netfront_resume,
	.otherend_changed = netback_changed,
};

static int __init netif_init(void)
{
	if (!xen_domain())
		return -ENODEV;

	if (!xen_has_pv_nic_devices())
		return -ENODEV;

	pr_info("Initialising Xen virtual ethernet driver\n");

	/* Allow as many queues as there are CPUs inut max. 8 if user has not
	 * specified a value.
	 */
	if (xennet_max_queues == 0)
		xennet_max_queues = min_t(unsigned int, MAX_QUEUES_DEFAULT,
					  num_online_cpus());

	return xenbus_register_frontend(&netfront_driver);
}
module_init(netif_init);


static void __exit netif_exit(void)
{
	xenbus_unregister_driver(&netfront_driver);
}
module_exit(netif_exit);

MODULE_DESCRIPTION("Xen virtual network device frontend");
MODULE_LICENSE("GPL");
MODULE_ALIAS("xen:vif");
MODULE_ALIAS("xennet");