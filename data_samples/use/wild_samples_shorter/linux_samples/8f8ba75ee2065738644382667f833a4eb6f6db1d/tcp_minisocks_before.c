		struct tcp_sock *oldtp = tcp_sk(sk);
		struct tcp_cookie_values *oldcvp = oldtp->cookie_values;

		newicsk->icsk_af_ops->sk_rx_dst_set(newsk, skb);

		/* TCP Cookie Transactions require space for the cookie pair,
		 * as it differs for each connection.  There is no need to
		 * copy any s_data_payload stored at the original socket.
		 * Failure will prevent resuming the connection.