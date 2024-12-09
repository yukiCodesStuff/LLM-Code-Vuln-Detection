{
	struct net *net = sock_net(skb->sk);
	struct dn_fib_table *tb;
	struct rtmsg *r = nlmsg_data(nlh);
	struct nlattr *attrs[RTA_MAX+1];
	int err;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;

	err = nlmsg_parse(nlh, sizeof(*r), attrs, RTA_MAX, rtm_dn_policy);
	if (err < 0)
		return err;

	tb = dn_fib_get_table(rtm_get_table(attrs, r->rtm_table), 0);
	if (!tb)
		return -ESRCH;

	return tb->delete(tb, r, attrs, nlh, &NETLINK_CB(skb));
}

static int dn_fib_rtm_newroute(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	struct net *net = sock_net(skb->sk);
	struct dn_fib_table *tb;
	struct rtmsg *r = nlmsg_data(nlh);
	struct nlattr *attrs[RTA_MAX+1];
	int err;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;

	err = nlmsg_parse(nlh, sizeof(*r), attrs, RTA_MAX, rtm_dn_policy);
	if (err < 0)
		return err;

	tb = dn_fib_get_table(rtm_get_table(attrs, r->rtm_table), 1);
	if (!tb)
		return -ENOBUFS;

	return tb->insert(tb, r, attrs, nlh, &NETLINK_CB(skb));
}

static void fib_magic(int cmd, int type, __le16 dst, int dst_len, struct dn_ifaddr *ifa)
{
	struct dn_fib_table *tb;
	struct {
		struct nlmsghdr nlh;
		struct rtmsg rtm;
	} req;
	struct {
		struct nlattr hdr;
		__le16 dst;
	} dst_attr = {
		.dst = dst,
	};
	struct {
		struct nlattr hdr;
		__le16 prefsrc;
	} prefsrc_attr = {
		.prefsrc = ifa->ifa_local,
	};
	struct {
		struct nlattr hdr;
		u32 oif;
	} oif_attr = {
		.oif = ifa->ifa_dev->dev->ifindex,
	};
	struct nlattr *attrs[RTA_MAX+1] = {
		[RTA_DST] = (struct nlattr *) &dst_attr,
		[RTA_PREFSRC] = (struct nlattr * ) &prefsrc_attr,
		[RTA_OIF] = (struct nlattr *) &oif_attr,
	};

	memset(&req.rtm, 0, sizeof(req.rtm));

	if (type == RTN_UNICAST)
		tb = dn_fib_get_table(RT_MIN_TABLE, 1);
	else
		tb = dn_fib_get_table(RT_TABLE_LOCAL, 1);

	if (tb == NULL)
		return;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = cmd;
	req.nlh.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_APPEND;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = 0;

	req.rtm.rtm_dst_len = dst_len;
	req.rtm.rtm_table = tb->n;
	req.rtm.rtm_protocol = RTPROT_KERNEL;
	req.rtm.rtm_scope = (type != RTN_LOCAL ? RT_SCOPE_LINK : RT_SCOPE_HOST);
	req.rtm.rtm_type = type;

	if (cmd == RTM_NEWROUTE)
		tb->insert(tb, &req.rtm, attrs, &req.nlh, NULL);
	else
		tb->delete(tb, &req.rtm, attrs, &req.nlh, NULL);
}

static void dn_fib_add_ifaddr(struct dn_ifaddr *ifa)
{

	fib_magic(RTM_NEWROUTE, RTN_LOCAL, ifa->ifa_local, 16, ifa);

#if 0
	if (!(dev->flags&IFF_UP))
		return;
	/* In the future, we will want to add default routes here */

#endif
}

static void dn_fib_del_ifaddr(struct dn_ifaddr *ifa)
{
	int found_it = 0;
	struct net_device *dev;
	struct dn_dev *dn_db;
	struct dn_ifaddr *ifa2;

	ASSERT_RTNL();

	/* Scan device list */
	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev) {
		dn_db = rcu_dereference(dev->dn_ptr);
		if (dn_db == NULL)
			continue;
		for (ifa2 = rcu_dereference(dn_db->ifa_list);
		     ifa2 != NULL;
		     ifa2 = rcu_dereference(ifa2->ifa_next)) {
			if (ifa2->ifa_local == ifa->ifa_local) {
				found_it = 1;
				break;
			}
		}
	}
	rcu_read_unlock();

	if (found_it == 0) {
		fib_magic(RTM_DELROUTE, RTN_LOCAL, ifa->ifa_local, 16, ifa);

		if (dnet_addr_type(ifa->ifa_local) != RTN_LOCAL) {
			if (dn_fib_sync_down(ifa->ifa_local, NULL, 0))
				dn_fib_flush();
		}
	}
}

