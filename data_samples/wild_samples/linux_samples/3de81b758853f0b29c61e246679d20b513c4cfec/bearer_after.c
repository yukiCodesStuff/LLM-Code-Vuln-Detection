{
	struct net_device *dev;
	char *driver_name = strchr((const char *)b->name, ':') + 1;

	/* Find device with specified name */
	dev = dev_get_by_name(net, driver_name);
	if (!dev)
		return -ENODEV;
	if (tipc_mtu_bad(dev, 0)) {
		dev_put(dev);
		return -EINVAL;
	}
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);
	struct tipc_bearer *b;

	b = rtnl_dereference(dev->tipc_ptr);
	if (!b)
		return NOTIFY_DONE;

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
		if (tipc_mtu_bad(dev, 0)) {
			bearer_disable(net, b);
			break;
		}
		b->mtu = dev->mtu;
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
		if (tipc_mtu_bad(dev, 0)) {
			bearer_disable(net, b);
			break;
		}