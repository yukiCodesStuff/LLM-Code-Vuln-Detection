{
	struct sock *sk;
	struct irda_sock *self;

	if (protocol < 0 || protocol > SK_PROTOCOL_MAX)
		return -EINVAL;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	/* Check for valid socket type */
	switch (sock->type) {
	case SOCK_STREAM:     /* For TTP connections with SAR disabled */
	case SOCK_SEQPACKET:  /* For TTP connections with SAR enabled */
	case SOCK_DGRAM:      /* For TTP Unitdata or LMP Ultra transfers */
		break;
	default:
		return -ESOCKTNOSUPPORT;
	}