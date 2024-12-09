{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm = nlmsg_data(n);
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	u32 clid;
	struct Qdisc *q = NULL;
	struct Qdisc *p = NULL;
	int err;

	if ((n->nlmsg_type != RTM_GETQDISC) && !capable(CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;

	clid = tcm->tcm_parent;
	if (clid) {
		if (clid != TC_H_ROOT) {
			if (TC_H_MAJ(clid) != TC_H_MAJ(TC_H_INGRESS)) {
				p = qdisc_lookup(dev, TC_H_MAJ(clid));
				if (!p)
					return -ENOENT;
				q = qdisc_leaf(p, clid);
			} else if (dev_ingress_queue(dev)) {
				q = dev_ingress_queue(dev)->qdisc_sleeping;
			}
		} else {
			q = dev->qdisc;
		}
		if (!q)
			return -ENOENT;

		if (tcm->tcm_handle && q->handle != tcm->tcm_handle)
			return -EINVAL;
	} else {
		q = qdisc_lookup(dev, tcm->tcm_handle);
		if (!q)
			return -ENOENT;
	}

	if (tca[TCA_KIND] && nla_strcmp(tca[TCA_KIND], q->ops->id))
		return -EINVAL;

	if (n->nlmsg_type == RTM_DELQDISC) {
		if (!clid)
			return -EINVAL;
		if (q->handle == 0)
			return -ENOENT;
		err = qdisc_graft(dev, p, skb, n, clid, NULL, q);
		if (err != 0)
			return err;
	} else {
		qdisc_notify(net, skb, n, clid, NULL, q);
	}
	return 0;
}

/*
 * Create/change qdisc.
 */

static int tc_modify_qdisc(struct sk_buff *skb, struct nlmsghdr *n)
{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm;
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	u32 clid;
	struct Qdisc *q, *p;
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

replay:
	/* Reinit, just in case something touches this. */
	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	tcm = nlmsg_data(n);
	clid = tcm->tcm_parent;
	q = p = NULL;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;


	if (clid) {
		if (clid != TC_H_ROOT) {
			if (clid != TC_H_INGRESS) {
				p = qdisc_lookup(dev, TC_H_MAJ(clid));
				if (!p)
					return -ENOENT;
				q = qdisc_leaf(p, clid);
			} else if (dev_ingress_queue_create(dev)) {
				q = dev_ingress_queue(dev)->qdisc_sleeping;
			}
		} else {
			q = dev->qdisc;
		}

		/* It may be default qdisc, ignore it */
		if (q && q->handle == 0)
			q = NULL;

		if (!q || !tcm->tcm_handle || q->handle != tcm->tcm_handle) {
			if (tcm->tcm_handle) {
				if (q && !(n->nlmsg_flags & NLM_F_REPLACE))
					return -EEXIST;
				if (TC_H_MIN(tcm->tcm_handle))
					return -EINVAL;
				q = qdisc_lookup(dev, tcm->tcm_handle);
				if (!q)
					goto create_n_graft;
				if (n->nlmsg_flags & NLM_F_EXCL)
					return -EEXIST;
				if (tca[TCA_KIND] && nla_strcmp(tca[TCA_KIND], q->ops->id))
					return -EINVAL;
				if (q == p ||
				    (p && check_loop(q, p, 0)))
					return -ELOOP;
				atomic_inc(&q->refcnt);
				goto graft;
			} else {
				if (!q)
					goto create_n_graft;

				/* This magic test requires explanation.
				 *
				 *   We know, that some child q is already
				 *   attached to this parent and have choice:
				 *   either to change it or to create/graft new one.
				 *
				 *   1. We are allowed to create/graft only
				 *   if CREATE and REPLACE flags are set.
				 *
				 *   2. If EXCL is set, requestor wanted to say,
				 *   that qdisc tcm_handle is not expected
				 *   to exist, so that we choose create/graft too.
				 *
				 *   3. The last case is when no flags are set.
				 *   Alas, it is sort of hole in API, we
				 *   cannot decide what to do unambiguously.
				 *   For now we select create/graft, if
				 *   user gave KIND, which does not match existing.
				 */
				if ((n->nlmsg_flags & NLM_F_CREATE) &&
				    (n->nlmsg_flags & NLM_F_REPLACE) &&
				    ((n->nlmsg_flags & NLM_F_EXCL) ||
				     (tca[TCA_KIND] &&
				      nla_strcmp(tca[TCA_KIND], q->ops->id))))
					goto create_n_graft;
			}
		}
	} else {
		if (!tcm->tcm_handle)
			return -EINVAL;
		q = qdisc_lookup(dev, tcm->tcm_handle);
	}

	/* Change qdisc parameters */
	if (q == NULL)
		return -ENOENT;
	if (n->nlmsg_flags & NLM_F_EXCL)
		return -EEXIST;
	if (tca[TCA_KIND] && nla_strcmp(tca[TCA_KIND], q->ops->id))
		return -EINVAL;
	err = qdisc_change(q, tca);
	if (err == 0)
		qdisc_notify(net, skb, n, clid, NULL, q);
	return err;

create_n_graft:
	if (!(n->nlmsg_flags & NLM_F_CREATE))
		return -ENOENT;
	if (clid == TC_H_INGRESS) {
		if (dev_ingress_queue(dev))
			q = qdisc_create(dev, dev_ingress_queue(dev), p,
					 tcm->tcm_parent, tcm->tcm_parent,
					 tca, &err);
		else
			err = -ENOENT;
	} else {
		struct netdev_queue *dev_queue;

		if (p && p->ops->cl_ops && p->ops->cl_ops->select_queue)
			dev_queue = p->ops->cl_ops->select_queue(p, tcm);
		else if (p)
			dev_queue = p->dev_queue;
		else
			dev_queue = netdev_get_tx_queue(dev, 0);

		q = qdisc_create(dev, dev_queue, p,
				 tcm->tcm_parent, tcm->tcm_handle,
				 tca, &err);
	}
	if (q == NULL) {
		if (err == -EAGAIN)
			goto replay;
		return err;
	}

graft:
	err = qdisc_graft(dev, p, skb, n, clid, q, NULL);
	if (err) {
		if (q)
			qdisc_destroy(q);
		return err;
	}

	return 0;
}

static int tc_fill_qdisc(struct sk_buff *skb, struct Qdisc *q, u32 clid,
			 u32 portid, u32 seq, u16 flags, int event)
{
	struct tcmsg *tcm;
	struct nlmsghdr  *nlh;
	unsigned char *b = skb_tail_pointer(skb);
	struct gnet_dump d;
	struct qdisc_size_table *stab;

	cond_resched();
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*tcm), flags);
	if (!nlh)
		goto out_nlmsg_trim;
	tcm = nlmsg_data(nlh);
	tcm->tcm_family = AF_UNSPEC;
	tcm->tcm__pad1 = 0;
	tcm->tcm__pad2 = 0;
	tcm->tcm_ifindex = qdisc_dev(q)->ifindex;
	tcm->tcm_parent = clid;
	tcm->tcm_handle = q->handle;
	tcm->tcm_info = atomic_read(&q->refcnt);
	if (nla_put_string(skb, TCA_KIND, q->ops->id))
		goto nla_put_failure;
	if (q->ops->dump && q->ops->dump(q, skb) < 0)
		goto nla_put_failure;
	q->qstats.qlen = q->q.qlen;

	stab = rtnl_dereference(q->stab);
	if (stab && qdisc_dump_stab(skb, stab) < 0)
		goto nla_put_failure;

	if (gnet_stats_start_copy_compat(skb, TCA_STATS2, TCA_STATS, TCA_XSTATS,
					 qdisc_root_sleeping_lock(q), &d) < 0)
		goto nla_put_failure;

	if (q->ops->dump_stats && q->ops->dump_stats(q, &d) < 0)
		goto nla_put_failure;

	if (gnet_stats_copy_basic(&d, &q->bstats) < 0 ||
	    gnet_stats_copy_rate_est(&d, &q->bstats, &q->rate_est) < 0 ||
	    gnet_stats_copy_queue(&d, &q->qstats) < 0)
		goto nla_put_failure;

	if (gnet_stats_finish_copy(&d) < 0)
		goto nla_put_failure;

	nlh->nlmsg_len = skb_tail_pointer(skb) - b;
	return skb->len;

out_nlmsg_trim:
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static bool tc_qdisc_dump_ignore(struct Qdisc *q)
{
	return (q->flags & TCQ_F_BUILTIN) ? true : false;
}

static int qdisc_notify(struct net *net, struct sk_buff *oskb,
			struct nlmsghdr *n, u32 clid,
			struct Qdisc *old, struct Qdisc *new)
{
	struct sk_buff *skb;
	u32 portid = oskb ? NETLINK_CB(oskb).portid : 0;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOBUFS;

	if (old && !tc_qdisc_dump_ignore(old)) {
		if (tc_fill_qdisc(skb, old, clid, portid, n->nlmsg_seq,
				  0, RTM_DELQDISC) < 0)
			goto err_out;
	}
	if (new && !tc_qdisc_dump_ignore(new)) {
		if (tc_fill_qdisc(skb, new, clid, portid, n->nlmsg_seq,
				  old ? NLM_F_REPLACE : 0, RTM_NEWQDISC) < 0)
			goto err_out;
	}

	if (skb->len)
		return rtnetlink_send(skb, net, portid, RTNLGRP_TC,
				      n->nlmsg_flags & NLM_F_ECHO);

err_out:
	kfree_skb(skb);
	return -EINVAL;
}

static int tc_dump_qdisc_root(struct Qdisc *root, struct sk_buff *skb,
			      struct netlink_callback *cb,
			      int *q_idx_p, int s_q_idx)
{
	int ret = 0, q_idx = *q_idx_p;
	struct Qdisc *q;

	if (!root)
		return 0;

	q = root;
	if (q_idx < s_q_idx) {
		q_idx++;
	} else {
		if (!tc_qdisc_dump_ignore(q) &&
		    tc_fill_qdisc(skb, q, q->parent, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWQDISC) <= 0)
			goto done;
		q_idx++;
	}
	list_for_each_entry(q, &root->list, list) {
		if (q_idx < s_q_idx) {
			q_idx++;
			continue;
		}
		if (!tc_qdisc_dump_ignore(q) &&
		    tc_fill_qdisc(skb, q, q->parent, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWQDISC) <= 0)
			goto done;
		q_idx++;
	}

out:
	*q_idx_p = q_idx;
	return ret;
done:
	ret = -1;
	goto out;
}

static int tc_dump_qdisc(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct net *net = sock_net(skb->sk);
	int idx, q_idx;
	int s_idx, s_q_idx;
	struct net_device *dev;

	s_idx = cb->args[0];
	s_q_idx = q_idx = cb->args[1];

	idx = 0;
	ASSERT_RTNL();
	for_each_netdev(net, dev) {
		struct netdev_queue *dev_queue;

		if (idx < s_idx)
			goto cont;
		if (idx > s_idx)
			s_q_idx = 0;
		q_idx = 0;

		if (tc_dump_qdisc_root(dev->qdisc, skb, cb, &q_idx, s_q_idx) < 0)
			goto done;

		dev_queue = dev_ingress_queue(dev);
		if (dev_queue &&
		    tc_dump_qdisc_root(dev_queue->qdisc_sleeping, skb, cb,
				       &q_idx, s_q_idx) < 0)
			goto done;

cont:
		idx++;
	}

done:
	cb->args[0] = idx;
	cb->args[1] = q_idx;

	return skb->len;
}



/************************************************
 *	Traffic classes manipulation.		*
 ************************************************/



static int tc_ctl_tclass(struct sk_buff *skb, struct nlmsghdr *n)
{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm = nlmsg_data(n);
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	struct Qdisc *q = NULL;
	const struct Qdisc_class_ops *cops;
	unsigned long cl = 0;
	unsigned long new_cl;
	u32 portid;
	u32 clid;
	u32 qid;
	int err;

	if ((n->nlmsg_type != RTM_GETTCLASS) && !capable(CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;

	/*
	   parent == TC_H_UNSPEC - unspecified parent.
	   parent == TC_H_ROOT   - class is root, which has no parent.
	   parent == X:0	 - parent is root class.
	   parent == X:Y	 - parent is a node in hierarchy.
	   parent == 0:Y	 - parent is X:Y, where X:0 is qdisc.

	   handle == 0:0	 - generate handle from kernel pool.
	   handle == 0:Y	 - class is X:Y, where X:0 is qdisc.
	   handle == X:Y	 - clear.
	   handle == X:0	 - root class.
	 */

	/* Step 1. Determine qdisc handle X:0 */

	portid = tcm->tcm_parent;
	clid = tcm->tcm_handle;
	qid = TC_H_MAJ(clid);

	if (portid != TC_H_ROOT) {
		u32 qid1 = TC_H_MAJ(portid);

		if (qid && qid1) {
			/* If both majors are known, they must be identical. */
			if (qid != qid1)
				return -EINVAL;
		} else if (qid1) {
			qid = qid1;
		} else if (qid == 0)
			qid = dev->qdisc->handle;

		/* Now qid is genuine qdisc handle consistent
		 * both with parent and child.
		 *
		 * TC_H_MAJ(portid) still may be unspecified, complete it now.
		 */
		if (portid)
			portid = TC_H_MAKE(qid, portid);
	} else {
		if (qid == 0)
			qid = dev->qdisc->handle;
	}

	/* OK. Locate qdisc */
	q = qdisc_lookup(dev, qid);
	if (!q)
		return -ENOENT;

	/* An check that it supports classes */
	cops = q->ops->cl_ops;
	if (cops == NULL)
		return -EINVAL;

	/* Now try to get class */
	if (clid == 0) {
		if (portid == TC_H_ROOT)
			clid = qid;
	} else
		clid = TC_H_MAKE(qid, clid);

	if (clid)
		cl = cops->get(q, clid);

	if (cl == 0) {
		err = -ENOENT;
		if (n->nlmsg_type != RTM_NEWTCLASS ||
		    !(n->nlmsg_flags & NLM_F_CREATE))
			goto out;
	} else {
		switch (n->nlmsg_type) {
		case RTM_NEWTCLASS:
			err = -EEXIST;
			if (n->nlmsg_flags & NLM_F_EXCL)
				goto out;
			break;
		case RTM_DELTCLASS:
			err = -EOPNOTSUPP;
			if (cops->delete)
				err = cops->delete(q, cl);
			if (err == 0)
				tclass_notify(net, skb, n, q, cl, RTM_DELTCLASS);
			goto out;
		case RTM_GETTCLASS:
			err = tclass_notify(net, skb, n, q, cl, RTM_NEWTCLASS);
			goto out;
		default:
			err = -EINVAL;
			goto out;
		}
	}

	new_cl = cl;
	err = -EOPNOTSUPP;
	if (cops->change)
		err = cops->change(q, clid, portid, tca, &new_cl);
	if (err == 0)
		tclass_notify(net, skb, n, q, new_cl, RTM_NEWTCLASS);

out:
	if (cl)
		cops->put(q, cl);

	return err;
}


static int tc_fill_tclass(struct sk_buff *skb, struct Qdisc *q,
			  unsigned long cl,
			  u32 portid, u32 seq, u16 flags, int event)
{
	struct tcmsg *tcm;
	struct nlmsghdr  *nlh;
	unsigned char *b = skb_tail_pointer(skb);
	struct gnet_dump d;
	const struct Qdisc_class_ops *cl_ops = q->ops->cl_ops;

	cond_resched();
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*tcm), flags);
	if (!nlh)
		goto out_nlmsg_trim;
	tcm = nlmsg_data(nlh);
	tcm->tcm_family = AF_UNSPEC;
	tcm->tcm__pad1 = 0;
	tcm->tcm__pad2 = 0;
	tcm->tcm_ifindex = qdisc_dev(q)->ifindex;
	tcm->tcm_parent = q->handle;
	tcm->tcm_handle = q->handle;
	tcm->tcm_info = 0;
	if (nla_put_string(skb, TCA_KIND, q->ops->id))
		goto nla_put_failure;
	if (cl_ops->dump && cl_ops->dump(q, cl, skb, tcm) < 0)
		goto nla_put_failure;

	if (gnet_stats_start_copy_compat(skb, TCA_STATS2, TCA_STATS, TCA_XSTATS,
					 qdisc_root_sleeping_lock(q), &d) < 0)
		goto nla_put_failure;

	if (cl_ops->dump_stats && cl_ops->dump_stats(q, cl, &d) < 0)
		goto nla_put_failure;

	if (gnet_stats_finish_copy(&d) < 0)
		goto nla_put_failure;

	nlh->nlmsg_len = skb_tail_pointer(skb) - b;
	return skb->len;

out_nlmsg_trim:
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static int tclass_notify(struct net *net, struct sk_buff *oskb,
			 struct nlmsghdr *n, struct Qdisc *q,
			 unsigned long cl, int event)
{
	struct sk_buff *skb;
	u32 portid = oskb ? NETLINK_CB(oskb).portid : 0;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOBUFS;

	if (tc_fill_tclass(skb, q, cl, portid, n->nlmsg_seq, 0, event) < 0) {
		kfree_skb(skb);
		return -EINVAL;
	}

	return rtnetlink_send(skb, net, portid, RTNLGRP_TC,
			      n->nlmsg_flags & NLM_F_ECHO);
}

struct qdisc_dump_args {
	struct qdisc_walker	w;
	struct sk_buff		*skb;
	struct netlink_callback	*cb;
};

static int qdisc_class_dump(struct Qdisc *q, unsigned long cl, struct qdisc_walker *arg)
{
	struct qdisc_dump_args *a = (struct qdisc_dump_args *)arg;

	return tc_fill_tclass(a->skb, q, cl, NETLINK_CB(a->cb->skb).portid,
			      a->cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWTCLASS);
}

static int tc_dump_tclass_qdisc(struct Qdisc *q, struct sk_buff *skb,
				struct tcmsg *tcm, struct netlink_callback *cb,
				int *t_p, int s_t)
{
	struct qdisc_dump_args arg;

	if (tc_qdisc_dump_ignore(q) ||
	    *t_p < s_t || !q->ops->cl_ops ||
	    (tcm->tcm_parent &&
	     TC_H_MAJ(tcm->tcm_parent) != q->handle)) {
		(*t_p)++;
		return 0;
	}
	if (*t_p > s_t)
		memset(&cb->args[1], 0, sizeof(cb->args)-sizeof(cb->args[0]));
	arg.w.fn = qdisc_class_dump;
	arg.skb = skb;
	arg.cb = cb;
	arg.w.stop  = 0;
	arg.w.skip = cb->args[1];
	arg.w.count = 0;
	q->ops->cl_ops->walk(q, &arg.w);
	cb->args[1] = arg.w.count;
	if (arg.w.stop)
		return -1;
	(*t_p)++;
	return 0;
}

static int tc_dump_tclass_root(struct Qdisc *root, struct sk_buff *skb,
			       struct tcmsg *tcm, struct netlink_callback *cb,
			       int *t_p, int s_t)
{
	struct Qdisc *q;

	if (!root)
		return 0;

	if (tc_dump_tclass_qdisc(root, skb, tcm, cb, t_p, s_t) < 0)
		return -1;

	list_for_each_entry(q, &root->list, list) {
		if (tc_dump_tclass_qdisc(q, skb, tcm, cb, t_p, s_t) < 0)
			return -1;
	}

	return 0;
}

static int tc_dump_tclass(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct tcmsg *tcm = nlmsg_data(cb->nlh);
	struct net *net = sock_net(skb->sk);
	struct netdev_queue *dev_queue;
	struct net_device *dev;
	int t, s_t;

	if (nlmsg_len(cb->nlh) < sizeof(*tcm))
		return 0;
	dev = dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return 0;

	s_t = cb->args[0];
	t = 0;

	if (tc_dump_tclass_root(dev->qdisc, skb, tcm, cb, &t, s_t) < 0)
		goto done;

	dev_queue = dev_ingress_queue(dev);
	if (dev_queue &&
	    tc_dump_tclass_root(dev_queue->qdisc_sleeping, skb, tcm, cb,
				&t, s_t) < 0)
		goto done;

done:
	cb->args[0] = t;

	dev_put(dev);
	return skb->len;
}

/* Main classifier routine: scans classifier chain attached
 * to this qdisc, (optionally) tests for protocol and asks
 * specific classifiers.
 */
int tc_classify_compat(struct sk_buff *skb, const struct tcf_proto *tp,
		       struct tcf_result *res)
{
	__be16 protocol = skb->protocol;
	int err;

	for (; tp; tp = tp->next) {
		if (tp->protocol != protocol &&
		    tp->protocol != htons(ETH_P_ALL))
			continue;
		err = tp->classify(skb, tp, res);

		if (err >= 0) {
#ifdef CONFIG_NET_CLS_ACT
			if (err != TC_ACT_RECLASSIFY && skb->tc_verd)
				skb->tc_verd = SET_TC_VERD(skb->tc_verd, 0);
#endif
			return err;
		}
	}
	return -1;
}
EXPORT_SYMBOL(tc_classify_compat);

int tc_classify(struct sk_buff *skb, const struct tcf_proto *tp,
		struct tcf_result *res)
{
	int err = 0;
#ifdef CONFIG_NET_CLS_ACT
	const struct tcf_proto *otp = tp;
reclassify:
#endif

	err = tc_classify_compat(skb, tp, res);
#ifdef CONFIG_NET_CLS_ACT
	if (err == TC_ACT_RECLASSIFY) {
		u32 verd = G_TC_VERD(skb->tc_verd);
		tp = otp;

		if (verd++ >= MAX_REC_LOOP) {
			net_notice_ratelimited("%s: packet reclassify loop rule prio %u protocol %02x\n",
					       tp->q->ops->id,
					       tp->prio & 0xffff,
					       ntohs(tp->protocol));
			return TC_ACT_SHOT;
		}
		skb->tc_verd = SET_TC_VERD(skb->tc_verd, verd);
		goto reclassify;
	}
#endif
	return err;
}
EXPORT_SYMBOL(tc_classify);

void tcf_destroy(struct tcf_proto *tp)
{
	tp->ops->destroy(tp);
	module_put(tp->ops->owner);
	kfree(tp);
}

void tcf_destroy_chain(struct tcf_proto **fl)
{
	struct tcf_proto *tp;

	while ((tp = *fl) != NULL) {
		*fl = tp->next;
		tcf_destroy(tp);
	}
}
EXPORT_SYMBOL(tcf_destroy_chain);

#ifdef CONFIG_PROC_FS
static int psched_show(struct seq_file *seq, void *v)
{
	struct timespec ts;

	hrtimer_get_res(CLOCK_MONOTONIC, &ts);
	seq_printf(seq, "%08x %08x %08x %08x\n",
		   (u32)NSEC_PER_USEC, (u32)PSCHED_TICKS2NS(1),
		   1000000,
		   (u32)NSEC_PER_SEC/(u32)ktime_to_ns(timespec_to_ktime(ts)));

	return 0;
}

static int psched_open(struct inode *inode, struct file *file)
{
	return single_open(file, psched_show, NULL);
}

static const struct file_operations psched_fops = {
	.owner = THIS_MODULE,
	.open = psched_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __net_init psched_net_init(struct net *net)
{
	struct proc_dir_entry *e;

	e = proc_create("psched", 0, net->proc_net, &psched_fops);
	if (e == NULL)
		return -ENOMEM;

	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
	remove_proc_entry("psched", net->proc_net);
}
#else
static int __net_init psched_net_init(struct net *net)
{
	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
}
#endif

static struct pernet_operations psched_net_ops = {
	.init = psched_net_init,
	.exit = psched_net_exit,
};

static int __init pktsched_init(void)
{
	int err;

	err = register_pernet_subsys(&psched_net_ops);
	if (err) {
		pr_err("pktsched_init: "
		       "cannot initialize per netns operations\n");
		return err;
	}

	register_qdisc(&pfifo_fast_ops);
	register_qdisc(&pfifo_qdisc_ops);
	register_qdisc(&bfifo_qdisc_ops);
	register_qdisc(&pfifo_head_drop_qdisc_ops);
	register_qdisc(&mq_qdisc_ops);

	rtnl_register(PF_UNSPEC, RTM_NEWQDISC, tc_modify_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELQDISC, tc_get_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETQDISC, tc_get_qdisc, tc_dump_qdisc, NULL);
	rtnl_register(PF_UNSPEC, RTM_NEWTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETTCLASS, tc_ctl_tclass, tc_dump_tclass, NULL);

	return 0;
}

subsys_initcall(pktsched_init);
{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm;
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	u32 clid;
	struct Qdisc *q, *p;
	int err;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

replay:
	/* Reinit, just in case something touches this. */
	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	tcm = nlmsg_data(n);
	clid = tcm->tcm_parent;
	q = p = NULL;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;


	if (clid) {
		if (clid != TC_H_ROOT) {
			if (clid != TC_H_INGRESS) {
				p = qdisc_lookup(dev, TC_H_MAJ(clid));
				if (!p)
					return -ENOENT;
				q = qdisc_leaf(p, clid);
			} else if (dev_ingress_queue_create(dev)) {
				q = dev_ingress_queue(dev)->qdisc_sleeping;
			}
		} else {
			q = dev->qdisc;
		}

		/* It may be default qdisc, ignore it */
		if (q && q->handle == 0)
			q = NULL;

		if (!q || !tcm->tcm_handle || q->handle != tcm->tcm_handle) {
			if (tcm->tcm_handle) {
				if (q && !(n->nlmsg_flags & NLM_F_REPLACE))
					return -EEXIST;
				if (TC_H_MIN(tcm->tcm_handle))
					return -EINVAL;
				q = qdisc_lookup(dev, tcm->tcm_handle);
				if (!q)
					goto create_n_graft;
				if (n->nlmsg_flags & NLM_F_EXCL)
					return -EEXIST;
				if (tca[TCA_KIND] && nla_strcmp(tca[TCA_KIND], q->ops->id))
					return -EINVAL;
				if (q == p ||
				    (p && check_loop(q, p, 0)))
					return -ELOOP;
				atomic_inc(&q->refcnt);
				goto graft;
			} else {
				if (!q)
					goto create_n_graft;

				/* This magic test requires explanation.
				 *
				 *   We know, that some child q is already
				 *   attached to this parent and have choice:
				 *   either to change it or to create/graft new one.
				 *
				 *   1. We are allowed to create/graft only
				 *   if CREATE and REPLACE flags are set.
				 *
				 *   2. If EXCL is set, requestor wanted to say,
				 *   that qdisc tcm_handle is not expected
				 *   to exist, so that we choose create/graft too.
				 *
				 *   3. The last case is when no flags are set.
				 *   Alas, it is sort of hole in API, we
				 *   cannot decide what to do unambiguously.
				 *   For now we select create/graft, if
				 *   user gave KIND, which does not match existing.
				 */
				if ((n->nlmsg_flags & NLM_F_CREATE) &&
				    (n->nlmsg_flags & NLM_F_REPLACE) &&
				    ((n->nlmsg_flags & NLM_F_EXCL) ||
				     (tca[TCA_KIND] &&
				      nla_strcmp(tca[TCA_KIND], q->ops->id))))
					goto create_n_graft;
			}
		}
	} else {
		if (!tcm->tcm_handle)
			return -EINVAL;
		q = qdisc_lookup(dev, tcm->tcm_handle);
	}

	/* Change qdisc parameters */
	if (q == NULL)
		return -ENOENT;
	if (n->nlmsg_flags & NLM_F_EXCL)
		return -EEXIST;
	if (tca[TCA_KIND] && nla_strcmp(tca[TCA_KIND], q->ops->id))
		return -EINVAL;
	err = qdisc_change(q, tca);
	if (err == 0)
		qdisc_notify(net, skb, n, clid, NULL, q);
	return err;

create_n_graft:
	if (!(n->nlmsg_flags & NLM_F_CREATE))
		return -ENOENT;
	if (clid == TC_H_INGRESS) {
		if (dev_ingress_queue(dev))
			q = qdisc_create(dev, dev_ingress_queue(dev), p,
					 tcm->tcm_parent, tcm->tcm_parent,
					 tca, &err);
		else
			err = -ENOENT;
	} else {
		struct netdev_queue *dev_queue;

		if (p && p->ops->cl_ops && p->ops->cl_ops->select_queue)
			dev_queue = p->ops->cl_ops->select_queue(p, tcm);
		else if (p)
			dev_queue = p->dev_queue;
		else
			dev_queue = netdev_get_tx_queue(dev, 0);

		q = qdisc_create(dev, dev_queue, p,
				 tcm->tcm_parent, tcm->tcm_handle,
				 tca, &err);
	}
	if (q == NULL) {
		if (err == -EAGAIN)
			goto replay;
		return err;
	}

graft:
	err = qdisc_graft(dev, p, skb, n, clid, q, NULL);
	if (err) {
		if (q)
			qdisc_destroy(q);
		return err;
	}

	return 0;
}

static int tc_fill_qdisc(struct sk_buff *skb, struct Qdisc *q, u32 clid,
			 u32 portid, u32 seq, u16 flags, int event)
{
	struct tcmsg *tcm;
	struct nlmsghdr  *nlh;
	unsigned char *b = skb_tail_pointer(skb);
	struct gnet_dump d;
	struct qdisc_size_table *stab;

	cond_resched();
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*tcm), flags);
	if (!nlh)
		goto out_nlmsg_trim;
	tcm = nlmsg_data(nlh);
	tcm->tcm_family = AF_UNSPEC;
	tcm->tcm__pad1 = 0;
	tcm->tcm__pad2 = 0;
	tcm->tcm_ifindex = qdisc_dev(q)->ifindex;
	tcm->tcm_parent = clid;
	tcm->tcm_handle = q->handle;
	tcm->tcm_info = atomic_read(&q->refcnt);
	if (nla_put_string(skb, TCA_KIND, q->ops->id))
		goto nla_put_failure;
	if (q->ops->dump && q->ops->dump(q, skb) < 0)
		goto nla_put_failure;
	q->qstats.qlen = q->q.qlen;

	stab = rtnl_dereference(q->stab);
	if (stab && qdisc_dump_stab(skb, stab) < 0)
		goto nla_put_failure;

	if (gnet_stats_start_copy_compat(skb, TCA_STATS2, TCA_STATS, TCA_XSTATS,
					 qdisc_root_sleeping_lock(q), &d) < 0)
		goto nla_put_failure;

	if (q->ops->dump_stats && q->ops->dump_stats(q, &d) < 0)
		goto nla_put_failure;

	if (gnet_stats_copy_basic(&d, &q->bstats) < 0 ||
	    gnet_stats_copy_rate_est(&d, &q->bstats, &q->rate_est) < 0 ||
	    gnet_stats_copy_queue(&d, &q->qstats) < 0)
		goto nla_put_failure;

	if (gnet_stats_finish_copy(&d) < 0)
		goto nla_put_failure;

	nlh->nlmsg_len = skb_tail_pointer(skb) - b;
	return skb->len;

out_nlmsg_trim:
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static bool tc_qdisc_dump_ignore(struct Qdisc *q)
{
	return (q->flags & TCQ_F_BUILTIN) ? true : false;
}

static int qdisc_notify(struct net *net, struct sk_buff *oskb,
			struct nlmsghdr *n, u32 clid,
			struct Qdisc *old, struct Qdisc *new)
{
	struct sk_buff *skb;
	u32 portid = oskb ? NETLINK_CB(oskb).portid : 0;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOBUFS;

	if (old && !tc_qdisc_dump_ignore(old)) {
		if (tc_fill_qdisc(skb, old, clid, portid, n->nlmsg_seq,
				  0, RTM_DELQDISC) < 0)
			goto err_out;
	}
	if (new && !tc_qdisc_dump_ignore(new)) {
		if (tc_fill_qdisc(skb, new, clid, portid, n->nlmsg_seq,
				  old ? NLM_F_REPLACE : 0, RTM_NEWQDISC) < 0)
			goto err_out;
	}

	if (skb->len)
		return rtnetlink_send(skb, net, portid, RTNLGRP_TC,
				      n->nlmsg_flags & NLM_F_ECHO);

err_out:
	kfree_skb(skb);
	return -EINVAL;
}

static int tc_dump_qdisc_root(struct Qdisc *root, struct sk_buff *skb,
			      struct netlink_callback *cb,
			      int *q_idx_p, int s_q_idx)
{
	int ret = 0, q_idx = *q_idx_p;
	struct Qdisc *q;

	if (!root)
		return 0;

	q = root;
	if (q_idx < s_q_idx) {
		q_idx++;
	} else {
		if (!tc_qdisc_dump_ignore(q) &&
		    tc_fill_qdisc(skb, q, q->parent, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWQDISC) <= 0)
			goto done;
		q_idx++;
	}
	list_for_each_entry(q, &root->list, list) {
		if (q_idx < s_q_idx) {
			q_idx++;
			continue;
		}
		if (!tc_qdisc_dump_ignore(q) &&
		    tc_fill_qdisc(skb, q, q->parent, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWQDISC) <= 0)
			goto done;
		q_idx++;
	}

out:
	*q_idx_p = q_idx;
	return ret;
done:
	ret = -1;
	goto out;
}

static int tc_dump_qdisc(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct net *net = sock_net(skb->sk);
	int idx, q_idx;
	int s_idx, s_q_idx;
	struct net_device *dev;

	s_idx = cb->args[0];
	s_q_idx = q_idx = cb->args[1];

	idx = 0;
	ASSERT_RTNL();
	for_each_netdev(net, dev) {
		struct netdev_queue *dev_queue;

		if (idx < s_idx)
			goto cont;
		if (idx > s_idx)
			s_q_idx = 0;
		q_idx = 0;

		if (tc_dump_qdisc_root(dev->qdisc, skb, cb, &q_idx, s_q_idx) < 0)
			goto done;

		dev_queue = dev_ingress_queue(dev);
		if (dev_queue &&
		    tc_dump_qdisc_root(dev_queue->qdisc_sleeping, skb, cb,
				       &q_idx, s_q_idx) < 0)
			goto done;

cont:
		idx++;
	}

done:
	cb->args[0] = idx;
	cb->args[1] = q_idx;

	return skb->len;
}



/************************************************
 *	Traffic classes manipulation.		*
 ************************************************/



static int tc_ctl_tclass(struct sk_buff *skb, struct nlmsghdr *n)
{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm = nlmsg_data(n);
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	struct Qdisc *q = NULL;
	const struct Qdisc_class_ops *cops;
	unsigned long cl = 0;
	unsigned long new_cl;
	u32 portid;
	u32 clid;
	u32 qid;
	int err;

	if ((n->nlmsg_type != RTM_GETTCLASS) && !capable(CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;

	/*
	   parent == TC_H_UNSPEC - unspecified parent.
	   parent == TC_H_ROOT   - class is root, which has no parent.
	   parent == X:0	 - parent is root class.
	   parent == X:Y	 - parent is a node in hierarchy.
	   parent == 0:Y	 - parent is X:Y, where X:0 is qdisc.

	   handle == 0:0	 - generate handle from kernel pool.
	   handle == 0:Y	 - class is X:Y, where X:0 is qdisc.
	   handle == X:Y	 - clear.
	   handle == X:0	 - root class.
	 */

	/* Step 1. Determine qdisc handle X:0 */

	portid = tcm->tcm_parent;
	clid = tcm->tcm_handle;
	qid = TC_H_MAJ(clid);

	if (portid != TC_H_ROOT) {
		u32 qid1 = TC_H_MAJ(portid);

		if (qid && qid1) {
			/* If both majors are known, they must be identical. */
			if (qid != qid1)
				return -EINVAL;
		} else if (qid1) {
			qid = qid1;
		} else if (qid == 0)
			qid = dev->qdisc->handle;

		/* Now qid is genuine qdisc handle consistent
		 * both with parent and child.
		 *
		 * TC_H_MAJ(portid) still may be unspecified, complete it now.
		 */
		if (portid)
			portid = TC_H_MAKE(qid, portid);
	} else {
		if (qid == 0)
			qid = dev->qdisc->handle;
	}

	/* OK. Locate qdisc */
	q = qdisc_lookup(dev, qid);
	if (!q)
		return -ENOENT;

	/* An check that it supports classes */
	cops = q->ops->cl_ops;
	if (cops == NULL)
		return -EINVAL;

	/* Now try to get class */
	if (clid == 0) {
		if (portid == TC_H_ROOT)
			clid = qid;
	} else
		clid = TC_H_MAKE(qid, clid);

	if (clid)
		cl = cops->get(q, clid);

	if (cl == 0) {
		err = -ENOENT;
		if (n->nlmsg_type != RTM_NEWTCLASS ||
		    !(n->nlmsg_flags & NLM_F_CREATE))
			goto out;
	} else {
		switch (n->nlmsg_type) {
		case RTM_NEWTCLASS:
			err = -EEXIST;
			if (n->nlmsg_flags & NLM_F_EXCL)
				goto out;
			break;
		case RTM_DELTCLASS:
			err = -EOPNOTSUPP;
			if (cops->delete)
				err = cops->delete(q, cl);
			if (err == 0)
				tclass_notify(net, skb, n, q, cl, RTM_DELTCLASS);
			goto out;
		case RTM_GETTCLASS:
			err = tclass_notify(net, skb, n, q, cl, RTM_NEWTCLASS);
			goto out;
		default:
			err = -EINVAL;
			goto out;
		}
	}

	new_cl = cl;
	err = -EOPNOTSUPP;
	if (cops->change)
		err = cops->change(q, clid, portid, tca, &new_cl);
	if (err == 0)
		tclass_notify(net, skb, n, q, new_cl, RTM_NEWTCLASS);

out:
	if (cl)
		cops->put(q, cl);

	return err;
}


static int tc_fill_tclass(struct sk_buff *skb, struct Qdisc *q,
			  unsigned long cl,
			  u32 portid, u32 seq, u16 flags, int event)
{
	struct tcmsg *tcm;
	struct nlmsghdr  *nlh;
	unsigned char *b = skb_tail_pointer(skb);
	struct gnet_dump d;
	const struct Qdisc_class_ops *cl_ops = q->ops->cl_ops;

	cond_resched();
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*tcm), flags);
	if (!nlh)
		goto out_nlmsg_trim;
	tcm = nlmsg_data(nlh);
	tcm->tcm_family = AF_UNSPEC;
	tcm->tcm__pad1 = 0;
	tcm->tcm__pad2 = 0;
	tcm->tcm_ifindex = qdisc_dev(q)->ifindex;
	tcm->tcm_parent = q->handle;
	tcm->tcm_handle = q->handle;
	tcm->tcm_info = 0;
	if (nla_put_string(skb, TCA_KIND, q->ops->id))
		goto nla_put_failure;
	if (cl_ops->dump && cl_ops->dump(q, cl, skb, tcm) < 0)
		goto nla_put_failure;

	if (gnet_stats_start_copy_compat(skb, TCA_STATS2, TCA_STATS, TCA_XSTATS,
					 qdisc_root_sleeping_lock(q), &d) < 0)
		goto nla_put_failure;

	if (cl_ops->dump_stats && cl_ops->dump_stats(q, cl, &d) < 0)
		goto nla_put_failure;

	if (gnet_stats_finish_copy(&d) < 0)
		goto nla_put_failure;

	nlh->nlmsg_len = skb_tail_pointer(skb) - b;
	return skb->len;

out_nlmsg_trim:
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static int tclass_notify(struct net *net, struct sk_buff *oskb,
			 struct nlmsghdr *n, struct Qdisc *q,
			 unsigned long cl, int event)
{
	struct sk_buff *skb;
	u32 portid = oskb ? NETLINK_CB(oskb).portid : 0;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOBUFS;

	if (tc_fill_tclass(skb, q, cl, portid, n->nlmsg_seq, 0, event) < 0) {
		kfree_skb(skb);
		return -EINVAL;
	}

	return rtnetlink_send(skb, net, portid, RTNLGRP_TC,
			      n->nlmsg_flags & NLM_F_ECHO);
}

struct qdisc_dump_args {
	struct qdisc_walker	w;
	struct sk_buff		*skb;
	struct netlink_callback	*cb;
};

static int qdisc_class_dump(struct Qdisc *q, unsigned long cl, struct qdisc_walker *arg)
{
	struct qdisc_dump_args *a = (struct qdisc_dump_args *)arg;

	return tc_fill_tclass(a->skb, q, cl, NETLINK_CB(a->cb->skb).portid,
			      a->cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWTCLASS);
}

static int tc_dump_tclass_qdisc(struct Qdisc *q, struct sk_buff *skb,
				struct tcmsg *tcm, struct netlink_callback *cb,
				int *t_p, int s_t)
{
	struct qdisc_dump_args arg;

	if (tc_qdisc_dump_ignore(q) ||
	    *t_p < s_t || !q->ops->cl_ops ||
	    (tcm->tcm_parent &&
	     TC_H_MAJ(tcm->tcm_parent) != q->handle)) {
		(*t_p)++;
		return 0;
	}
	if (*t_p > s_t)
		memset(&cb->args[1], 0, sizeof(cb->args)-sizeof(cb->args[0]));
	arg.w.fn = qdisc_class_dump;
	arg.skb = skb;
	arg.cb = cb;
	arg.w.stop  = 0;
	arg.w.skip = cb->args[1];
	arg.w.count = 0;
	q->ops->cl_ops->walk(q, &arg.w);
	cb->args[1] = arg.w.count;
	if (arg.w.stop)
		return -1;
	(*t_p)++;
	return 0;
}

static int tc_dump_tclass_root(struct Qdisc *root, struct sk_buff *skb,
			       struct tcmsg *tcm, struct netlink_callback *cb,
			       int *t_p, int s_t)
{
	struct Qdisc *q;

	if (!root)
		return 0;

	if (tc_dump_tclass_qdisc(root, skb, tcm, cb, t_p, s_t) < 0)
		return -1;

	list_for_each_entry(q, &root->list, list) {
		if (tc_dump_tclass_qdisc(q, skb, tcm, cb, t_p, s_t) < 0)
			return -1;
	}

	return 0;
}

static int tc_dump_tclass(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct tcmsg *tcm = nlmsg_data(cb->nlh);
	struct net *net = sock_net(skb->sk);
	struct netdev_queue *dev_queue;
	struct net_device *dev;
	int t, s_t;

	if (nlmsg_len(cb->nlh) < sizeof(*tcm))
		return 0;
	dev = dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return 0;

	s_t = cb->args[0];
	t = 0;

	if (tc_dump_tclass_root(dev->qdisc, skb, tcm, cb, &t, s_t) < 0)
		goto done;

	dev_queue = dev_ingress_queue(dev);
	if (dev_queue &&
	    tc_dump_tclass_root(dev_queue->qdisc_sleeping, skb, tcm, cb,
				&t, s_t) < 0)
		goto done;

done:
	cb->args[0] = t;

	dev_put(dev);
	return skb->len;
}

/* Main classifier routine: scans classifier chain attached
 * to this qdisc, (optionally) tests for protocol and asks
 * specific classifiers.
 */
int tc_classify_compat(struct sk_buff *skb, const struct tcf_proto *tp,
		       struct tcf_result *res)
{
	__be16 protocol = skb->protocol;
	int err;

	for (; tp; tp = tp->next) {
		if (tp->protocol != protocol &&
		    tp->protocol != htons(ETH_P_ALL))
			continue;
		err = tp->classify(skb, tp, res);

		if (err >= 0) {
#ifdef CONFIG_NET_CLS_ACT
			if (err != TC_ACT_RECLASSIFY && skb->tc_verd)
				skb->tc_verd = SET_TC_VERD(skb->tc_verd, 0);
#endif
			return err;
		}
	}
	return -1;
}
EXPORT_SYMBOL(tc_classify_compat);

int tc_classify(struct sk_buff *skb, const struct tcf_proto *tp,
		struct tcf_result *res)
{
	int err = 0;
#ifdef CONFIG_NET_CLS_ACT
	const struct tcf_proto *otp = tp;
reclassify:
#endif

	err = tc_classify_compat(skb, tp, res);
#ifdef CONFIG_NET_CLS_ACT
	if (err == TC_ACT_RECLASSIFY) {
		u32 verd = G_TC_VERD(skb->tc_verd);
		tp = otp;

		if (verd++ >= MAX_REC_LOOP) {
			net_notice_ratelimited("%s: packet reclassify loop rule prio %u protocol %02x\n",
					       tp->q->ops->id,
					       tp->prio & 0xffff,
					       ntohs(tp->protocol));
			return TC_ACT_SHOT;
		}
		skb->tc_verd = SET_TC_VERD(skb->tc_verd, verd);
		goto reclassify;
	}
#endif
	return err;
}
EXPORT_SYMBOL(tc_classify);

void tcf_destroy(struct tcf_proto *tp)
{
	tp->ops->destroy(tp);
	module_put(tp->ops->owner);
	kfree(tp);
}

void tcf_destroy_chain(struct tcf_proto **fl)
{
	struct tcf_proto *tp;

	while ((tp = *fl) != NULL) {
		*fl = tp->next;
		tcf_destroy(tp);
	}
}
EXPORT_SYMBOL(tcf_destroy_chain);

#ifdef CONFIG_PROC_FS
static int psched_show(struct seq_file *seq, void *v)
{
	struct timespec ts;

	hrtimer_get_res(CLOCK_MONOTONIC, &ts);
	seq_printf(seq, "%08x %08x %08x %08x\n",
		   (u32)NSEC_PER_USEC, (u32)PSCHED_TICKS2NS(1),
		   1000000,
		   (u32)NSEC_PER_SEC/(u32)ktime_to_ns(timespec_to_ktime(ts)));

	return 0;
}

static int psched_open(struct inode *inode, struct file *file)
{
	return single_open(file, psched_show, NULL);
}

static const struct file_operations psched_fops = {
	.owner = THIS_MODULE,
	.open = psched_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __net_init psched_net_init(struct net *net)
{
	struct proc_dir_entry *e;

	e = proc_create("psched", 0, net->proc_net, &psched_fops);
	if (e == NULL)
		return -ENOMEM;

	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
	remove_proc_entry("psched", net->proc_net);
}
#else
static int __net_init psched_net_init(struct net *net)
{
	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
}
#endif

static struct pernet_operations psched_net_ops = {
	.init = psched_net_init,
	.exit = psched_net_exit,
};

static int __init pktsched_init(void)
{
	int err;

	err = register_pernet_subsys(&psched_net_ops);
	if (err) {
		pr_err("pktsched_init: "
		       "cannot initialize per netns operations\n");
		return err;
	}

	register_qdisc(&pfifo_fast_ops);
	register_qdisc(&pfifo_qdisc_ops);
	register_qdisc(&bfifo_qdisc_ops);
	register_qdisc(&pfifo_head_drop_qdisc_ops);
	register_qdisc(&mq_qdisc_ops);

	rtnl_register(PF_UNSPEC, RTM_NEWQDISC, tc_modify_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELQDISC, tc_get_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETQDISC, tc_get_qdisc, tc_dump_qdisc, NULL);
	rtnl_register(PF_UNSPEC, RTM_NEWTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETTCLASS, tc_ctl_tclass, tc_dump_tclass, NULL);

	return 0;
}

subsys_initcall(pktsched_init);
{
	struct net *net = sock_net(skb->sk);
	struct tcmsg *tcm = nlmsg_data(n);
	struct nlattr *tca[TCA_MAX + 1];
	struct net_device *dev;
	struct Qdisc *q = NULL;
	const struct Qdisc_class_ops *cops;
	unsigned long cl = 0;
	unsigned long new_cl;
	u32 portid;
	u32 clid;
	u32 qid;
	int err;

	if ((n->nlmsg_type != RTM_GETTCLASS) && !capable(CAP_NET_ADMIN))
		return -EPERM;

	err = nlmsg_parse(n, sizeof(*tcm), tca, TCA_MAX, NULL);
	if (err < 0)
		return err;

	dev = __dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return -ENODEV;

	/*
	   parent == TC_H_UNSPEC - unspecified parent.
	   parent == TC_H_ROOT   - class is root, which has no parent.
	   parent == X:0	 - parent is root class.
	   parent == X:Y	 - parent is a node in hierarchy.
	   parent == 0:Y	 - parent is X:Y, where X:0 is qdisc.

	   handle == 0:0	 - generate handle from kernel pool.
	   handle == 0:Y	 - class is X:Y, where X:0 is qdisc.
	   handle == X:Y	 - clear.
	   handle == X:0	 - root class.
	 */

	/* Step 1. Determine qdisc handle X:0 */

	portid = tcm->tcm_parent;
	clid = tcm->tcm_handle;
	qid = TC_H_MAJ(clid);

	if (portid != TC_H_ROOT) {
		u32 qid1 = TC_H_MAJ(portid);

		if (qid && qid1) {
			/* If both majors are known, they must be identical. */
			if (qid != qid1)
				return -EINVAL;
		} else if (qid1) {
			qid = qid1;
		} else if (qid == 0)
			qid = dev->qdisc->handle;

		/* Now qid is genuine qdisc handle consistent
		 * both with parent and child.
		 *
		 * TC_H_MAJ(portid) still may be unspecified, complete it now.
		 */
		if (portid)
			portid = TC_H_MAKE(qid, portid);
	} else {
		if (qid == 0)
			qid = dev->qdisc->handle;
	}

	/* OK. Locate qdisc */
	q = qdisc_lookup(dev, qid);
	if (!q)
		return -ENOENT;

	/* An check that it supports classes */
	cops = q->ops->cl_ops;
	if (cops == NULL)
		return -EINVAL;

	/* Now try to get class */
	if (clid == 0) {
		if (portid == TC_H_ROOT)
			clid = qid;
	} else
		clid = TC_H_MAKE(qid, clid);

	if (clid)
		cl = cops->get(q, clid);

	if (cl == 0) {
		err = -ENOENT;
		if (n->nlmsg_type != RTM_NEWTCLASS ||
		    !(n->nlmsg_flags & NLM_F_CREATE))
			goto out;
	} else {
		switch (n->nlmsg_type) {
		case RTM_NEWTCLASS:
			err = -EEXIST;
			if (n->nlmsg_flags & NLM_F_EXCL)
				goto out;
			break;
		case RTM_DELTCLASS:
			err = -EOPNOTSUPP;
			if (cops->delete)
				err = cops->delete(q, cl);
			if (err == 0)
				tclass_notify(net, skb, n, q, cl, RTM_DELTCLASS);
			goto out;
		case RTM_GETTCLASS:
			err = tclass_notify(net, skb, n, q, cl, RTM_NEWTCLASS);
			goto out;
		default:
			err = -EINVAL;
			goto out;
		}
	}

	new_cl = cl;
	err = -EOPNOTSUPP;
	if (cops->change)
		err = cops->change(q, clid, portid, tca, &new_cl);
	if (err == 0)
		tclass_notify(net, skb, n, q, new_cl, RTM_NEWTCLASS);

out:
	if (cl)
		cops->put(q, cl);

	return err;
}


static int tc_fill_tclass(struct sk_buff *skb, struct Qdisc *q,
			  unsigned long cl,
			  u32 portid, u32 seq, u16 flags, int event)
{
	struct tcmsg *tcm;
	struct nlmsghdr  *nlh;
	unsigned char *b = skb_tail_pointer(skb);
	struct gnet_dump d;
	const struct Qdisc_class_ops *cl_ops = q->ops->cl_ops;

	cond_resched();
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*tcm), flags);
	if (!nlh)
		goto out_nlmsg_trim;
	tcm = nlmsg_data(nlh);
	tcm->tcm_family = AF_UNSPEC;
	tcm->tcm__pad1 = 0;
	tcm->tcm__pad2 = 0;
	tcm->tcm_ifindex = qdisc_dev(q)->ifindex;
	tcm->tcm_parent = q->handle;
	tcm->tcm_handle = q->handle;
	tcm->tcm_info = 0;
	if (nla_put_string(skb, TCA_KIND, q->ops->id))
		goto nla_put_failure;
	if (cl_ops->dump && cl_ops->dump(q, cl, skb, tcm) < 0)
		goto nla_put_failure;

	if (gnet_stats_start_copy_compat(skb, TCA_STATS2, TCA_STATS, TCA_XSTATS,
					 qdisc_root_sleeping_lock(q), &d) < 0)
		goto nla_put_failure;

	if (cl_ops->dump_stats && cl_ops->dump_stats(q, cl, &d) < 0)
		goto nla_put_failure;

	if (gnet_stats_finish_copy(&d) < 0)
		goto nla_put_failure;

	nlh->nlmsg_len = skb_tail_pointer(skb) - b;
	return skb->len;

out_nlmsg_trim:
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static int tclass_notify(struct net *net, struct sk_buff *oskb,
			 struct nlmsghdr *n, struct Qdisc *q,
			 unsigned long cl, int event)
{
	struct sk_buff *skb;
	u32 portid = oskb ? NETLINK_CB(oskb).portid : 0;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOBUFS;

	if (tc_fill_tclass(skb, q, cl, portid, n->nlmsg_seq, 0, event) < 0) {
		kfree_skb(skb);
		return -EINVAL;
	}

	return rtnetlink_send(skb, net, portid, RTNLGRP_TC,
			      n->nlmsg_flags & NLM_F_ECHO);
}

struct qdisc_dump_args {
	struct qdisc_walker	w;
	struct sk_buff		*skb;
	struct netlink_callback	*cb;
};

static int qdisc_class_dump(struct Qdisc *q, unsigned long cl, struct qdisc_walker *arg)
{
	struct qdisc_dump_args *a = (struct qdisc_dump_args *)arg;

	return tc_fill_tclass(a->skb, q, cl, NETLINK_CB(a->cb->skb).portid,
			      a->cb->nlh->nlmsg_seq, NLM_F_MULTI, RTM_NEWTCLASS);
}

static int tc_dump_tclass_qdisc(struct Qdisc *q, struct sk_buff *skb,
				struct tcmsg *tcm, struct netlink_callback *cb,
				int *t_p, int s_t)
{
	struct qdisc_dump_args arg;

	if (tc_qdisc_dump_ignore(q) ||
	    *t_p < s_t || !q->ops->cl_ops ||
	    (tcm->tcm_parent &&
	     TC_H_MAJ(tcm->tcm_parent) != q->handle)) {
		(*t_p)++;
		return 0;
	}
	if (*t_p > s_t)
		memset(&cb->args[1], 0, sizeof(cb->args)-sizeof(cb->args[0]));
	arg.w.fn = qdisc_class_dump;
	arg.skb = skb;
	arg.cb = cb;
	arg.w.stop  = 0;
	arg.w.skip = cb->args[1];
	arg.w.count = 0;
	q->ops->cl_ops->walk(q, &arg.w);
	cb->args[1] = arg.w.count;
	if (arg.w.stop)
		return -1;
	(*t_p)++;
	return 0;
}

static int tc_dump_tclass_root(struct Qdisc *root, struct sk_buff *skb,
			       struct tcmsg *tcm, struct netlink_callback *cb,
			       int *t_p, int s_t)
{
	struct Qdisc *q;

	if (!root)
		return 0;

	if (tc_dump_tclass_qdisc(root, skb, tcm, cb, t_p, s_t) < 0)
		return -1;

	list_for_each_entry(q, &root->list, list) {
		if (tc_dump_tclass_qdisc(q, skb, tcm, cb, t_p, s_t) < 0)
			return -1;
	}

	return 0;
}

static int tc_dump_tclass(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct tcmsg *tcm = nlmsg_data(cb->nlh);
	struct net *net = sock_net(skb->sk);
	struct netdev_queue *dev_queue;
	struct net_device *dev;
	int t, s_t;

	if (nlmsg_len(cb->nlh) < sizeof(*tcm))
		return 0;
	dev = dev_get_by_index(net, tcm->tcm_ifindex);
	if (!dev)
		return 0;

	s_t = cb->args[0];
	t = 0;

	if (tc_dump_tclass_root(dev->qdisc, skb, tcm, cb, &t, s_t) < 0)
		goto done;

	dev_queue = dev_ingress_queue(dev);
	if (dev_queue &&
	    tc_dump_tclass_root(dev_queue->qdisc_sleeping, skb, tcm, cb,
				&t, s_t) < 0)
		goto done;

done:
	cb->args[0] = t;

	dev_put(dev);
	return skb->len;
}

/* Main classifier routine: scans classifier chain attached
 * to this qdisc, (optionally) tests for protocol and asks
 * specific classifiers.
 */
int tc_classify_compat(struct sk_buff *skb, const struct tcf_proto *tp,
		       struct tcf_result *res)
{
	__be16 protocol = skb->protocol;
	int err;

	for (; tp; tp = tp->next) {
		if (tp->protocol != protocol &&
		    tp->protocol != htons(ETH_P_ALL))
			continue;
		err = tp->classify(skb, tp, res);

		if (err >= 0) {
#ifdef CONFIG_NET_CLS_ACT
			if (err != TC_ACT_RECLASSIFY && skb->tc_verd)
				skb->tc_verd = SET_TC_VERD(skb->tc_verd, 0);
#endif
			return err;
		}
	}
	return -1;
}
EXPORT_SYMBOL(tc_classify_compat);

int tc_classify(struct sk_buff *skb, const struct tcf_proto *tp,
		struct tcf_result *res)
{
	int err = 0;
#ifdef CONFIG_NET_CLS_ACT
	const struct tcf_proto *otp = tp;
reclassify:
#endif

	err = tc_classify_compat(skb, tp, res);
#ifdef CONFIG_NET_CLS_ACT
	if (err == TC_ACT_RECLASSIFY) {
		u32 verd = G_TC_VERD(skb->tc_verd);
		tp = otp;

		if (verd++ >= MAX_REC_LOOP) {
			net_notice_ratelimited("%s: packet reclassify loop rule prio %u protocol %02x\n",
					       tp->q->ops->id,
					       tp->prio & 0xffff,
					       ntohs(tp->protocol));
			return TC_ACT_SHOT;
		}
		skb->tc_verd = SET_TC_VERD(skb->tc_verd, verd);
		goto reclassify;
	}
#endif
	return err;
}
EXPORT_SYMBOL(tc_classify);

void tcf_destroy(struct tcf_proto *tp)
{
	tp->ops->destroy(tp);
	module_put(tp->ops->owner);
	kfree(tp);
}

void tcf_destroy_chain(struct tcf_proto **fl)
{
	struct tcf_proto *tp;

	while ((tp = *fl) != NULL) {
		*fl = tp->next;
		tcf_destroy(tp);
	}
}
EXPORT_SYMBOL(tcf_destroy_chain);

#ifdef CONFIG_PROC_FS
static int psched_show(struct seq_file *seq, void *v)
{
	struct timespec ts;

	hrtimer_get_res(CLOCK_MONOTONIC, &ts);
	seq_printf(seq, "%08x %08x %08x %08x\n",
		   (u32)NSEC_PER_USEC, (u32)PSCHED_TICKS2NS(1),
		   1000000,
		   (u32)NSEC_PER_SEC/(u32)ktime_to_ns(timespec_to_ktime(ts)));

	return 0;
}

static int psched_open(struct inode *inode, struct file *file)
{
	return single_open(file, psched_show, NULL);
}

static const struct file_operations psched_fops = {
	.owner = THIS_MODULE,
	.open = psched_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __net_init psched_net_init(struct net *net)
{
	struct proc_dir_entry *e;

	e = proc_create("psched", 0, net->proc_net, &psched_fops);
	if (e == NULL)
		return -ENOMEM;

	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
	remove_proc_entry("psched", net->proc_net);
}
#else
static int __net_init psched_net_init(struct net *net)
{
	return 0;
}

static void __net_exit psched_net_exit(struct net *net)
{
}
#endif

static struct pernet_operations psched_net_ops = {
	.init = psched_net_init,
	.exit = psched_net_exit,
};

static int __init pktsched_init(void)
{
	int err;

	err = register_pernet_subsys(&psched_net_ops);
	if (err) {
		pr_err("pktsched_init: "
		       "cannot initialize per netns operations\n");
		return err;
	}

	register_qdisc(&pfifo_fast_ops);
	register_qdisc(&pfifo_qdisc_ops);
	register_qdisc(&bfifo_qdisc_ops);
	register_qdisc(&pfifo_head_drop_qdisc_ops);
	register_qdisc(&mq_qdisc_ops);

	rtnl_register(PF_UNSPEC, RTM_NEWQDISC, tc_modify_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELQDISC, tc_get_qdisc, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETQDISC, tc_get_qdisc, tc_dump_qdisc, NULL);
	rtnl_register(PF_UNSPEC, RTM_NEWTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_DELTCLASS, tc_ctl_tclass, NULL, NULL);
	rtnl_register(PF_UNSPEC, RTM_GETTCLASS, tc_ctl_tclass, tc_dump_tclass, NULL);

	return 0;
}

subsys_initcall(pktsched_init);