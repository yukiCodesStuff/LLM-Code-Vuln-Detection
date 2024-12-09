	if (!nskb) {
		gwj->dropped_frames++;
		return;
	}

	/* put the incremented hop counter in the cloned skb */
	cgw_hops(nskb) = cgw_hops(skb) + 1;

	/* first processing of this CAN frame -> adjust to private hop limit */
	if (gwj->limit_hops && cgw_hops(nskb) == 1)
		cgw_hops(nskb) = max_hops - gwj->limit_hops + 1;

	nskb->dev = gwj->dst.dev;

	/* pointer to modifiable CAN frame */
	cf = (struct can_frame *)nskb->data;

	/* perform preprocessed modification functions if there are any */
	while (modidx < MAX_MODFUNCTIONS && gwj->mod.modfunc[modidx])
		(*gwj->mod.modfunc[modidx++])(cf, &gwj->mod);

	/* Has the CAN frame been modified? */
	if (modidx) {
		/* get available space for the processed CAN frame type */
		int max_len = nskb->len - offsetof(struct can_frame, data);

		/* dlc may have changed, make sure it fits to the CAN frame */
		if (cf->can_dlc > max_len)
			goto out_delete;

		/* check for checksum updates in classic CAN length only */
		if (gwj->mod.csumfunc.crc8) {
			if (cf->can_dlc > 8)
				goto out_delete;

			(*gwj->mod.csumfunc.crc8)(cf, &gwj->mod.csum.crc8);
		}

		if (gwj->mod.csumfunc.xor) {
			if (cf->can_dlc > 8)
				goto out_delete;

			(*gwj->mod.csumfunc.xor)(cf, &gwj->mod.csum.xor);
		}
	}
		if (gwj->mod.csumfunc.xor) {
			if (cf->can_dlc > 8)
				goto out_delete;

			(*gwj->mod.csumfunc.xor)(cf, &gwj->mod.csum.xor);
		}
	}

	/* clear the skb timestamp if not configured the other way */
	if (!(gwj->flags & CGW_FLAGS_CAN_SRC_TSTAMP))
		nskb->tstamp = 0;

	/* send to netdevice */
	if (can_send(nskb, gwj->flags & CGW_FLAGS_CAN_ECHO))
		gwj->dropped_frames++;
	else
		gwj->handled_frames++;

	return;

 out_delete:
	/* delete frame due to misconfiguration */
	gwj->deleted_frames++;
	kfree_skb(nskb);
	return;
}

static inline int cgw_register_filter(struct net *net, struct cgw_job *gwj)
{
	return can_rx_register(net, gwj->src.dev, gwj->ccgw.filter.can_id,
			       gwj->ccgw.filter.can_mask, can_can_gw_rcv,
			       gwj, "gw", NULL);
}

static inline void cgw_unregister_filter(struct net *net, struct cgw_job *gwj)
{
	can_rx_unregister(net, gwj->src.dev, gwj->ccgw.filter.can_id,
			  gwj->ccgw.filter.can_mask, can_can_gw_rcv, gwj);
}

static int cgw_notifier(struct notifier_block *nb,
			unsigned long msg, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);

	if (dev->type != ARPHRD_CAN)
		return NOTIFY_DONE;

	if (msg == NETDEV_UNREGISTER) {

		struct cgw_job *gwj = NULL;
		struct hlist_node *nx;

		ASSERT_RTNL();

		hlist_for_each_entry_safe(gwj, nx, &net->can.cgw_list, list) {

			if (gwj->src.dev == dev || gwj->dst.dev == dev) {
				hlist_del(&gwj->list);
				cgw_unregister_filter(net, gwj);
				kmem_cache_free(cgw_cache, gwj);
			}
		}
	}