	struct sock *sk;
	struct irda_sock *self;

	if (protocol < 0 || protocol > SK_PROTOCOL_MAX)
		return -EINVAL;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	/* Check for valid socket type */