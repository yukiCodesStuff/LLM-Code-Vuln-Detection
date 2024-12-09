static const struct nla_policy macvlan_policy[IFLA_MACVLAN_MAX + 1] = {
	[IFLA_MACVLAN_MODE]  = { .type = NLA_U32 },
	[IFLA_MACVLAN_FLAGS] = { .type = NLA_U16 },
	[IFLA_MACVLAN_MACADDR_MODE] = { .type = NLA_U32 },
	[IFLA_MACVLAN_MACADDR] = { .type = NLA_BINARY, .len = MAX_ADDR_LEN },
	[IFLA_MACVLAN_MACADDR_DATA] = { .type = NLA_NESTED },
	[IFLA_MACVLAN_MACADDR_COUNT] = { .type = NLA_U32 },
	[IFLA_MACVLAN_BC_QUEUE_LEN] = { .type = NLA_U32 },
	[IFLA_MACVLAN_BC_QUEUE_LEN_USED] = { .type = NLA_REJECT },
};

int macvlan_link_register(struct rtnl_link_ops *ops)
{
	/* common fields */
	ops->validate		= macvlan_validate;
	ops->maxtype		= IFLA_MACVLAN_MAX;
	ops->policy		= macvlan_policy;
	ops->changelink		= macvlan_changelink;
	ops->get_size		= macvlan_get_size;
	ops->fill_info		= macvlan_fill_info;

	return rtnl_link_register(ops);
};
EXPORT_SYMBOL_GPL(macvlan_link_register);

static struct net *macvlan_get_link_net(const struct net_device *dev)
{
	return dev_net(macvlan_dev_real_dev(dev));
}

static struct rtnl_link_ops macvlan_link_ops = {
	.kind		= "macvlan",
	.setup		= macvlan_setup,
	.newlink	= macvlan_newlink,
	.dellink	= macvlan_dellink,
	.get_link_net	= macvlan_get_link_net,
	.priv_size      = sizeof(struct macvlan_dev),
};

static void update_port_bc_queue_len(struct macvlan_port *port)
{
	u32 max_bc_queue_len_req = 0;
	struct macvlan_dev *vlan;

	list_for_each_entry(vlan, &port->vlans, list) {
		if (vlan->bc_queue_len_req > max_bc_queue_len_req)
			max_bc_queue_len_req = vlan->bc_queue_len_req;
	}
	port->bc_queue_len_used = max_bc_queue_len_req;
}

static int macvlan_device_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct macvlan_dev *vlan, *next;
	struct macvlan_port *port;
	LIST_HEAD(list_kill);

	if (!netif_is_macvlan_port(dev))
		return NOTIFY_DONE;

	port = macvlan_port_get_rtnl(dev);

	switch (event) {
	case NETDEV_UP:
	case NETDEV_DOWN:
	case NETDEV_CHANGE:
		list_for_each_entry(vlan, &port->vlans, list)
			netif_stacked_transfer_operstate(vlan->lowerdev,
							 vlan->dev);
		break;
	case NETDEV_FEAT_CHANGE:
		list_for_each_entry(vlan, &port->vlans, list) {
			netif_inherit_tso_max(vlan->dev, dev);
			netdev_update_features(vlan->dev);
		}
		break;
	case NETDEV_CHANGEMTU:
		list_for_each_entry(vlan, &port->vlans, list) {
			if (vlan->dev->mtu <= dev->mtu)
				continue;
			dev_set_mtu(vlan->dev, dev->mtu);
		}
		break;
	case NETDEV_CHANGEADDR:
		if (!macvlan_passthru(port))
			return NOTIFY_DONE;

		vlan = list_first_entry_or_null(&port->vlans,
						struct macvlan_dev,
						list);

		if (vlan && macvlan_sync_address(vlan->dev, dev->dev_addr))
			return NOTIFY_BAD;

		break;
	case NETDEV_UNREGISTER:
		/* twiddle thumbs on netns device moves */
		if (dev->reg_state != NETREG_UNREGISTERING)
			break;

		list_for_each_entry_safe(vlan, next, &port->vlans, list)
			vlan->dev->rtnl_link_ops->dellink(vlan->dev, &list_kill);
		unregister_netdevice_many(&list_kill);
		break;
	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid underlying device to change its type. */
		return NOTIFY_BAD;

	case NETDEV_NOTIFY_PEERS:
	case NETDEV_BONDING_FAILOVER:
	case NETDEV_RESEND_IGMP:
		/* Propagate to all vlans */
		list_for_each_entry(vlan, &port->vlans, list)
			call_netdevice_notifiers(event, vlan->dev);
	}
	return NOTIFY_DONE;
}

static struct notifier_block macvlan_notifier_block __read_mostly = {
	.notifier_call	= macvlan_device_event,
};

static int __init macvlan_init_module(void)
{
	int err;

	register_netdevice_notifier(&macvlan_notifier_block);

	err = macvlan_link_register(&macvlan_link_ops);
	if (err < 0)
		goto err1;
	return 0;
err1:
	unregister_netdevice_notifier(&macvlan_notifier_block);
	return err;
}

static void __exit macvlan_cleanup_module(void)
{
	rtnl_link_unregister(&macvlan_link_ops);
	unregister_netdevice_notifier(&macvlan_notifier_block);
}

module_init(macvlan_init_module);
module_exit(macvlan_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_DESCRIPTION("Driver for MAC address based VLANs");
MODULE_ALIAS_RTNL_LINK("macvlan");