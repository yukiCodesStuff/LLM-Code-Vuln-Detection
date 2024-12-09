	if (nsize < 0)
		nsize = 0;

	if (unlikely((sk->sk_wmem_queued >> 1) > sk->sk_sndbuf)) {
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPWQUEUETOOBIG);
		return -ENOMEM;
	}

	if (skb_unclone(skb, gfp))
		return -ENOMEM;

	/* Get a new skb... force flag on. */