	iph->ihl = 5;
	iph->tos = inet->tos;
	iph->frag_off = df;
	ip_select_ident(iph, &rt->dst, sk);
	iph->ttl = ttl;
	iph->protocol = sk->sk_protocol;
	ip_copy_addrs(iph, fl4);

	if (opt) {
		iph->ihl += opt->optlen>>2;
		ip_options_build(skb, opt, cork->addr, rt, 0);