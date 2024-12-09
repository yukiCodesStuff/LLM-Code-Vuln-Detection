		release_sock(sk);
	}

	/* racing with another thread binding seems ok here */
	if (daddr == 0 || rs->rs_bound_addr == 0) {
		ret = -ENOTCONN; /* XXX not a great errno */
		goto out;
	}

	if (payload_len > rds_sk_sndbuf(rs)) {
		ret = -EMSGSIZE;
		goto out;