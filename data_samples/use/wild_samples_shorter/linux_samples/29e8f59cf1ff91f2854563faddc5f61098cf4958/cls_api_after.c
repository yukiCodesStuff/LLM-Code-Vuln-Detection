	int err;
	int tp_created = 0;

	if ((n->nlmsg_type != RTM_GETTFILTER) && !netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

replay:
	err = nlmsg_parse(n, sizeof(*t), tca, TCA_MAX, NULL);