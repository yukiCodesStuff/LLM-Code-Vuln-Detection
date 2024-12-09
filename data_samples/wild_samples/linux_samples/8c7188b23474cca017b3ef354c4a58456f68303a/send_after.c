	} else {
		/* We only care about consistency with ->connect() */
		lock_sock(sk);
		daddr = rs->rs_conn_addr;
		dport = rs->rs_conn_port;
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
	}

	/* size of rm including all sgs */
	ret = rds_rm_size(msg, payload_len);
	if (ret < 0)
		goto out;

	rm = rds_message_alloc(ret, GFP_KERNEL);
	if (!rm) {
		ret = -ENOMEM;
		goto out;
	}

	/* Attach data to the rm */
	if (payload_len) {
		rm->data.op_sg = rds_message_alloc_sgs(rm, ceil(payload_len, PAGE_SIZE));
		if (!rm->data.op_sg) {
			ret = -ENOMEM;
			goto out;
		}