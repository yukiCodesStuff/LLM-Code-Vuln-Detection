	if (last && del_timer(&last->timeout)) {
		nf_ct_unlink_expect(last);
		nf_ct_expect_put(last);
	}
}

static inline int refresh_timer(struct nf_conntrack_expect *i)
{
	struct nf_conn_help *master_help = nfct_help(i->master);
	const struct nf_conntrack_expect_policy *p;

	if (!del_timer(&i->timeout))
		return 0;

	p = &rcu_dereference_protected(
		master_help->helper,
		lockdep_is_held(&nf_conntrack_lock)
		)->expect_policy[i->class];
	i->timeout.expires = jiffies + p->timeout * HZ;
	add_timer(&i->timeout);
	return 1;
}

static inline int __nf_ct_expect_check(struct nf_conntrack_expect *expect)
{
	const struct nf_conntrack_expect_policy *p;
	struct nf_conntrack_expect *i;
	struct nf_conn *master = expect->master;
	struct nf_conn_help *master_help = nfct_help(master);
	struct nf_conntrack_helper *helper;
	struct net *net = nf_ct_exp_net(expect);
	struct hlist_node *n;
	unsigned int h;
	int ret = 1;

	if (!master_help) {
		ret = -ESHUTDOWN;
		goto out;
	}
{
	const struct nf_conntrack_expect_policy *p;
	struct nf_conntrack_expect *i;
	struct nf_conn *master = expect->master;
	struct nf_conn_help *master_help = nfct_help(master);
	struct nf_conntrack_helper *helper;
	struct net *net = nf_ct_exp_net(expect);
	struct hlist_node *n;
	unsigned int h;
	int ret = 1;

	if (!master_help) {
		ret = -ESHUTDOWN;
		goto out;
	}
	if (!master_help) {
		ret = -ESHUTDOWN;
		goto out;
	}
	h = nf_ct_expect_dst_hash(&expect->tuple);
	hlist_for_each_entry(i, n, &net->ct.expect_hash[h], hnode) {
		if (expect_matches(i, expect)) {
			/* Refresh timer: if it's dying, ignore.. */
			if (refresh_timer(i)) {
				ret = 0;
				goto out;
			}
		} else if (expect_clash(i, expect)) {
			ret = -EBUSY;
			goto out;
		}
	}