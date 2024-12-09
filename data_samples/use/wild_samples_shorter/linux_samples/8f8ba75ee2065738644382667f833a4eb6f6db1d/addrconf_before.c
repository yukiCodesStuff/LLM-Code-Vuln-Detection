	struct net_device *dev;
	struct inet6_dev *idev;

	rcu_read_lock();
	for_each_netdev_rcu(net, dev) {
		idev = __in6_dev_get(dev);
		if (idev) {
			int changed = (!idev->cnf.forwarding) ^ (!newf);
			idev->cnf.forwarding = newf;
				dev_forward_change(idev);
		}
	}
	rcu_read_unlock();
}

static int addrconf_fixup_forwarding(struct ctl_table *table, int *p, int newf)
{