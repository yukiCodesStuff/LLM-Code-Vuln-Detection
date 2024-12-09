	int err;
	u8 pnaddr;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	ASSERT_RTNL();

	int err;
	u8 dst;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	ASSERT_RTNL();
