		     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	skb->queue_mapping = qdisc_skb_cb(skb)->slave_dev_queue_mapping;

	if (unlikely(netpoll_tx_running(slave_dev)))
		bond_netpoll_send_skb(bond_get_slave_by_dev(bond, slave_dev), skb);
	else
		dev_queue_xmit(skb);

	struct netpoll *np;
	int err = 0;

	np = kzalloc(sizeof(*np), GFP_KERNEL);
	err = -ENOMEM;
	if (!np)
		goto out;

	err = __netpoll_setup(np, slave->dev);
	if (err) {
		kfree(np);
		goto out;
	}
		return;

	slave->np = NULL;
	synchronize_rcu_bh();
	__netpoll_cleanup(np);
	kfree(np);
}
static inline bool slave_dev_support_netpoll(struct net_device *slave_dev)
{
	if (slave_dev->priv_flags & IFF_DISABLE_NETPOLL)
	read_unlock(&bond->lock);
}

static int bond_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct bonding *bond = netdev_priv(dev);
	struct slave *slave;
	int i, err = 0;