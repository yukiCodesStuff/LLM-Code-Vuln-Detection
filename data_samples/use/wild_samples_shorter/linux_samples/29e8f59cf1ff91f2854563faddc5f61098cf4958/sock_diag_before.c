}
EXPORT_SYMBOL_GPL(sock_diag_put_meminfo);

int sock_diag_put_filterinfo(struct sock *sk,
			     struct sk_buff *skb, int attrtype)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	unsigned int flen;
	int err = 0;

	if (!ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN)) {
		nla_reserve(skb, attrtype, 0);
		return 0;
	}
