
static int sk_diag_fill(struct sock *sk, struct sk_buff *skb,
			struct packet_diag_req *req,
			struct user_namespace *user_ns,
			u32 portid, u32 seq, u32 flags, int sk_ino)
{
	struct nlmsghdr *nlh;
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_FILTER) &&
	    sock_diag_put_filterinfo(sk, skb, PACKET_DIAG_FILTER))
		goto out_nlmsg_trim;

	return nlmsg_end(skb, nlh);

	struct packet_diag_req *req;
	struct net *net;
	struct sock *sk;

	net = sock_net(skb->sk);
	req = nlmsg_data(cb->nlh);

	mutex_lock(&net->packet.sklist_lock);
	sk_for_each(sk, &net->packet.sklist) {
		if (!net_eq(sock_net(sk), net))
			goto next;

		if (sk_diag_fill(sk, skb, req,
				 sk_user_ns(NETLINK_CB(cb->skb).sk),
				 NETLINK_CB(cb->skb).portid,
				 cb->nlh->nlmsg_seq, NLM_F_MULTI,
				 sock_i_ino(sk)) < 0)