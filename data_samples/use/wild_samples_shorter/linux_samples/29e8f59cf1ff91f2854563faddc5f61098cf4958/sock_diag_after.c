}
EXPORT_SYMBOL_GPL(sock_diag_put_meminfo);

int sock_diag_put_filterinfo(bool may_report_filterinfo, struct sock *sk,
			     struct sk_buff *skb, int attrtype)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	unsigned int flen;
	int err = 0;

	if (!may_report_filterinfo) {
		nla_reserve(skb, attrtype, 0);
		return 0;
	}
