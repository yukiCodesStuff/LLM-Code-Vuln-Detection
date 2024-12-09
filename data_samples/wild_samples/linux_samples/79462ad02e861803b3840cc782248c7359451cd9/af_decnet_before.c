{
	struct sock *sk;

	if (!net_eq(net, &init_net))
		return -EAFNOSUPPORT;

	switch (sock->type) {
	case SOCK_SEQPACKET:
		if (protocol != DNPROTO_NSP)
			return -EPROTONOSUPPORT;
		break;
	case SOCK_STREAM:
		break;
	default:
		return -ESOCKTNOSUPPORT;
	}