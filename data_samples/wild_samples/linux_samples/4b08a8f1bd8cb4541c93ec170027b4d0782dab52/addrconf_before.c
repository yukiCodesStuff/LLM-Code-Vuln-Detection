	if (tmp_prefered_lft <= regen_advance) {
		in6_ifa_put(ifp);
		in6_dev_put(idev);
		ret = -1;
		goto out;
	}

	addr_flags = IFA_F_TEMPORARY;
	/* set in addrconf_prefix_rcv() */
	if (ifp->flags & IFA_F_OPTIMISTIC)
		addr_flags |= IFA_F_OPTIMISTIC;

	ift = !max_addresses ||
	      ipv6_count_addresses(idev) < max_addresses ?
		ipv6_add_addr(idev, &addr, NULL, tmp_plen,
			      ipv6_addr_scope(&addr), addr_flags,
			      tmp_valid_lft, tmp_prefered_lft) : NULL;
	if (IS_ERR_OR_NULL(ift)) {
		in6_ifa_put(ifp);
		in6_dev_put(idev);
		pr_info("%s: retry temporary address regeneration\n", __func__);
		tmpaddr = &addr;
		write_lock(&idev->lock);
		goto retry;
	}