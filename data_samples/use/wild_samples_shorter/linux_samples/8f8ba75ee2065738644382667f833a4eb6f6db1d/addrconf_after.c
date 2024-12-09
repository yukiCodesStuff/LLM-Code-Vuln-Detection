	struct net_device *dev;
	struct inet6_dev *idev;

	for_each_netdev(net, dev) {
		idev = __in6_dev_get(dev);
		if (idev) {
			int changed = (!idev->cnf.forwarding) ^ (!newf);
			idev->cnf.forwarding = newf;
				dev_forward_change(idev);
		}
	}
}

static int addrconf_fixup_forwarding(struct ctl_table *table, int *p, int newf)
{