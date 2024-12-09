{
	struct t4vf_port_stats stats;
	struct port_info *pi = netdev2pinfo(dev);
	struct adapter *adapter = pi->adapter;
	struct net_device_stats *ns = &dev->stats;
	int err;

	spin_lock(&adapter->stats_lock);
	err = t4vf_get_port_stats(adapter, pi->pidx, &stats);
	spin_unlock(&adapter->stats_lock);

	memset(ns, 0, sizeof(*ns));
	if (err)
		return ns;

	ns->tx_bytes = (stats.tx_bcast_bytes + stats.tx_mcast_bytes +
			stats.tx_ucast_bytes + stats.tx_offload_bytes);
	ns->tx_packets = (stats.tx_bcast_frames + stats.tx_mcast_frames +
			  stats.tx_ucast_frames + stats.tx_offload_frames);
	ns->rx_bytes = (stats.rx_bcast_bytes + stats.rx_mcast_bytes +
			stats.rx_ucast_bytes);
	ns->rx_packets = (stats.rx_bcast_frames + stats.rx_mcast_frames +
			  stats.rx_ucast_frames);
	ns->multicast = stats.rx_mcast_frames;
	ns->tx_errors = stats.tx_drop_frames;
	ns->rx_errors = stats.rx_err_frames;

	return ns;
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
{
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