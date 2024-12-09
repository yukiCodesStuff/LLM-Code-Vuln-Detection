		if (UDP_SKB_CB(skb)->cscov  <  up->pcrlen) {
			net_dbg_ratelimited("UDPLITE6: coverage %d too small, need min %d\n",
					    UDP_SKB_CB(skb)->cscov, up->pcrlen);
			goto drop;
		}
	}

	if (rcu_access_pointer(sk->sk_filter) &&
	    udp_lib_checksum_complete(skb))
		goto csum_error;

	if (sk_filter(sk, skb))
		goto drop;

	udp_csum_pull_header(skb);
	if (sk_rcvqueues_full(sk, sk->sk_rcvbuf)) {
		__UDP6_INC_STATS(sock_net(sk),
				 UDP_MIB_RCVBUFERRORS, is_udplite);
		goto drop;
	}