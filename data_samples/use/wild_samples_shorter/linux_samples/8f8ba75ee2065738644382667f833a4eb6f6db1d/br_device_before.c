	struct net_bridge_mdb_entry *mdst;
	struct br_cpu_netstats *brstats = this_cpu_ptr(br->stats);

#ifdef CONFIG_BRIDGE_NETFILTER
	if (skb->nf_bridge && (skb->nf_bridge->mask & BRNF_BRIDGED_DNAT)) {
		br_nf_pre_routing_finish_bridge_slow(skb);
		return NETDEV_TX_OK;
	}
#endif

	skb_reset_mac_header(skb);
	skb_pull(skb, ETH_HLEN);

	rcu_read_lock();
	if (is_broadcast_ether_addr(dest))
		br_flood_deliver(br, skb);
	else if (is_multicast_ether_addr(dest)) {
		if (unlikely(netpoll_tx_running(dev))) {
static void br_netpoll_cleanup(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *p, *n;

	list_for_each_entry_safe(p, n, &br->port_list, list) {
		br_netpoll_disable(p);
	}
}

static int br_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *p, *n;
	int err = 0;

	list_for_each_entry_safe(p, n, &br->port_list, list) {
		if (!p->dev)
			continue;

		err = br_netpoll_enable(p);
		if (err)
			goto fail;
	}

	goto out;
}

int br_netpoll_enable(struct net_bridge_port *p)
{
	struct netpoll *np;
	int err = 0;

	np = kzalloc(sizeof(*p->np), GFP_KERNEL);
	err = -ENOMEM;
	if (!np)
		goto out;

	err = __netpoll_setup(np, p->dev);
	if (err) {
		kfree(np);
		goto out;
	}

	p->np = NULL;

	/* Wait for transmitting packets to finish before freeing. */
	synchronize_rcu_bh();

	__netpoll_cleanup(np);
	kfree(np);
}

#endif
