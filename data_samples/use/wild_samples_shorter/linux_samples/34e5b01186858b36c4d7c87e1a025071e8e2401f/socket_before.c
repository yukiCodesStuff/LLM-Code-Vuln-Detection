	return af;
}

/* Bind a local address either to an endpoint or to an association.  */
static int sctp_do_bind(struct sock *sk, union sctp_addr *addr, int len)
{
	struct net *net = sock_net(sk);
		return -EADDRINUSE;

	/* Refresh ephemeral port.  */
	if (!bp->port)
		bp->port = inet_sk(sk)->inet_num;

	/* Add the address to the bind address list.
	 * Use GFP_ATOMIC since BHs will be disabled.
	 */
	sk_sockets_allocated_inc(sk);
	sock_prot_inuse_add(net, sk->sk_prot, 1);

	/* Nothing can fail after this block, otherwise
	 * sctp_destroy_sock() will be called without addr_wq_lock held
	 */
	if (net->sctp.default_auto_asconf) {
		spin_lock(&sock_net(sk)->sctp.addr_wq_lock);
		list_add_tail(&sp->auto_asconf_list,
		    &net->sctp.auto_asconf_splist);
		sp->do_auto_asconf = 1;
		spin_unlock(&sock_net(sk)->sctp.addr_wq_lock);
	} else {
		sp->do_auto_asconf = 0;
	}

	local_bh_enable();

	return 0;
}
			return err;
	}

	/* Move any messages in the old socket's receive queue that are for the
	 * peeled off association to the new socket's receive queue.
	 */
	sctp_skb_for_each(skb, &oldsk->sk_receive_queue, tmp) {