	struct nlattr *attrs[RTA_MAX+1];
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;
	struct nlattr *attrs[RTA_MAX+1];
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;