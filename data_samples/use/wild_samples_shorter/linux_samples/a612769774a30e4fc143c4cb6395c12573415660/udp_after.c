
	if (sk_filter(sk, skb))
		goto drop;
	if (unlikely(skb->len < sizeof(struct udphdr)))
		goto drop;

	udp_csum_pull_header(skb);
	if (sk_rcvqueues_full(sk, sk->sk_rcvbuf)) {
		__UDP6_INC_STATS(sock_net(sk),