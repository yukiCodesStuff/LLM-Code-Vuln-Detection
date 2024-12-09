		release_sock(sk);
	}

	lock_sock(sk);
	if (daddr == 0 || rs->rs_bound_addr == 0) {
		release_sock(sk);
		ret = -ENOTCONN; /* XXX not a great errno */
		goto out;
	}
	release_sock(sk);

	if (payload_len > rds_sk_sndbuf(rs)) {
		ret = -EMSGSIZE;
		goto out;