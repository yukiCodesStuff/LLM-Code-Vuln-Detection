{
	struct net *net = sock_net(skb->sk);
	struct nlattr *tb[IFA_MAX+1];
	struct dn_dev *dn_db;
	struct ifaddrmsg *ifm;
	struct dn_ifaddr *ifa;
	struct dn_ifaddr __rcu **ifap;
	int err = -EINVAL;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		goto errout;

	err = nlmsg_parse(nlh, sizeof(*ifm), tb, IFA_MAX, dn_ifa_policy);
	if (err < 0)
		goto errout;

	err = -ENODEV;
	ifm = nlmsg_data(nlh);
	if ((dn_db = dn_dev_by_index(ifm->ifa_index)) == NULL)
		goto errout;

	err = -EADDRNOTAVAIL;
	for (ifap = &dn_db->ifa_list;
	     (ifa = rtnl_dereference(*ifap)) != NULL;
	     ifap = &ifa->ifa_next) {
		if (tb[IFA_LOCAL] &&
		    nla_memcmp(tb[IFA_LOCAL], &ifa->ifa_local, 2))
			continue;

		if (tb[IFA_LABEL] && nla_strcmp(tb[IFA_LABEL], ifa->ifa_label))
			continue;

		dn_dev_del_ifa(dn_db, ifap, 1);
		return 0;
	}
{
	struct net *net = sock_net(skb->sk);
	struct nlattr *tb[IFA_MAX+1];
	struct net_device *dev;
	struct dn_dev *dn_db;
	struct ifaddrmsg *ifm;
	struct dn_ifaddr *ifa;
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;

	err = nlmsg_parse(nlh, sizeof(*ifm), tb, IFA_MAX, dn_ifa_policy);
	if (err < 0)
		return err;

	if (tb[IFA_LOCAL] == NULL)
		return -EINVAL;

	ifm = nlmsg_data(nlh);
	if ((dev = __dev_get_by_index(&init_net, ifm->ifa_index)) == NULL)
		return -ENODEV;

	if ((dn_db = rtnl_dereference(dev->dn_ptr)) == NULL) {
		dn_db = dn_dev_create(dev, &err);
		if (!dn_db)
			return err;
	}