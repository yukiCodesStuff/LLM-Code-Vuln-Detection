	u8 limhops = 0;
	int err = 0;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;
	u8 limhops = 0;
	int err = 0;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;