static void dn_fib_disable_addr(struct net_device *dev, int force)
{
	if (dn_fib_sync_down(0, dev, force))
		dn_fib_flush();
	dn_rt_cache_flush(0);
	neigh_ifdown(&dn_neigh_table, dev);
}

static int dn_fib_dnaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct dn_ifaddr *ifa = (struct dn_ifaddr *)ptr;

	switch (event) {
	case NETDEV_UP:
		dn_fib_add_ifaddr(ifa);
		dn_fib_sync_up(ifa->ifa_dev->dev);
		dn_rt_cache_flush(-1);
		break;
	case NETDEV_DOWN:
		dn_fib_del_ifaddr(ifa);
		if (ifa->ifa_dev && ifa->ifa_dev->ifa_list == NULL) {
			dn_fib_disable_addr(ifa->ifa_dev->dev, 1);
		} else {
			dn_rt_cache_flush(-1);
		}
		break;
	}
	return NOTIFY_DONE;
}

static int dn_fib_sync_down(__le16 local, struct net_device *dev, int force)
{
	int ret = 0;
	int scope = RT_SCOPE_NOWHERE;

	if (force)
		scope = -1;

	for_fib_info() {
		/*
		 * This makes no sense for DECnet.... we will almost
		 * certainly have more than one local address the same
		 * over all our interfaces. It needs thinking about
		 * some more.
		 */
		if (local && fi->fib_prefsrc == local) {
			fi->fib_flags |= RTNH_F_DEAD;
			ret++;
		} else if (dev && fi->fib_nhs) {
			int dead = 0;

			change_nexthops(fi) {
				if (nh->nh_flags&RTNH_F_DEAD)
					dead++;
				else if (nh->nh_dev == dev &&
						nh->nh_scope != scope) {
					spin_lock_bh(&dn_fib_multipath_lock);
					nh->nh_flags |= RTNH_F_DEAD;
					fi->fib_power -= nh->nh_power;
					nh->nh_power = 0;
					spin_unlock_bh(&dn_fib_multipath_lock);
					dead++;
				}
			} endfor_nexthops(fi)
			if (dead == fi->fib_nhs) {
				fi->fib_flags |= RTNH_F_DEAD;
				ret++;
			}
		}
	} endfor_fib_info();
	return ret;
}


static int dn_fib_sync_up(struct net_device *dev)
{
	int ret = 0;

	if (!(dev->flags&IFF_UP))
		return 0;

	for_fib_info() {
		int alive = 0;

		change_nexthops(fi) {
			if (!(nh->nh_flags&RTNH_F_DEAD)) {
				alive++;
				continue;
			}
			if (nh->nh_dev == NULL || !(nh->nh_dev->flags&IFF_UP))
				continue;
			if (nh->nh_dev != dev || dev->dn_ptr == NULL)
				continue;
			alive++;
			spin_lock_bh(&dn_fib_multipath_lock);
			nh->nh_power = 0;
			nh->nh_flags &= ~RTNH_F_DEAD;
			spin_unlock_bh(&dn_fib_multipath_lock);
		} endfor_nexthops(fi);

		if (alive > 0) {
			fi->fib_flags &= ~RTNH_F_DEAD;
			ret++;
		}
	} endfor_fib_info();
	return ret;
}

static struct notifier_block dn_fib_dnaddr_notifier = {
	.notifier_call = dn_fib_dnaddr_event,
};

void __exit dn_fib_cleanup(void)
{
	dn_fib_table_cleanup();
	dn_fib_rules_cleanup();

	unregister_dnaddr_notifier(&dn_fib_dnaddr_notifier);
}


