	struct sock *sk;
	ax25_cb *ax25;

	if (!net_eq(net, &init_net))
		return -EAFNOSUPPORT;

	switch (sock->type) {