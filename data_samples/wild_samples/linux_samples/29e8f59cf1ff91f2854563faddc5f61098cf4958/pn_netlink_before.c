{
	struct net *net = sock_net(skb->sk);
	struct nlattr *tb[IFA_MAX+1];
	struct net_device *dev;
	struct ifaddrmsg *ifm;
	int err;
	u8 pnaddr;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	ASSERT_RTNL();

	err = nlmsg_parse(nlh, sizeof(*ifm), tb, IFA_MAX, ifa_phonet_policy);
	if (err < 0)
		return err;

	ifm = nlmsg_data(nlh);
	if (tb[IFA_LOCAL] == NULL)
		return -EINVAL;
	pnaddr = nla_get_u8(tb[IFA_LOCAL]);
	if (pnaddr & 3)
		/* Phonet addresses only have 6 high-order bits */
		return -EINVAL;

	dev = __dev_get_by_index(net, ifm->ifa_index);
	if (dev == NULL)
		return -ENODEV;

	if (nlh->nlmsg_type == RTM_NEWADDR)
		err = phonet_address_add(dev, pnaddr);
	else
		err = phonet_address_del(dev, pnaddr);
	if (!err)
		phonet_address_notify(nlh->nlmsg_type, dev, pnaddr);
	return err;
}

static int fill_addr(struct sk_buff *skb, struct net_device *dev, u8 addr,
			u32 portid, u32 seq, int event)
{
	struct ifaddrmsg *ifm;
	struct nlmsghdr *nlh;

	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*ifm), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	ifm = nlmsg_data(nlh);
	ifm->ifa_family = AF_PHONET;
	ifm->ifa_prefixlen = 0;
	ifm->ifa_flags = IFA_F_PERMANENT;
	ifm->ifa_scope = RT_SCOPE_LINK;
	ifm->ifa_index = dev->ifindex;
	if (nla_put_u8(skb, IFA_LOCAL, addr))
		goto nla_put_failure;
	return nlmsg_end(skb, nlh);

nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}

static int getaddr_dumpit(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct phonet_device_list *pndevs;
	struct phonet_device *pnd;
	int dev_idx = 0, dev_start_idx = cb->args[0];
	int addr_idx = 0, addr_start_idx = cb->args[1];

	pndevs = phonet_device_list(sock_net(skb->sk));
	rcu_read_lock();
	list_for_each_entry_rcu(pnd, &pndevs->list, list) {
		u8 addr;

		if (dev_idx > dev_start_idx)
			addr_start_idx = 0;
		if (dev_idx++ < dev_start_idx)
			continue;

		addr_idx = 0;
		for_each_set_bit(addr, pnd->addrs, 64) {
			if (addr_idx++ < addr_start_idx)
				continue;

			if (fill_addr(skb, pnd->netdev, addr << 2,
					 NETLINK_CB(cb->skb).portid,
					cb->nlh->nlmsg_seq, RTM_NEWADDR) < 0)
				goto out;
		}
	}
{
	struct net *net = sock_net(skb->sk);
	struct nlattr *tb[RTA_MAX+1];
	struct net_device *dev;
	struct rtmsg *rtm;
	int err;
	u8 dst;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	ASSERT_RTNL();

	err = nlmsg_parse(nlh, sizeof(*rtm), tb, RTA_MAX, rtm_phonet_policy);
	if (err < 0)
		return err;

	rtm = nlmsg_data(nlh);
	if (rtm->rtm_table != RT_TABLE_MAIN || rtm->rtm_type != RTN_UNICAST)
		return -EINVAL;
	if (tb[RTA_DST] == NULL || tb[RTA_OIF] == NULL)
		return -EINVAL;
	dst = nla_get_u8(tb[RTA_DST]);
	if (dst & 3) /* Phonet addresses only have 6 high-order bits */
		return -EINVAL;

	dev = __dev_get_by_index(net, nla_get_u32(tb[RTA_OIF]));
	if (dev == NULL)
		return -ENODEV;

	if (nlh->nlmsg_type == RTM_NEWROUTE)
		err = phonet_route_add(dev, dst);
	else
		err = phonet_route_del(dev, dst);
	if (!err)
		rtm_phonet_notify(nlh->nlmsg_type, dev, dst);
	return err;
}

static int route_dumpit(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct net *net = sock_net(skb->sk);
	u8 addr, addr_idx = 0, addr_start_idx = cb->args[0];

	rcu_read_lock();
	for (addr = 0; addr < 64; addr++) {
		struct net_device *dev;

		dev = phonet_route_get_rcu(net, addr << 2);
		if (!dev)
			continue;

		if (addr_idx++ < addr_start_idx)
			continue;
		if (fill_route(skb, dev, addr << 2, NETLINK_CB(cb->skb).portid,
				cb->nlh->nlmsg_seq, RTM_NEWROUTE))
			goto out;
	}