void __init dn_fib_init(void)
{
	dn_fib_table_init();
	dn_fib_rules_init();

	register_dnaddr_notifier(&dn_fib_dnaddr_notifier);

	rtnl_register(PF_DECnet, RTM_NEWROUTE, dn_fib_rtm_newroute, NULL, NULL);
	rtnl_register(PF_DECnet, RTM_DELROUTE, dn_fib_rtm_delroute, NULL, NULL);
}


{
	struct net *net = sock_net(skb->sk);
	struct dn_fib_table *tb;
	struct rtmsg *r = nlmsg_data(nlh);
	struct nlattr *attrs[RTA_MAX+1];
	int err;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (!net_eq(net, &init_net))
		return -EINVAL;

	err = nlmsg_parse(nlh, sizeof(*r), attrs, RTA_MAX, rtm_dn_policy);
	if (err < 0)
		return err;

	tb = dn_fib_get_table(rtm_get_table(attrs, r->rtm_table), 1);
	if (!tb)
		return -ENOBUFS;

	return tb->insert(tb, r, attrs, nlh, &NETLINK_CB(skb));
}

static void fib_magic(int cmd, int type, __le16 dst, int dst_len, struct dn_ifaddr *ifa)
{
	struct dn_fib_table *tb;
	struct {
		struct nlmsghdr nlh;
		struct rtmsg rtm;
	} req;
	struct {
		struct nlattr hdr;
		__le16 dst;
	} dst_attr = {
		.dst = dst,
	};
	struct {
		struct nlattr hdr;
		__le16 prefsrc;
	} prefsrc_attr = {
		.prefsrc = ifa->ifa_local,
	};
	struct {
		struct nlattr hdr;
		u32 oif;
	} oif_attr = {
		.oif = ifa->ifa_dev->dev->ifindex,
	};
	struct nlattr *attrs[RTA_MAX+1] = {
		[RTA_DST] = (struct nlattr *) &dst_attr,
		[RTA_PREFSRC] = (struct nlattr * ) &prefsrc_attr,
		[RTA_OIF] = (struct nlattr *) &oif_attr,
	};

	memset(&req.rtm, 0, sizeof(req.rtm));

	if (type == RTN_UNICAST)
		tb = dn_fib_get_table(RT_MIN_TABLE, 1);
	else
		tb = dn_fib_get_table(RT_TABLE_LOCAL, 1);

	if (tb == NULL)
		return;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = cmd;
	req.nlh.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_APPEND;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = 0;

	req.rtm.rtm_dst_len = dst_len;
	req.rtm.rtm_table = tb->n;
	req.rtm.rtm_protocol = RTPROT_KERNEL;
	req.rtm.rtm_scope = (type != RTN_LOCAL ? RT_SCOPE_LINK : RT_SCOPE_HOST);
	req.rtm.rtm_type = type;

	if (cmd == RTM_NEWROUTE)
		tb->insert(tb, &req.rtm, attrs, &req.nlh, NULL);
	else
		tb->delete(tb, &req.rtm, attrs, &req.nlh, NULL);
}

static void dn_fib_add_ifaddr(struct dn_ifaddr *ifa)
{

	fib_magic(RTM_NEWROUTE, RTN_LOCAL, ifa->ifa_local, 16, ifa);

#if 0
	if (!(dev->flags&IFF_UP))
		return;
	/* In the future, we will want to add default routes here */

#endif
}

static void dn_fib_del_ifaddr(struct dn_ifaddr *ifa)
{
	int found_it = 0;
	struct net_device *dev;
	struct dn_dev *dn_db;
	struct dn_ifaddr *ifa2;

	ASSERT_RTNL();

	/* Scan device list */
	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev) {
		dn_db = rcu_dereference(dev->dn_ptr);
		if (dn_db == NULL)
			continue;
		for (ifa2 = rcu_dereference(dn_db->ifa_list);
		     ifa2 != NULL;
		     ifa2 = rcu_dereference(ifa2->ifa_next)) {
			if (ifa2->ifa_local == ifa->ifa_local) {
				found_it = 1;
				break;
			}
		}
	}
	rcu_read_unlock();

	if (found_it == 0) {
		fib_magic(RTM_DELROUTE, RTN_LOCAL, ifa->ifa_local, 16, ifa);

		if (dnet_addr_type(ifa->ifa_local) != RTN_LOCAL) {
			if (dn_fib_sync_down(ifa->ifa_local, NULL, 0))
				dn_fib_flush();
		}
	}
}

