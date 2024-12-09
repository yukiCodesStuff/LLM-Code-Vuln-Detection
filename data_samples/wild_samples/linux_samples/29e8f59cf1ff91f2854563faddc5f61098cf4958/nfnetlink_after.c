{
	struct nlmsghdr *nlh = nlmsg_hdr(skb);
	struct net *net = sock_net(skb->sk);
	int msglen;

	if (nlh->nlmsg_len < NLMSG_HDRLEN ||
	    skb->len < nlh->nlmsg_len)
		return;

	if (!netlink_net_capable(skb, CAP_NET_ADMIN)) {
		netlink_ack(skb, nlh, -EPERM);
		return;
	}