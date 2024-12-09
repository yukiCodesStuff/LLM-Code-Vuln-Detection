	u8 limhops = 0;
	int err = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;
	u8 limhops = 0;
	int err = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (nlmsg_len(nlh) < sizeof(*r))
		return -EINVAL;