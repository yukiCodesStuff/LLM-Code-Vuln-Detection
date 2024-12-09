	if (err)
		goto err2;

	if (br_netpoll_info(br) && ((err = br_netpoll_enable(p, GFP_KERNEL))))
		goto err3;

	err = netdev_set_master(dev, br->dev);
	if (err)
	if (!p || p->br != br)
		return -EINVAL;

	/* Since more than one interface can be attached to a bridge,
	 * there still maybe an alternate path for netconsole to use;
	 * therefore there is no reason for a NETDEV_RELEASE event.
	 */
	del_nbp(p);

	spin_lock_bh(&br->lock);
	changed_addr = br_stp_recalculate_bridge_id(br);