}
#endif

static void tcp_v6_hash(struct sock *sk)
{
	if (sk->sk_state != TCP_CLOSE) {
		if (inet_csk(sk)->icsk_af_ops == &ipv6_mapped) {

	newsk->sk_gso_type = SKB_GSO_TCPV6;
	__ip6_dst_store(newsk, dst, NULL, NULL);

	newtcp6sk = (struct tcp6_sock *)newsk;
	inet_sk(newsk)->pinet6 = &newtcp6sk->inet6;

	.twsk_destructor= tcp_twsk_destructor,
};

static void inet6_sk_rx_dst_set(struct sock *sk, const struct sk_buff *skb)
{
	struct dst_entry *dst = skb_dst(skb);
	const struct rt6_info *rt = (const struct rt6_info *)dst;

	dst_hold(dst);
	sk->sk_rx_dst = dst;
	inet_sk(sk)->rx_dst_ifindex = skb->skb_iif;
	if (rt->rt6i_node)
		inet6_sk(sk)->rx_dst_cookie = rt->rt6i_node->fn_sernum;
}

static const struct inet_connection_sock_af_ops ipv6_specific = {
	.queue_xmit	   = inet6_csk_xmit,
	.send_check	   = tcp_v6_send_check,
	.rebuild_header	   = inet6_sk_rebuild_header,