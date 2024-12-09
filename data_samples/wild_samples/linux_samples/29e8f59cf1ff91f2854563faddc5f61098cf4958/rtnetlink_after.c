		} else {
			return -EOPNOTSUPP;
		}
	}
	return 0;
}

static int do_setlink(const struct sk_buff *skb,
		      struct net_device *dev, struct ifinfomsg *ifm,
		      struct nlattr **tb, char *ifname, int modified)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int err;

	if (tb[IFLA_NET_NS_PID] || tb[IFLA_NET_NS_FD]) {
		struct net *net = rtnl_link_get_net(dev_net(dev), tb);
		if (IS_ERR(net)) {
			err = PTR_ERR(net);
			goto errout;
		}
		if (!netlink_ns_capable(skb, net->user_ns, CAP_NET_ADMIN)) {
			err = -EPERM;
			goto errout;
		}
		err = dev_change_net_namespace(dev, net, ifname);
		put_net(net);
		if (err)
			goto errout;
		modified = 1;
	}
		if (IS_ERR(net)) {
			err = PTR_ERR(net);
			goto errout;
		}
		if (!netlink_ns_capable(skb, net->user_ns, CAP_NET_ADMIN)) {
			err = -EPERM;
			goto errout;
		}
		err = dev_change_net_namespace(dev, net, ifname);
		put_net(net);
		if (err)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_MAP]) {
		struct rtnl_link_ifmap *u_map;
		struct ifmap k_map;

		if (!ops->ndo_set_config) {
			err = -EOPNOTSUPP;
			goto errout;
		}

		if (!netif_device_present(dev)) {
			err = -ENODEV;
			goto errout;
		}

		u_map = nla_data(tb[IFLA_MAP]);
		k_map.mem_start = (unsigned long) u_map->mem_start;
		k_map.mem_end = (unsigned long) u_map->mem_end;
		k_map.base_addr = (unsigned short) u_map->base_addr;
		k_map.irq = (unsigned char) u_map->irq;
		k_map.dma = (unsigned char) u_map->dma;
		k_map.port = (unsigned char) u_map->port;

		err = ops->ndo_set_config(dev, &k_map);
		if (err < 0)
			goto errout;

		modified = 1;
	}

	if (tb[IFLA_ADDRESS]) {
		struct sockaddr *sa;
		int len;

		len = sizeof(sa_family_t) + dev->addr_len;
		sa = kmalloc(len, GFP_KERNEL);
		if (!sa) {
			err = -ENOMEM;
			goto errout;
		}
		sa->sa_family = dev->type;
		memcpy(sa->sa_data, nla_data(tb[IFLA_ADDRESS]),
		       dev->addr_len);
		err = dev_set_mac_address(dev, sa);
		kfree(sa);
		if (err)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_MTU]) {
		err = dev_set_mtu(dev, nla_get_u32(tb[IFLA_MTU]));
		if (err < 0)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_GROUP]) {
		dev_set_group(dev, nla_get_u32(tb[IFLA_GROUP]));
		modified = 1;
	}

	/*
	 * Interface selected by interface index but interface
	 * name provided implies that a name change has been
	 * requested.
	 */
	if (ifm->ifi_index > 0 && ifname[0]) {
		err = dev_change_name(dev, ifname);
		if (err < 0)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_IFALIAS]) {
		err = dev_set_alias(dev, nla_data(tb[IFLA_IFALIAS]),
				    nla_len(tb[IFLA_IFALIAS]));
		if (err < 0)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_BROADCAST]) {
		nla_memcpy(dev->broadcast, tb[IFLA_BROADCAST], dev->addr_len);
		call_netdevice_notifiers(NETDEV_CHANGEADDR, dev);
	}

	if (ifm->ifi_flags || ifm->ifi_change) {
		err = dev_change_flags(dev, rtnl_dev_combine_flags(dev, ifm));
		if (err < 0)
			goto errout;
	}

	if (tb[IFLA_MASTER]) {
		err = do_set_master(dev, nla_get_u32(tb[IFLA_MASTER]));
		if (err)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_CARRIER]) {
		err = dev_change_carrier(dev, nla_get_u8(tb[IFLA_CARRIER]));
		if (err)
			goto errout;
		modified = 1;
	}

	if (tb[IFLA_TXQLEN])
		dev->tx_queue_len = nla_get_u32(tb[IFLA_TXQLEN]);

	if (tb[IFLA_OPERSTATE])
		set_operstate(dev, nla_get_u8(tb[IFLA_OPERSTATE]));

	if (tb[IFLA_LINKMODE]) {
		write_lock_bh(&dev_base_lock);
		dev->link_mode = nla_get_u8(tb[IFLA_LINKMODE]);
		write_unlock_bh(&dev_base_lock);
	}

	if (tb[IFLA_VFINFO_LIST]) {
		struct nlattr *attr;
		int rem;
		nla_for_each_nested(attr, tb[IFLA_VFINFO_LIST], rem) {
			if (nla_type(attr) != IFLA_VF_INFO) {
				err = -EINVAL;
				goto errout;
			}
	if (dev == NULL) {
		err = -ENODEV;
		goto errout;
	}

	err = validate_linkmsg(dev, tb);
	if (err < 0)
		goto errout;

	err = do_setlink(skb, dev, ifm, tb, ifname, 0);
errout:
	return err;
}

static int rtnl_dellink(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	struct net *net = sock_net(skb->sk);
	const struct rtnl_link_ops *ops;
	struct net_device *dev;
	struct ifinfomsg *ifm;
	char ifname[IFNAMSIZ];
	struct nlattr *tb[IFLA_MAX+1];
	int err;
	LIST_HEAD(list_kill);

	err = nlmsg_parse(nlh, sizeof(*ifm), tb, IFLA_MAX, ifla_policy);
	if (err < 0)
		return err;

	if (tb[IFLA_IFNAME])
		nla_strlcpy(ifname, tb[IFLA_IFNAME], IFNAMSIZ);

	ifm = nlmsg_data(nlh);
	if (ifm->ifi_index > 0)
		dev = __dev_get_by_index(net, ifm->ifi_index);
	else if (tb[IFLA_IFNAME])
		dev = __dev_get_by_name(net, ifname);
	else
		return -EINVAL;

	if (!dev)
		return -ENODEV;

	ops = dev->rtnl_link_ops;
	if (!ops)
		return -EOPNOTSUPP;

	ops->dellink(dev, &list_kill);
	unregister_netdevice_many(&list_kill);
	list_del(&list_kill);
	return 0;
}

int rtnl_configure_link(struct net_device *dev, const struct ifinfomsg *ifm)
{
	unsigned int old_flags;
	int err;

	old_flags = dev->flags;
	if (ifm && (ifm->ifi_flags || ifm->ifi_change)) {
		err = __dev_change_flags(dev, rtnl_dev_combine_flags(dev, ifm));
		if (err < 0)
			return err;
	}
	if (tb[IFLA_ADDRESS]) {
		memcpy(dev->dev_addr, nla_data(tb[IFLA_ADDRESS]),
				nla_len(tb[IFLA_ADDRESS]));
		dev->addr_assign_type = NET_ADDR_SET;
	}
	if (tb[IFLA_BROADCAST])
		memcpy(dev->broadcast, nla_data(tb[IFLA_BROADCAST]),
				nla_len(tb[IFLA_BROADCAST]));
	if (tb[IFLA_TXQLEN])
		dev->tx_queue_len = nla_get_u32(tb[IFLA_TXQLEN]);
	if (tb[IFLA_OPERSTATE])
		set_operstate(dev, nla_get_u8(tb[IFLA_OPERSTATE]));
	if (tb[IFLA_LINKMODE])
		dev->link_mode = nla_get_u8(tb[IFLA_LINKMODE]);
	if (tb[IFLA_GROUP])
		dev_set_group(dev, nla_get_u32(tb[IFLA_GROUP]));

	return dev;

err:
	return ERR_PTR(err);
}
EXPORT_SYMBOL(rtnl_create_link);

static int rtnl_group_changelink(const struct sk_buff *skb,
		struct net *net, int group,
		struct ifinfomsg *ifm,
		struct nlattr **tb)
{
	struct net_device *dev;
	int err;

	for_each_netdev(net, dev) {
		if (dev->group == group) {
			err = do_setlink(skb, dev, ifm, tb, NULL, 0);
			if (err < 0)
				return err;
		}
	}

	return 0;
}
	for_each_netdev(net, dev) {
		if (dev->group == group) {
			err = do_setlink(skb, dev, ifm, tb, NULL, 0);
			if (err < 0)
				return err;
		}
	}
			if (linkinfo[IFLA_INFO_SLAVE_DATA]) {
				if (!m_ops || !m_ops->slave_changelink)
					return -EOPNOTSUPP;

				err = m_ops->slave_changelink(master_dev, dev,
							      tb, slave_data);
				if (err < 0)
					return err;
				modified = 1;
			}

			return do_setlink(skb, dev, ifm, tb, ifname, modified);
		}

		if (!(nlh->nlmsg_flags & NLM_F_CREATE)) {
			if (ifm->ifi_index == 0 && tb[IFLA_GROUP])
				return rtnl_group_changelink(skb, net,
						nla_get_u32(tb[IFLA_GROUP]),
						ifm, tb);
			return -ENODEV;
		}

		if (tb[IFLA_MAP] || tb[IFLA_MASTER] || tb[IFLA_PROTINFO])
			return -EOPNOTSUPP;

		if (!ops) {
#ifdef CONFIG_MODULES
			if (kind[0]) {
				__rtnl_unlock();
				request_module("rtnl-link-%s", kind);
				rtnl_lock();
				ops = rtnl_link_ops_get(kind);
				if (ops)
					goto replay;
			}
#endif
			return -EOPNOTSUPP;
		}

		if (!ifname[0])
			snprintf(ifname, IFNAMSIZ, "%s%%d", ops->kind);

		dest_net = rtnl_link_get_net(net, tb);
		if (IS_ERR(dest_net))
			return PTR_ERR(dest_net);

		dev = rtnl_create_link(dest_net, ifname, ops, tb);
		if (IS_ERR(dev)) {
			err = PTR_ERR(dev);
			goto out;
		}

		dev->ifindex = ifm->ifi_index;

		if (ops->newlink) {
			err = ops->newlink(net, dev, tb, data);
			/* Drivers should call free_netdev() in ->destructor
			 * and unregister it on failure so that device could be
			 * finally freed in rtnl_unlock.
			 */
			if (err < 0)
				goto out;
		} else {
			err = register_netdevice(dev);
			if (err < 0) {
				free_netdev(dev);
				goto out;
			}
{
	struct net *net = sock_net(skb->sk);
	struct ndmsg *ndm;
	struct nlattr *tb[NDA_MAX+1];
	struct net_device *dev;
	int err = -EINVAL;
	__u8 *addr;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(nlh, sizeof(*ndm), tb, NDA_MAX, NULL);
	if (err < 0)
		return err;

	ndm = nlmsg_data(nlh);
	if (ndm->ndm_ifindex == 0) {
		pr_info("PF_BRIDGE: RTM_DELNEIGH with invalid ifindex\n");
		return -EINVAL;
	}
{
	struct net *net = sock_net(skb->sk);
	rtnl_doit_func doit;
	int sz_idx, kind;
	int family;
	int type;
	int err;

	type = nlh->nlmsg_type;
	if (type > RTM_MAX)
		return -EOPNOTSUPP;

	type -= RTM_BASE;

	/* All the messages must have at least 1 byte length */
	if (nlmsg_len(nlh) < sizeof(struct rtgenmsg))
		return 0;

	family = ((struct rtgenmsg *)nlmsg_data(nlh))->rtgen_family;
	sz_idx = type>>2;
	kind = type&3;

	if (kind != 2 && !netlink_net_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (kind == 2 && nlh->nlmsg_flags&NLM_F_DUMP) {
		struct sock *rtnl;
		rtnl_dumpit_func dumpit;
		rtnl_calcit_func calcit;
		u16 min_dump_alloc = 0;

		dumpit = rtnl_get_dumpit(family, type);
		if (dumpit == NULL)
			return -EOPNOTSUPP;
		calcit = rtnl_get_calcit(family, type);
		if (calcit)
			min_dump_alloc = calcit(skb, nlh);

		__rtnl_unlock();
		rtnl = net->rtnl;
		{
			struct netlink_dump_control c = {
				.dump		= dumpit,
				.min_dump_alloc	= min_dump_alloc,
			};
			err = netlink_dump_start(rtnl, skb, nlh, &c);
		}
		rtnl_lock();
		return err;
	}

	doit = rtnl_get_doit(family, type);
	if (doit == NULL)
		return -EOPNOTSUPP;

	return doit(skb, nlh);
}