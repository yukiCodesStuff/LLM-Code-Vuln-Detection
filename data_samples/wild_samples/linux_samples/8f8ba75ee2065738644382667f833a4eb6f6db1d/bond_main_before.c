{
	skb->dev = slave_dev;

	BUILD_BUG_ON(sizeof(skb->queue_mapping) !=
		     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	skb->queue_mapping = qdisc_skb_cb(skb)->slave_dev_queue_mapping;

	if (unlikely(netpoll_tx_running(slave_dev)))
		bond_netpoll_send_skb(bond_get_slave_by_dev(bond, slave_dev), skb);
	else
		dev_queue_xmit(skb);

	return 0;
}

/*
 * In the following 2 functions, bond_vlan_rx_add_vid and bond_vlan_rx_kill_vid,
 * We don't protect the slave list iteration with a lock because:
 * a. This operation is performed in IOCTL context,
 * b. The operation is protected by the RTNL semaphore in the 8021q code,
 * c. Holding a lock with BH disabled while directly calling a base driver
 *    entry point is generally a BAD idea.
 *
 * The design of synchronization/protection for this operation in the 8021q
 * module is good for one or more VLAN devices over a single physical device
 * and cannot be extended for a teaming solution like bonding, so there is a
 * potential race condition here where a net device from the vlan group might
 * be referenced (either by a base driver or the 8021q code) while it is being
 * removed from the system. However, it turns out we're not making matters
 * worse, and if it works for regular VLAN usage it will work here too.
*/

/**
 * bond_vlan_rx_add_vid - Propagates adding an id to slaves
 * @bond_dev: bonding net device that got called
 * @vid: vlan id being added
 */
static int bond_vlan_rx_add_vid(struct net_device *bond_dev, uint16_t vid)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *stop_at;
	int i, res;

	bond_for_each_slave(bond, slave, i) {
		res = vlan_vid_add(slave->dev, vid);
		if (res)
			goto unwind;
	}
{
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
{
	struct netpoll *np = slave->np;

	if (!np)
		return;

	slave->np = NULL;
	synchronize_rcu_bh();
	__netpoll_cleanup(np);
	kfree(np);
}
static inline bool slave_dev_support_netpoll(struct net_device *slave_dev)
{
	if (slave_dev->priv_flags & IFF_DISABLE_NETPOLL)
		return false;
	if (!slave_dev->netdev_ops->ndo_poll_controller)
		return false;
	return true;
}

static void bond_poll_controller(struct net_device *bond_dev)
{
}

static void __bond_netpoll_cleanup(struct bonding *bond)
{
	struct slave *slave;
	int i;

	bond_for_each_slave(bond, slave, i)
		if (IS_UP(slave->dev))
			slave_disable_netpoll(slave);
}
static void bond_netpoll_cleanup(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);

	read_lock(&bond->lock);
	__bond_netpoll_cleanup(bond);
	read_unlock(&bond->lock);
}

static int bond_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct bonding *bond = netdev_priv(dev);
	struct slave *slave;
	int i, err = 0;

	read_lock(&bond->lock);
	bond_for_each_slave(bond, slave, i) {
		err = slave_enable_netpoll(slave);
		if (err) {
			__bond_netpoll_cleanup(bond);
			break;
		}
	}
{
	struct bonding *bond = netdev_priv(bond_dev);

	read_lock(&bond->lock);
	__bond_netpoll_cleanup(bond);
	read_unlock(&bond->lock);
}

static int bond_netpoll_setup(struct net_device *dev, struct netpoll_info *ni)
{
	struct bonding *bond = netdev_priv(dev);
	struct slave *slave;
	int i, err = 0;

	read_lock(&bond->lock);
	bond_for_each_slave(bond, slave, i) {
		err = slave_enable_netpoll(slave);
		if (err) {
			__bond_netpoll_cleanup(bond);
			break;
		}
	}