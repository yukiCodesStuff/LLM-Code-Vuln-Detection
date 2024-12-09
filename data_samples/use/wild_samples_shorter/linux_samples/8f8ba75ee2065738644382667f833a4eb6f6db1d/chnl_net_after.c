
	/* check the version of IP */
	ip_version = skb_header_pointer(skb, 0, 1, &buf);
	if (!ip_version) {
		kfree_skb(skb);
		return -EINVAL;
	}

	switch (*ip_version >> 4) {
	case 4:
		skb->protocol = htons(ETH_P_IP);