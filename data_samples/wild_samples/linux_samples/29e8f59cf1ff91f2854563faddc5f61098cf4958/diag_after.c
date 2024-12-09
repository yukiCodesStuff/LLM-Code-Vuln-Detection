	if (po->fanout) {
		u32 val;

		val = (u32)po->fanout->id | ((u32)po->fanout->type << 16);
		ret = nla_put_u32(nlskb, PACKET_DIAG_FANOUT, val);
	}
	mutex_unlock(&fanout_mutex);

	return ret;
}

static int sk_diag_fill(struct sock *sk, struct sk_buff *skb,
			struct packet_diag_req *req,
			bool may_report_filterinfo,
			struct user_namespace *user_ns,
			u32 portid, u32 seq, u32 flags, int sk_ino)
{
	struct nlmsghdr *nlh;
	struct packet_diag_msg *rp;
	struct packet_sock *po = pkt_sk(sk);

	nlh = nlmsg_put(skb, portid, seq, SOCK_DIAG_BY_FAMILY, sizeof(*rp), flags);
	if (!nlh)
		return -EMSGSIZE;

	rp = nlmsg_data(nlh);
	rp->pdiag_family = AF_PACKET;
	rp->pdiag_type = sk->sk_type;
	rp->pdiag_num = ntohs(po->num);
	rp->pdiag_ino = sk_ino;
	sock_diag_save_cookie(sk, rp->pdiag_cookie);

	if ((req->pdiag_show & PACKET_SHOW_INFO) &&
			pdiag_put_info(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_INFO) &&
	    nla_put_u32(skb, PACKET_DIAG_UID,
			from_kuid_munged(user_ns, sock_i_uid(sk))))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_MCLIST) &&
			pdiag_put_mclist(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_RING_CFG) &&
			pdiag_put_rings_cfg(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_FANOUT) &&
			pdiag_put_fanout(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_MEMINFO) &&
	    sock_diag_put_meminfo(sk, skb, PACKET_DIAG_MEMINFO))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_FILTER) &&
	    sock_diag_put_filterinfo(may_report_filterinfo, sk, skb,
				     PACKET_DIAG_FILTER))
		goto out_nlmsg_trim;

	return nlmsg_end(skb, nlh);

out_nlmsg_trim:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}
{
	struct nlmsghdr *nlh;
	struct packet_diag_msg *rp;
	struct packet_sock *po = pkt_sk(sk);

	nlh = nlmsg_put(skb, portid, seq, SOCK_DIAG_BY_FAMILY, sizeof(*rp), flags);
	if (!nlh)
		return -EMSGSIZE;

	rp = nlmsg_data(nlh);
	rp->pdiag_family = AF_PACKET;
	rp->pdiag_type = sk->sk_type;
	rp->pdiag_num = ntohs(po->num);
	rp->pdiag_ino = sk_ino;
	sock_diag_save_cookie(sk, rp->pdiag_cookie);

	if ((req->pdiag_show & PACKET_SHOW_INFO) &&
			pdiag_put_info(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_INFO) &&
	    nla_put_u32(skb, PACKET_DIAG_UID,
			from_kuid_munged(user_ns, sock_i_uid(sk))))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_MCLIST) &&
			pdiag_put_mclist(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_RING_CFG) &&
			pdiag_put_rings_cfg(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_FANOUT) &&
			pdiag_put_fanout(po, skb))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_MEMINFO) &&
	    sock_diag_put_meminfo(sk, skb, PACKET_DIAG_MEMINFO))
		goto out_nlmsg_trim;

	if ((req->pdiag_show & PACKET_SHOW_FILTER) &&
	    sock_diag_put_filterinfo(may_report_filterinfo, sk, skb,
				     PACKET_DIAG_FILTER))
		goto out_nlmsg_trim;

	return nlmsg_end(skb, nlh);

out_nlmsg_trim:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}

static int packet_diag_dump(struct sk_buff *skb, struct netlink_callback *cb)
{
	int num = 0, s_num = cb->args[0];
	struct packet_diag_req *req;
	struct net *net;
	struct sock *sk;
	bool may_report_filterinfo;

	net = sock_net(skb->sk);
	req = nlmsg_data(cb->nlh);
	may_report_filterinfo = netlink_net_capable(cb->skb, CAP_NET_ADMIN);

	mutex_lock(&net->packet.sklist_lock);
	sk_for_each(sk, &net->packet.sklist) {
		if (!net_eq(sock_net(sk), net))
			continue;
		if (num < s_num)
			goto next;

		if (sk_diag_fill(sk, skb, req,
				 may_report_filterinfo,
				 sk_user_ns(NETLINK_CB(cb->skb).sk),
				 NETLINK_CB(cb->skb).portid,
				 cb->nlh->nlmsg_seq, NLM_F_MULTI,
				 sock_i_ino(sk)) < 0)
			goto done;
next:
		num++;
	}
{
	int num = 0, s_num = cb->args[0];
	struct packet_diag_req *req;
	struct net *net;
	struct sock *sk;
	bool may_report_filterinfo;

	net = sock_net(skb->sk);
	req = nlmsg_data(cb->nlh);
	may_report_filterinfo = netlink_net_capable(cb->skb, CAP_NET_ADMIN);

	mutex_lock(&net->packet.sklist_lock);
	sk_for_each(sk, &net->packet.sklist) {
		if (!net_eq(sock_net(sk), net))
			continue;
		if (num < s_num)
			goto next;

		if (sk_diag_fill(sk, skb, req,
				 may_report_filterinfo,
				 sk_user_ns(NETLINK_CB(cb->skb).sk),
				 NETLINK_CB(cb->skb).portid,
				 cb->nlh->nlmsg_seq, NLM_F_MULTI,
				 sock_i_ino(sk)) < 0)
			goto done;
next:
		num++;
	}
	sk_for_each(sk, &net->packet.sklist) {
		if (!net_eq(sock_net(sk), net))
			continue;
		if (num < s_num)
			goto next;

		if (sk_diag_fill(sk, skb, req,
				 may_report_filterinfo,
				 sk_user_ns(NETLINK_CB(cb->skb).sk),
				 NETLINK_CB(cb->skb).portid,
				 cb->nlh->nlmsg_seq, NLM_F_MULTI,
				 sock_i_ino(sk)) < 0)
			goto done;
next:
		num++;
	}
done:
	mutex_unlock(&net->packet.sklist_lock);
	cb->args[0] = num;

	return skb->len;
}

static int packet_diag_handler_dump(struct sk_buff *skb, struct nlmsghdr *h)
{
	int hdrlen = sizeof(struct packet_diag_req);
	struct net *net = sock_net(skb->sk);
	struct packet_diag_req *req;

	if (nlmsg_len(h) < hdrlen)
		return -EINVAL;

	req = nlmsg_data(h);
	/* Make it possible to support protocol filtering later */
	if (req->sdiag_protocol)
		return -EINVAL;

	if (h->nlmsg_flags & NLM_F_DUMP) {
		struct netlink_dump_control c = {
			.dump = packet_diag_dump,
		};
		return netlink_dump_start(net->diag_nlsk, skb, h, &c);
	} else
		return -EOPNOTSUPP;
}