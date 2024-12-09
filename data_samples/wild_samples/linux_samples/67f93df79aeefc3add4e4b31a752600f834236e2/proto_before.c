	if (dccp_qpolicy_full(sk)) {
		rc = -EAGAIN;
		goto out_release;
	}

	timeo = sock_sndtimeo(sk, noblock);

	/*
	 * We have to use sk_stream_wait_connect here to set sk_write_pending,
	 * so that the trick in dccp_rcv_request_sent_state_process.
	 */
	/* Wait for a connection to finish. */
	if ((1 << sk->sk_state) & ~(DCCPF_OPEN | DCCPF_PARTOPEN))
		if ((rc = sk_stream_wait_connect(sk, &timeo)) != 0)
			goto out_release;

	size = sk->sk_prot->max_header + len;
	release_sock(sk);
	skb = sock_alloc_send_skb(sk, size, noblock, &rc);
	lock_sock(sk);
	if (skb == NULL)
		goto out_release;

	skb_reserve(skb, sk->sk_prot->max_header);
	rc = memcpy_from_msg(skb_put(skb, len), msg, len);
	if (rc != 0)
		goto out_discard;

	rc = dccp_msghdr_parse(msg, skb);
	if (rc != 0)
		goto out_discard;

	dccp_qpolicy_push(sk, skb);
	/*
	 * The xmit_timer is set if the TX CCID is rate-based and will expire
	 * when congestion control permits to release further packets into the
	 * network. Window-based CCIDs do not use this timer.
	 */
	if (!timer_pending(&dp->dccps_xmit_timer))
		dccp_write_xmit(sk);
out_release:
	release_sock(sk);
	return rc ? : len;
out_discard:
	kfree_skb(skb);
	goto out_release;
}

EXPORT_SYMBOL_GPL(dccp_sendmsg);

int dccp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
		 int flags, int *addr_len)
{
	const struct dccp_hdr *dh;
	long timeo;

	lock_sock(sk);

	if (sk->sk_state == DCCP_LISTEN) {
		len = -ENOTCONN;
		goto out;
	}