	struct tcp_sock *tp = tcp_sk(sk);
	struct sk_buff *buff;
	int nsize, old_factor;
	int nlen;
	u8 flags;

	if (WARN_ON(len > skb->len))
	if (nsize < 0)
		nsize = 0;

	if (unlikely((sk->sk_wmem_queued >> 1) > sk->sk_sndbuf &&
		     tcp_queue != TCP_FRAG_IN_WRITE_QUEUE)) {
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPWQUEUETOOBIG);
		return -ENOMEM;
	}
