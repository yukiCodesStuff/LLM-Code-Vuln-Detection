	type -= CRYPTO_MSG_BASE;
	link = &crypto_dispatch[type];

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if ((type == (CRYPTO_MSG_GETALG - CRYPTO_MSG_BASE) &&
	    (nlh->nlmsg_flags & NLM_F_DUMP))) {