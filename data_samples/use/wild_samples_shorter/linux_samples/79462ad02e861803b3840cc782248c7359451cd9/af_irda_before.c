	struct sock *sk;
	struct irda_sock *self;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	/* Check for valid socket type */