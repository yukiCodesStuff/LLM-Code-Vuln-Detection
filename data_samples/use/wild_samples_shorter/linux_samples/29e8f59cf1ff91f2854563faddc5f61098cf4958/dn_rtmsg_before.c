	if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
		return;

	if (!capable(CAP_NET_ADMIN))
		RCV_SKB_FAIL(-EPERM);

	/* Eventually we might send routing messages too */
