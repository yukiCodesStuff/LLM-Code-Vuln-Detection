	return af;
}

static void sctp_auto_asconf_init(struct sctp_sock *sp)
{
	struct net *net = sock_net(&sp->inet.sk);

	if (net->sctp.default_auto_asconf) {
		spin_lock(&net->sctp.addr_wq_lock);
		list_add_tail(&sp->auto_asconf_list, &net->sctp.auto_asconf_splist);
		spin_unlock(&net->sctp.addr_wq_lock);
		sp->do_auto_asconf = 1;
	}
}

/* Bind a local address either to an endpoint or to an association.  */
static int sctp_do_bind(struct sock *sk, union sctp_addr *addr, int len)
{
	struct net *net = sock_net(sk);
		return -EADDRINUSE;

	/* Refresh ephemeral port.  */
	if (!bp->port) {
		bp->port = inet_sk(sk)->inet_num;
		sctp_auto_asconf_init(sp);
	}

	/* Add the address to the bind address list.
	 * Use GFP_ATOMIC since BHs will be disabled.
	 */
	sk_sockets_allocated_inc(sk);
	sock_prot_inuse_add(net, sk->sk_prot, 1);

	local_bh_enable();

	return 0;
}
			return err;
	}

	sctp_auto_asconf_init(newsp);

	/* Move any messages in the old socket's receive queue that are for the
	 * peeled off association to the new socket's receive queue.
	 */
	sctp_skb_for_each(skb, &oldsk->sk_receive_queue, tmp) {