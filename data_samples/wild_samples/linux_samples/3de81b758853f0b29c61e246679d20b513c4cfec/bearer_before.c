{
	struct net_device *dev;
	char *driver_name = strchr((const char *)b->name, ':') + 1;

	/* Find device with specified name */
	dev = dev_get_by_name(net, driver_name);
	if (!dev)
		return -ENODEV;

	/* Associate TIPC bearer with L2 bearer */
	rcu_assign_pointer(b->media_ptr, dev);
	memset(&b->bcast_addr, 0, sizeof(b->bcast_addr));
	memcpy(b->bcast_addr.value, dev->broadcast, b->media->hwaddr_len);
	b->bcast_addr.media_id = b->media->type_id;
	b->bcast_addr.broadcast = 1;
	b->mtu = dev->mtu;
	b->media->raw2addr(b, &b->addr, (char *)dev->dev_addr);
	rcu_assign_pointer(dev->tipc_ptr, b);
	return 0;
}

/* tipc_disable_l2_media - detach TIPC bearer from an L2 interface
 *
 * Mark L2 bearer as inactive so that incoming buffers are thrown away
 */
void tipc_disable_l2_media(struct tipc_bearer *b)
{
	struct net_device *dev;

	dev = (struct net_device *)rtnl_dereference(b->media_ptr);
	RCU_INIT_POINTER(dev->tipc_ptr, NULL);
	synchronize_net();
	dev_put(dev);
}

/**
 * tipc_l2_send_msg - send a TIPC packet out over an L2 interface
 * @skb: the packet to be sent
 * @b: the bearer through which the packet is to be sent
 * @dest: peer destination address
 */
int tipc_l2_send_msg(struct net *net, struct sk_buff *skb,
		     struct tipc_bearer *b, struct tipc_media_addr *dest)
{
	struct net_device *dev;
	int delta;

	dev = (struct net_device *)rcu_dereference_rtnl(b->media_ptr);
	if (!dev)
		return 0;

	delta = SKB_DATA_ALIGN(dev->hard_header_len - skb_headroom(skb));
	if ((delta > 0) && pskb_expand_head(skb, delta, 0, GFP_ATOMIC)) {
		kfree_skb(skb);
		return 0;
	}
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);
	struct tipc_bearer *b;

	b = rtnl_dereference(dev->tipc_ptr);
	if (!b)
		return NOTIFY_DONE;

	b->mtu = dev->mtu;

	switch (evt) {
	case NETDEV_CHANGE:
		if (netif_carrier_ok(dev))
			break;
	case NETDEV_UP:
		test_and_set_bit_lock(0, &b->up);
		break;
	case NETDEV_GOING_DOWN:
		clear_bit_unlock(0, &b->up);
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEMTU:
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEADDR:
		b->media->raw2addr(b, &b->addr,
				   (char *)dev->dev_addr);
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_UNREGISTER:
	case NETDEV_CHANGENAME:
		bearer_disable(dev_net(dev), b);
		break;
	}
	switch (evt) {
	case NETDEV_CHANGE:
		if (netif_carrier_ok(dev))
			break;
	case NETDEV_UP:
		test_and_set_bit_lock(0, &b->up);
		break;
	case NETDEV_GOING_DOWN:
		clear_bit_unlock(0, &b->up);
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEMTU:
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEADDR:
		b->media->raw2addr(b, &b->addr,
				   (char *)dev->dev_addr);
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_UNREGISTER:
	case NETDEV_CHANGENAME:
		bearer_disable(dev_net(dev), b);
		break;
	}
	return NOTIFY_OK;
}

static struct packet_type tipc_packet_type __read_mostly = {
	.type = htons(ETH_P_TIPC),
	.func = tipc_l2_rcv_msg,
};

static struct notifier_block notifier = {
	.notifier_call  = tipc_l2_device_event,
	.priority	= 0,
};

int tipc_bearer_setup(void)
{
	int err;

	err = register_netdevice_notifier(&notifier);
	if (err)
		return err;
	dev_add_pack(&tipc_packet_type);
	return 0;
}