	struct dn_ifaddr __rcu **ifap;
	int err = -EINVAL;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		goto errout;
	struct dn_ifaddr *ifa;
	int err;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;