	bool skb_xdp = false;
	struct page *page;

	if (unlikely(datasize < ETH_HLEN))
		return -EINVAL;

	xdp_prog = rcu_dereference(tun->xdp_prog);
	if (xdp_prog) {
		if (gso->gso_type) {
			skb_xdp = true;