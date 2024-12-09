
		if (code == ICMP_FRAG_NEEDED) { /* PMTU discovery (RFC1191) */
			tp->mtu_info = info;
			if (!sock_owned_by_user(sk))
				tcp_v4_mtu_reduced(sk);
			else
				set_bit(TCP_MTU_REDUCED_DEFERRED, &tp->tsq_flags);
			goto out;
		}

		err = icmp_err_convert[code].errno;
		goto exit_nonewsk;

	newsk->sk_gso_type = SKB_GSO_TCPV4;

	newtp		      = tcp_sk(newsk);
	newinet		      = inet_sk(newsk);
	ireq		      = inet_rsk(req);