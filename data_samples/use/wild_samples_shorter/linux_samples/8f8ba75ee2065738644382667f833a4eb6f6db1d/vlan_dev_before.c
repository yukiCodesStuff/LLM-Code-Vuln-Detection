	return rc;
}

static netdev_tx_t vlan_dev_hard_start_xmit(struct sk_buff *skb,
					    struct net_device *dev)
{
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);
	unsigned int len;
	int ret;

	 * OTHER THINGS LIKE FDDI/TokenRing/802.3 SNAPs...
	 */
	if (veth->h_vlan_proto != htons(ETH_P_8021Q) ||
	    vlan_dev_priv(dev)->flags & VLAN_FLAG_REORDER_HDR) {
		u16 vlan_tci;
		vlan_tci = vlan_dev_priv(dev)->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(dev, skb);
		skb = __vlan_hwaccel_put_tag(skb, vlan_tci);
	}

	skb->dev = vlan_dev_priv(dev)->real_dev;
	len = skb->len;
	if (netpoll_tx_running(dev))
		return skb->dev->netdev_ops->ndo_start_xmit(skb, skb->dev);
	ret = dev_queue_xmit(skb);

	if (likely(ret == NET_XMIT_SUCCESS || ret == NET_XMIT_CN)) {
		struct vlan_pcpu_stats *stats;

		stats = this_cpu_ptr(vlan_dev_priv(dev)->vlan_pcpu_stats);
		u64_stats_update_begin(&stats->syncp);
		stats->tx_packets++;
		stats->tx_bytes += len;
		u64_stats_update_end(&stats->syncp);
	} else {
		this_cpu_inc(vlan_dev_priv(dev)->vlan_pcpu_stats->tx_dropped);
	}

	return ret;
}
	return;
}

static int vlan_dev_netpoll_setup(struct net_device *dev, struct netpoll_info *npinfo)
{
	struct vlan_dev_priv *info = vlan_dev_priv(dev);
	struct net_device *real_dev = info->real_dev;
	struct netpoll *netpoll;
	int err = 0;

	netpoll = kzalloc(sizeof(*netpoll), GFP_KERNEL);
	err = -ENOMEM;
	if (!netpoll)
		goto out;

	err = __netpoll_setup(netpoll, real_dev);
	if (err) {
		kfree(netpoll);
		goto out;
	}

	info->netpoll = netpoll;

out:
	return err;
}

static void vlan_dev_netpoll_cleanup(struct net_device *dev)
{
	struct vlan_dev_priv *info = vlan_dev_priv(dev);
	struct netpoll *netpoll = info->netpoll;

	if (!netpoll)
		return;

	info->netpoll = NULL;

        /* Wait for transmitting packets to finish before freeing. */
        synchronize_rcu_bh();

        __netpoll_cleanup(netpoll);
        kfree(netpoll);
}
#endif /* CONFIG_NET_POLL_CONTROLLER */

static const struct ethtool_ops vlan_ethtool_ops = {