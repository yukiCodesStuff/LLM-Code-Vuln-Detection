}

/*
 * Collect up to maxaddrs worth of a netdevice's unicast addresses, starting
 * at a specified offset within the list, into an array of addrss pointers and
 * return the number collected.
 */
static inline unsigned int collect_netdev_uc_list_addrs(const struct net_device *dev,
							const u8 **addr,
							unsigned int offset,
							unsigned int maxaddrs)
{
	unsigned int index = 0;
	unsigned int naddr = 0;
	const struct netdev_hw_addr *ha;

	for_each_dev_addr(dev, ha)
		if (index++ >= offset) {
			addr[naddr++] = ha->addr;
			if (naddr >= maxaddrs)
				break;
		}
	return naddr;
}

/*
 * Collect up to maxaddrs worth of a netdevice's multicast addresses, starting
 * at a specified offset within the list, into an array of addrss pointers and
 * return the number collected.
 */
static inline unsigned int collect_netdev_mc_list_addrs(const struct net_device *dev,
							const u8 **addr,
							unsigned int offset,
							unsigned int maxaddrs)
{
	unsigned int index = 0;
	unsigned int naddr = 0;
	const struct netdev_hw_addr *ha;

	netdev_for_each_mc_addr(ha, dev)
		if (index++ >= offset) {
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
	unsigned int offset, naddr;
	const u8 *addr[7];
	int ret;
	const struct port_info *pi = netdev_priv(dev);

	/* first do the secondary unicast addresses */
	for (offset = 0; ; offset += naddr) {
		naddr = collect_netdev_uc_list_addrs(dev, addr, offset,
						     ARRAY_SIZE(addr));
		if (naddr == 0)
			break;

		ret = t4vf_alloc_mac_filt(pi->adapter, pi->viid, free,
					  naddr, addr, NULL, &uhash, sleep);
		if (ret < 0)
			return ret;

		free = false;
	}

	/* next set up the multicast addresses */
	for (offset = 0; ; offset += naddr) {
		naddr = collect_netdev_mc_list_addrs(dev, addr, offset,
						     ARRAY_SIZE(addr));
		if (naddr == 0)
			break;

		ret = t4vf_alloc_mac_filt(pi->adapter, pi->viid, free,
					  naddr, addr, NULL, &mhash, sleep);
		if (ret < 0)
			return ret;
		free = false;
	}

	return t4vf_set_addr_hash(pi->adapter, pi->viid, uhash != 0,
				  uhash | mhash, sleep);