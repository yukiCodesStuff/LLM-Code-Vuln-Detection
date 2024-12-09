	dev = dev_get_by_name(net, driver_name);
	if (!dev)
		return -ENODEV;

	/* Associate TIPC bearer with L2 bearer */
	rcu_assign_pointer(b->media_ptr, dev);
	memset(&b->bcast_addr, 0, sizeof(b->bcast_addr));
	if (!b)
		return NOTIFY_DONE;

	b->mtu = dev->mtu;

	switch (evt) {
	case NETDEV_CHANGE:
		if (netif_carrier_ok(dev))
			break;
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEMTU:
		tipc_reset_bearer(net, b);
		break;
	case NETDEV_CHANGEADDR:
		b->media->raw2addr(b, &b->addr,