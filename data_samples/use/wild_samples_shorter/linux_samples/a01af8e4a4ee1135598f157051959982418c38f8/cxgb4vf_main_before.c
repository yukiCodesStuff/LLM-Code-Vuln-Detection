}

/*
 * Collect up to maxaddrs worth of a netdevice's unicast addresses into an
 * array of addrss pointers and return the number collected.
 */
static inline int collect_netdev_uc_list_addrs(const struct net_device *dev,
					       const u8 **addr,
					       unsigned int maxaddrs)
{
	unsigned int naddr = 0;
	const struct netdev_hw_addr *ha;

	for_each_dev_addr(dev, ha) {
		addr[naddr++] = ha->addr;
		if (naddr >= maxaddrs)
			break;
	}
	return naddr;
}

/*
 * Collect up to maxaddrs worth of a netdevice's multicast addresses into an
 * array of addrss pointers and return the number collected.
 */
static inline int collect_netdev_mc_list_addrs(const struct net_device *dev,
					       const u8 **addr,
					       unsigned int maxaddrs)
{
	unsigned int naddr = 0;
	const struct netdev_hw_addr *ha;

	netdev_for_each_mc_addr(ha, dev) {
		addr[naddr++] = ha->addr;
		if (naddr >= maxaddrs)
			break;
	}
	return naddr;
}

/*
	u64 mhash = 0;
	u64 uhash = 0;
	bool free = true;
	u16 filt_idx[7];
	const u8 *addr[7];
	int ret, naddr = 0;
	const struct port_info *pi = netdev_priv(dev);

	/* first do the secondary unicast addresses */
	naddr = collect_netdev_uc_list_addrs(dev, addr, ARRAY_SIZE(addr));
	if (naddr > 0) {
		ret = t4vf_alloc_mac_filt(pi->adapter, pi->viid, free,
					  naddr, addr, filt_idx, &uhash, sleep);
		if (ret < 0)
			return ret;

		free = false;
	}

	/* next set up the multicast addresses */
	naddr = collect_netdev_mc_list_addrs(dev, addr, ARRAY_SIZE(addr));
	if (naddr > 0) {
		ret = t4vf_alloc_mac_filt(pi->adapter, pi->viid, free,
					  naddr, addr, filt_idx, &mhash, sleep);
		if (ret < 0)
			return ret;
	}

	return t4vf_set_addr_hash(pi->adapter, pi->viid, uhash != 0,
				  uhash | mhash, sleep);