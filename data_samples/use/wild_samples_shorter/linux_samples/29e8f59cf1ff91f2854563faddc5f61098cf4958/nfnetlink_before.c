	    skb->len < nlh->nlmsg_len)
		return;

	if (!ns_capable(net->user_ns, CAP_NET_ADMIN)) {
		netlink_ack(skb, nlh, -EPERM);
		return;
	}
