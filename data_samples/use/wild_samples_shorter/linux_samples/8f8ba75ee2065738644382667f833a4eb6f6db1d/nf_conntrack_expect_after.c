	}
}

static inline int __nf_ct_expect_check(struct nf_conntrack_expect *expect)
{
	const struct nf_conntrack_expect_policy *p;
	struct nf_conntrack_expect *i;
	struct nf_conn_help *master_help = nfct_help(master);
	struct nf_conntrack_helper *helper;
	struct net *net = nf_ct_exp_net(expect);
	struct hlist_node *n, *next;
	unsigned int h;
	int ret = 1;

	if (!master_help) {
		goto out;
	}
	h = nf_ct_expect_dst_hash(&expect->tuple);
	hlist_for_each_entry_safe(i, n, next, &net->ct.expect_hash[h], hnode) {
		if (expect_matches(i, expect)) {
			if (del_timer(&i->timeout)) {
				nf_ct_unlink_expect(i);
				nf_ct_expect_put(i);
				break;
			}
		} else if (expect_clash(i, expect)) {
			ret = -EBUSY;
			goto out;