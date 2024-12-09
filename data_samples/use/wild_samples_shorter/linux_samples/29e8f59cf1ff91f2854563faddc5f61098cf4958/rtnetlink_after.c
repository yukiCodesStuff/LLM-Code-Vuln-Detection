	return 0;
}

static int do_setlink(const struct sk_buff *skb,
		      struct net_device *dev, struct ifinfomsg *ifm,
		      struct nlattr **tb, char *ifname, int modified)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	int err;
			err = PTR_ERR(net);
			goto errout;
		}
		if (!netlink_ns_capable(skb, net->user_ns, CAP_NET_ADMIN)) {
			err = -EPERM;
			goto errout;
		}
		err = dev_change_net_namespace(dev, net, ifname);
	if (err < 0)
		goto errout;

	err = do_setlink(skb, dev, ifm, tb, ifname, 0);
errout:
	return err;
}

}
EXPORT_SYMBOL(rtnl_create_link);

static int rtnl_group_changelink(const struct sk_buff *skb,
		struct net *net, int group,
		struct ifinfomsg *ifm,
		struct nlattr **tb)
{
	struct net_device *dev;

	for_each_netdev(net, dev) {
		if (dev->group == group) {
			err = do_setlink(skb, dev, ifm, tb, NULL, 0);
			if (err < 0)
				return err;
		}
	}
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
	int err = -EINVAL;
	__u8 *addr;

	if (!netlink_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(nlh, sizeof(*ndm), tb, NDA_MAX, NULL);
	if (err < 0)
	sz_idx = type>>2;
	kind = type&3;

	if (kind != 2 && !netlink_net_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (kind == 2 && nlh->nlmsg_flags&NLM_F_DUMP) {
		struct sock *rtnl;