static void dn_fib_disable_addr(struct net_device *dev, int force)
{
	if (dn_fib_sync_down(0, dev, force))
		dn_fib_flush();
	dn_rt_cache_flush(0);
	neigh_ifdown(&dn_neigh_table, dev);
}

static int dn_fib_dnaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct dn_ifaddr *ifa = (struct dn_ifaddr *)ptr;

	switch (event) {
	case NETDEV_UP:
		dn_fib_add_ifaddr(ifa);
		dn_fib_sync_up(ifa->ifa_dev->dev);
		dn_rt_cache_flush(-1);
		break;
	case NETDEV_DOWN:
		dn_fib_del_ifaddr(ifa);
		if (ifa->ifa_dev && ifa->ifa_dev->ifa_list == NULL) {
			dn_fib_disable_addr(ifa->ifa_dev->dev, 1);
		} else {
			dn_rt_cache_flush(-1);
		}
		break;
	}
	return NOTIFY_DONE;
}

static int dn_fib_sync_down(__le16 local, struct net_device *dev, int force)
{
	int ret = 0;
	int scope = RT_SCOPE_NOWHERE;

	if (force)
		scope = -1;

	for_fib_info() {
		/*
		 * This makes no sense for DECnet.... we will almost
		 * certainly have more than one local address the same
		 * over all our interfaces. It needs thinking about
		 * some more.
		 */
		if (local && fi->fib_prefsrc == local) {
			fi->fib_flags |= RTNH_F_DEAD;
			ret++;
		} else if (dev && fi->fib_nhs) {
			int dead = 0;

			change_nexthops(fi) {
				if (nh->nh_flags&RTNH_F_DEAD)
					dead++;
				else if (nh->nh_dev == dev &&
						nh->nh_scope != scope) {
					spin_lock_bh(&dn_fib_multipath_lock);
					nh->nh_flags |= RTNH_F_DEAD;
					fi->fib_power -= nh->nh_power;
					nh->nh_power = 0;
					spin_unlock_bh(&dn_fib_multipath_lock);
					dead++;
				}
			} endfor_nexthops(fi)
			if (dead == fi->fib_nhs) {
				fi->fib_flags |= RTNH_F_DEAD;
				ret++;
			}
		}
	} endfor_fib_info();
	return ret;
}


static int dn_fib_sync_up(struct net_device *dev)
{
	int ret = 0;

	if (!(dev->flags&IFF_UP))
		return 0;

	for_fib_info() {
		int alive = 0;

		change_nexthops(fi) {
			if (!(nh->nh_flags&RTNH_F_DEAD)) {
				alive++;
				continue;
			}
			if (nh->nh_dev == NULL || !(nh->nh_dev->flags&IFF_UP))
				continue;
			if (nh->nh_dev != dev || dev->dn_ptr == NULL)
				continue;
			alive++;
			spin_lock_bh(&dn_fib_multipath_lock);
			nh->nh_power = 0;
			nh->nh_flags &= ~RTNH_F_DEAD;
			spin_unlock_bh(&dn_fib_multipath_lock);
		} endfor_nexthops(fi);

		if (alive > 0) {
			fi->fib_flags &= ~RTNH_F_DEAD;
			ret++;
		}
	} endfor_fib_info();
	return ret;
}

static struct notifier_block dn_fib_dnaddr_notifier = {
	.notifier_call = dn_fib_dnaddr_event,
};

void __exit dn_fib_cleanup(void)
{
	dn_fib_table_cleanup();
	dn_fib_rules_cleanup();

	unregister_dnaddr_notifier(&dn_fib_dnaddr_notifier);
}


void __init dn_fib_init(void)
{
	dn_fib_table_init();
	dn_fib_rules_init();

	register_dnaddr_notifier(&dn_fib_dnaddr_notifier);

	rtnl_register(PF_DECnet, RTM_NEWROUTE, dn_fib_rtm_newroute, NULL, NULL);
	rtnl_register(PF_DECnet, RTM_DELROUTE, dn_fib_rtm_delroute, NULL, NULL);
}

