const struct nla_policy xfrma_policy[XFRMA_MAX+1] = {
	[XFRMA_SA]		= { .len = sizeof(struct xfrm_usersa_info)},
	[XFRMA_POLICY]		= { .len = sizeof(struct xfrm_userpolicy_info)},
	[XFRMA_LASTUSED]	= { .type = NLA_U64},
	[XFRMA_ALG_AUTH_TRUNC]	= { .len = sizeof(struct xfrm_algo_auth)},
	[XFRMA_ALG_AEAD]	= { .len = sizeof(struct xfrm_algo_aead) },
	[XFRMA_ALG_AUTH]	= { .len = sizeof(struct xfrm_algo) },
	[XFRMA_ALG_CRYPT]	= { .len = sizeof(struct xfrm_algo) },
	[XFRMA_ALG_COMP]	= { .len = sizeof(struct xfrm_algo) },
	[XFRMA_ENCAP]		= { .len = sizeof(struct xfrm_encap_tmpl) },
	[XFRMA_TMPL]		= { .len = sizeof(struct xfrm_user_tmpl) },
	[XFRMA_SEC_CTX]		= { .len = sizeof(struct xfrm_user_sec_ctx) },
	[XFRMA_LTIME_VAL]	= { .len = sizeof(struct xfrm_lifetime_cur) },
	[XFRMA_REPLAY_VAL]	= { .len = sizeof(struct xfrm_replay_state) },
	[XFRMA_REPLAY_THRESH]	= { .type = NLA_U32 },
	[XFRMA_ETIMER_THRESH]	= { .type = NLA_U32 },
	[XFRMA_SRCADDR]		= { .len = sizeof(xfrm_address_t) },
	[XFRMA_COADDR]		= { .len = sizeof(xfrm_address_t) },
	[XFRMA_POLICY_TYPE]	= { .len = sizeof(struct xfrm_userpolicy_type)},
	[XFRMA_MIGRATE]		= { .len = sizeof(struct xfrm_user_migrate) },
	[XFRMA_KMADDRESS]	= { .len = sizeof(struct xfrm_user_kmaddress) },
	[XFRMA_MARK]		= { .len = sizeof(struct xfrm_mark) },
	[XFRMA_TFCPAD]		= { .type = NLA_U32 },
	[XFRMA_REPLAY_ESN_VAL]	= { .len = sizeof(struct xfrm_replay_state_esn) },
	[XFRMA_SA_EXTRA_FLAGS]	= { .type = NLA_U32 },
	[XFRMA_PROTO]		= { .type = NLA_U8 },
	[XFRMA_ADDRESS_FILTER]	= { .len = sizeof(struct xfrm_address_filter) },
	[XFRMA_OFFLOAD_DEV]	= { .len = sizeof(struct xfrm_user_offload) },
	[XFRMA_SET_MARK]	= { .type = NLA_U32 },
	[XFRMA_SET_MARK_MASK]	= { .type = NLA_U32 },
	[XFRMA_IF_ID]		= { .type = NLA_U32 },
	[XFRMA_MTIMER_THRESH]   = { .type = NLA_U32 },
};
EXPORT_SYMBOL_GPL(xfrma_policy);

static const struct nla_policy xfrma_spd_policy[XFRMA_SPD_MAX+1] = {
	[XFRMA_SPD_IPV4_HTHRESH] = { .len = sizeof(struct xfrmu_spdhthresh) },
	[XFRMA_SPD_IPV6_HTHRESH] = { .len = sizeof(struct xfrmu_spdhthresh) },
};

static const struct xfrm_link {
	int (*doit)(struct sk_buff *, struct nlmsghdr *, struct nlattr **,
		    struct netlink_ext_ack *);
	int (*start)(struct netlink_callback *);
	int (*dump)(struct sk_buff *, struct netlink_callback *);
	int (*done)(struct netlink_callback *);
	const struct nla_policy *nla_pol;
	int nla_max;
} xfrm_dispatch[XFRM_NR_MSGTYPES] = {
	[XFRM_MSG_NEWSA       - XFRM_MSG_BASE] = { .doit = xfrm_add_sa        },
	[XFRM_MSG_DELSA       - XFRM_MSG_BASE] = { .doit = xfrm_del_sa        },
	[XFRM_MSG_GETSA       - XFRM_MSG_BASE] = { .doit = xfrm_get_sa,
						   .dump = xfrm_dump_sa,
						   .done = xfrm_dump_sa_done  },
	[XFRM_MSG_NEWPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_add_policy    },
	[XFRM_MSG_DELPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_get_policy    },
	[XFRM_MSG_GETPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_get_policy,
						   .start = xfrm_dump_policy_start,
						   .dump = xfrm_dump_policy,
						   .done = xfrm_dump_policy_done },
	[XFRM_MSG_ALLOCSPI    - XFRM_MSG_BASE] = { .doit = xfrm_alloc_userspi },
	[XFRM_MSG_ACQUIRE     - XFRM_MSG_BASE] = { .doit = xfrm_add_acquire   },
	[XFRM_MSG_EXPIRE      - XFRM_MSG_BASE] = { .doit = xfrm_add_sa_expire },
	[XFRM_MSG_UPDPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_add_policy    },
	[XFRM_MSG_UPDSA       - XFRM_MSG_BASE] = { .doit = xfrm_add_sa        },
	[XFRM_MSG_POLEXPIRE   - XFRM_MSG_BASE] = { .doit = xfrm_add_pol_expire},
	[XFRM_MSG_FLUSHSA     - XFRM_MSG_BASE] = { .doit = xfrm_flush_sa      },
	[XFRM_MSG_FLUSHPOLICY - XFRM_MSG_BASE] = { .doit = xfrm_flush_policy  },
	[XFRM_MSG_NEWAE       - XFRM_MSG_BASE] = { .doit = xfrm_new_ae  },
	[XFRM_MSG_GETAE       - XFRM_MSG_BASE] = { .doit = xfrm_get_ae  },
	[XFRM_MSG_MIGRATE     - XFRM_MSG_BASE] = { .doit = xfrm_do_migrate    },
	[XFRM_MSG_GETSADINFO  - XFRM_MSG_BASE] = { .doit = xfrm_get_sadinfo   },
	[XFRM_MSG_NEWSPDINFO  - XFRM_MSG_BASE] = { .doit = xfrm_set_spdinfo,
						   .nla_pol = xfrma_spd_policy,
						   .nla_max = XFRMA_SPD_MAX },
	[XFRM_MSG_GETSPDINFO  - XFRM_MSG_BASE] = { .doit = xfrm_get_spdinfo   },
	[XFRM_MSG_SETDEFAULT  - XFRM_MSG_BASE] = { .doit = xfrm_set_default   },
	[XFRM_MSG_GETDEFAULT  - XFRM_MSG_BASE] = { .doit = xfrm_get_default   },
};

static int xfrm_user_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh,
			     struct netlink_ext_ack *extack)
{
	struct net *net = sock_net(skb->sk);
	struct nlattr *attrs[XFRMA_MAX+1];
	const struct xfrm_link *link;
	struct nlmsghdr *nlh64 = NULL;
	int type, err;

	type = nlh->nlmsg_type;
	if (type > XFRM_MSG_MAX)
		return -EINVAL;

	type -= XFRM_MSG_BASE;
	link = &xfrm_dispatch[type];

	/* All operations require privileges, even GET */
	if (!netlink_net_capable(skb, CAP_NET_ADMIN))
		return -EPERM;

	if (in_compat_syscall()) {
		struct xfrm_translator *xtr = xfrm_get_translator();

		if (!xtr)
			return -EOPNOTSUPP;

		nlh64 = xtr->rcv_msg_compat(nlh, link->nla_max,
					    link->nla_pol, extack);
		xfrm_put_translator(xtr);
		if (IS_ERR(nlh64))
			return PTR_ERR(nlh64);
		if (nlh64)
			nlh = nlh64;
	}

	if ((type == (XFRM_MSG_GETSA - XFRM_MSG_BASE) ||
	     type == (XFRM_MSG_GETPOLICY - XFRM_MSG_BASE)) &&
	    (nlh->nlmsg_flags & NLM_F_DUMP)) {
		struct netlink_dump_control c = {
			.start = link->start,
			.dump = link->dump,
			.done = link->done,
		};

		if (link->dump == NULL) {
			err = -EINVAL;
			goto err;
		}

		err = netlink_dump_start(net->xfrm.nlsk, skb, nlh, &c);
		goto err;
	}

	err = nlmsg_parse_deprecated(nlh, xfrm_msg_min[type], attrs,
				     link->nla_max ? : XFRMA_MAX,
				     link->nla_pol ? : xfrma_policy, extack);
	if (err < 0)
		goto err;

	if (link->doit == NULL) {
		err = -EINVAL;
		goto err;
	}

	err = link->doit(skb, nlh, attrs, extack);

	/* We need to free skb allocated in xfrm_alloc_compat() before
	 * returning from this function, because consume_skb() won't take
	 * care of frag_list since netlink destructor sets
	 * sbk->head to NULL. (see netlink_skb_destructor())
	 */
	if (skb_has_frag_list(skb)) {
		kfree_skb(skb_shinfo(skb)->frag_list);
		skb_shinfo(skb)->frag_list = NULL;
	}

err:
	kvfree(nlh64);
	return err;
}

static void xfrm_netlink_rcv(struct sk_buff *skb)
{
	struct net *net = sock_net(skb->sk);

	mutex_lock(&net->xfrm.xfrm_cfg_mutex);
	netlink_rcv_skb(skb, &xfrm_user_rcv_msg);
	mutex_unlock(&net->xfrm.xfrm_cfg_mutex);
}

static inline unsigned int xfrm_expire_msgsize(void)
{
	return NLMSG_ALIGN(sizeof(struct xfrm_user_expire))
	       + nla_total_size(sizeof(struct xfrm_mark));
}

static int build_expire(struct sk_buff *skb, struct xfrm_state *x, const struct km_event *c)
{
	struct xfrm_user_expire *ue;
	struct nlmsghdr *nlh;
	int err;

	nlh = nlmsg_put(skb, c->portid, 0, XFRM_MSG_EXPIRE, sizeof(*ue), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	ue = nlmsg_data(nlh);
	copy_to_user_state(x, &ue->state);
	ue->hard = (c->data.hard != 0) ? 1 : 0;
	/* clear the padding bytes */
	memset_after(ue, 0, hard);

	err = xfrm_mark_put(skb, &x->mark);
	if (err)
		return err;

	err = xfrm_if_id_put(skb, x->if_id);
	if (err)
		return err;

	nlmsg_end(skb, nlh);
	return 0;
}

static int xfrm_exp_state_notify(struct xfrm_state *x, const struct km_event *c)
{
	struct net *net = xs_net(x);
	struct sk_buff *skb;

	skb = nlmsg_new(xfrm_expire_msgsize(), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	if (build_expire(skb, x, c) < 0) {
		kfree_skb(skb);
		return -EMSGSIZE;
	}

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_EXPIRE);
}

static int xfrm_aevent_state_notify(struct xfrm_state *x, const struct km_event *c)
{
	struct net *net = xs_net(x);
	struct sk_buff *skb;
	int err;

	skb = nlmsg_new(xfrm_aevent_msgsize(x), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	err = build_aevent(skb, x, c);
	BUG_ON(err < 0);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_AEVENTS);
}

static int xfrm_notify_sa_flush(const struct km_event *c)
{
	struct net *net = c->net;
	struct xfrm_usersa_flush *p;
	struct nlmsghdr *nlh;
	struct sk_buff *skb;
	int len = NLMSG_ALIGN(sizeof(struct xfrm_usersa_flush));

	skb = nlmsg_new(len, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	nlh = nlmsg_put(skb, c->portid, c->seq, XFRM_MSG_FLUSHSA, sizeof(*p), 0);
	if (nlh == NULL) {
		kfree_skb(skb);
		return -EMSGSIZE;
	}

	p = nlmsg_data(nlh);
	p->proto = c->data.proto;

	nlmsg_end(skb, nlh);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_SA);
}

static inline unsigned int xfrm_sa_len(struct xfrm_state *x)
{
	unsigned int l = 0;
	if (x->aead)
		l += nla_total_size(aead_len(x->aead));
	if (x->aalg) {
		l += nla_total_size(sizeof(struct xfrm_algo) +
				    (x->aalg->alg_key_len + 7) / 8);
		l += nla_total_size(xfrm_alg_auth_len(x->aalg));
	}
	if (x->ealg)
		l += nla_total_size(xfrm_alg_len(x->ealg));
	if (x->calg)
		l += nla_total_size(sizeof(*x->calg));
	if (x->encap)
		l += nla_total_size(sizeof(*x->encap));
	if (x->tfcpad)
		l += nla_total_size(sizeof(x->tfcpad));
	if (x->replay_esn)
		l += nla_total_size(xfrm_replay_state_esn_len(x->replay_esn));
	else
		l += nla_total_size(sizeof(struct xfrm_replay_state));
	if (x->security)
		l += nla_total_size(sizeof(struct xfrm_user_sec_ctx) +
				    x->security->ctx_len);
	if (x->coaddr)
		l += nla_total_size(sizeof(*x->coaddr));
	if (x->props.extra_flags)
		l += nla_total_size(sizeof(x->props.extra_flags));
	if (x->xso.dev)
		 l += nla_total_size(sizeof(struct xfrm_user_offload));
	if (x->props.smark.v | x->props.smark.m) {
		l += nla_total_size(sizeof(x->props.smark.v));
		l += nla_total_size(sizeof(x->props.smark.m));
	}
	if (x->if_id)
		l += nla_total_size(sizeof(x->if_id));

	/* Must count x->lastused as it may become non-zero behind our back. */
	l += nla_total_size_64bit(sizeof(u64));

	if (x->mapping_maxage)
		l += nla_total_size(sizeof(x->mapping_maxage));

	return l;
}

static int xfrm_notify_sa(struct xfrm_state *x, const struct km_event *c)
{
	struct net *net = xs_net(x);
	struct xfrm_usersa_info *p;
	struct xfrm_usersa_id *id;
	struct nlmsghdr *nlh;
	struct sk_buff *skb;
	unsigned int len = xfrm_sa_len(x);
	unsigned int headlen;
	int err;

	headlen = sizeof(*p);
	if (c->event == XFRM_MSG_DELSA) {
		len += nla_total_size(headlen);
		headlen = sizeof(*id);
		len += nla_total_size(sizeof(struct xfrm_mark));
	}
	len += NLMSG_ALIGN(headlen);

	skb = nlmsg_new(len, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	nlh = nlmsg_put(skb, c->portid, c->seq, c->event, headlen, 0);
	err = -EMSGSIZE;
	if (nlh == NULL)
		goto out_free_skb;

	p = nlmsg_data(nlh);
	if (c->event == XFRM_MSG_DELSA) {
		struct nlattr *attr;

		id = nlmsg_data(nlh);
		memset(id, 0, sizeof(*id));
		memcpy(&id->daddr, &x->id.daddr, sizeof(id->daddr));
		id->spi = x->id.spi;
		id->family = x->props.family;
		id->proto = x->id.proto;

		attr = nla_reserve(skb, XFRMA_SA, sizeof(*p));
		err = -EMSGSIZE;
		if (attr == NULL)
			goto out_free_skb;

		p = nla_data(attr);
	}
	err = copy_to_user_state_extra(x, p, skb);
	if (err)
		goto out_free_skb;

	nlmsg_end(skb, nlh);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_SA);

out_free_skb:
	kfree_skb(skb);
	return err;
}

static int xfrm_send_state_notify(struct xfrm_state *x, const struct km_event *c)
{

	switch (c->event) {
	case XFRM_MSG_EXPIRE:
		return xfrm_exp_state_notify(x, c);
	case XFRM_MSG_NEWAE:
		return xfrm_aevent_state_notify(x, c);
	case XFRM_MSG_DELSA:
	case XFRM_MSG_UPDSA:
	case XFRM_MSG_NEWSA:
		return xfrm_notify_sa(x, c);
	case XFRM_MSG_FLUSHSA:
		return xfrm_notify_sa_flush(c);
	default:
		printk(KERN_NOTICE "xfrm_user: Unknown SA event %d\n",
		       c->event);
		break;
	}

	return 0;

}

static inline unsigned int xfrm_acquire_msgsize(struct xfrm_state *x,
						struct xfrm_policy *xp)
{
	return NLMSG_ALIGN(sizeof(struct xfrm_user_acquire))
	       + nla_total_size(sizeof(struct xfrm_user_tmpl) * xp->xfrm_nr)
	       + nla_total_size(sizeof(struct xfrm_mark))
	       + nla_total_size(xfrm_user_sec_ctx_size(x->security))
	       + userpolicy_type_attrsize();
}

static int build_acquire(struct sk_buff *skb, struct xfrm_state *x,
			 struct xfrm_tmpl *xt, struct xfrm_policy *xp)
{
	__u32 seq = xfrm_get_acqseq();
	struct xfrm_user_acquire *ua;
	struct nlmsghdr *nlh;
	int err;

	nlh = nlmsg_put(skb, 0, 0, XFRM_MSG_ACQUIRE, sizeof(*ua), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	ua = nlmsg_data(nlh);
	memcpy(&ua->id, &x->id, sizeof(ua->id));
	memcpy(&ua->saddr, &x->props.saddr, sizeof(ua->saddr));
	memcpy(&ua->sel, &x->sel, sizeof(ua->sel));
	copy_to_user_policy(xp, &ua->policy, XFRM_POLICY_OUT);
	ua->aalgos = xt->aalgos;
	ua->ealgos = xt->ealgos;
	ua->calgos = xt->calgos;
	ua->seq = x->km.seq = seq;

	err = copy_to_user_tmpl(xp, skb);
	if (!err)
		err = copy_to_user_state_sec_ctx(x, skb);
	if (!err)
		err = copy_to_user_policy_type(xp->type, skb);
	if (!err)
		err = xfrm_mark_put(skb, &xp->mark);
	if (!err)
		err = xfrm_if_id_put(skb, xp->if_id);
	if (!err && xp->xdo.dev)
		err = copy_user_offload(&xp->xdo, skb);
	if (err) {
		nlmsg_cancel(skb, nlh);
		return err;
	}

	nlmsg_end(skb, nlh);
	return 0;
}

static int xfrm_send_acquire(struct xfrm_state *x, struct xfrm_tmpl *xt,
			     struct xfrm_policy *xp)
{
	struct net *net = xs_net(x);
	struct sk_buff *skb;
	int err;

	skb = nlmsg_new(xfrm_acquire_msgsize(x, xp), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	err = build_acquire(skb, x, xt, xp);
	BUG_ON(err < 0);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_ACQUIRE);
}

/* User gives us xfrm_user_policy_info followed by an array of 0
 * or more templates.
 */
static struct xfrm_policy *xfrm_compile_policy(struct sock *sk, int opt,
					       u8 *data, int len, int *dir)
{
	struct net *net = sock_net(sk);
	struct xfrm_userpolicy_info *p = (struct xfrm_userpolicy_info *)data;
	struct xfrm_user_tmpl *ut = (struct xfrm_user_tmpl *) (p + 1);
	struct xfrm_policy *xp;
	int nr;

	switch (sk->sk_family) {
	case AF_INET:
		if (opt != IP_XFRM_POLICY) {
			*dir = -EOPNOTSUPP;
			return NULL;
		}
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		if (opt != IPV6_XFRM_POLICY) {
			*dir = -EOPNOTSUPP;
			return NULL;
		}
		break;
#endif
	default:
		*dir = -EINVAL;
		return NULL;
	}

	*dir = -EINVAL;

	if (len < sizeof(*p) ||
	    verify_newpolicy_info(p, NULL))
		return NULL;

	nr = ((len - sizeof(*p)) / sizeof(*ut));
	if (validate_tmpl(nr, ut, p->sel.family, p->dir, NULL))
		return NULL;

	if (p->dir > XFRM_POLICY_OUT)
		return NULL;

	xp = xfrm_policy_alloc(net, GFP_ATOMIC);
	if (xp == NULL) {
		*dir = -ENOBUFS;
		return NULL;
	}

	copy_from_user_policy(xp, p);
	xp->type = XFRM_POLICY_TYPE_MAIN;
	copy_templates(xp, ut, nr);

	*dir = p->dir;

	return xp;
}

static inline unsigned int xfrm_polexpire_msgsize(struct xfrm_policy *xp)
{
	return NLMSG_ALIGN(sizeof(struct xfrm_user_polexpire))
	       + nla_total_size(sizeof(struct xfrm_user_tmpl) * xp->xfrm_nr)
	       + nla_total_size(xfrm_user_sec_ctx_size(xp->security))
	       + nla_total_size(sizeof(struct xfrm_mark))
	       + userpolicy_type_attrsize();
}

static int build_polexpire(struct sk_buff *skb, struct xfrm_policy *xp,
			   int dir, const struct km_event *c)
{
	struct xfrm_user_polexpire *upe;
	int hard = c->data.hard;
	struct nlmsghdr *nlh;
	int err;

	nlh = nlmsg_put(skb, c->portid, 0, XFRM_MSG_POLEXPIRE, sizeof(*upe), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	upe = nlmsg_data(nlh);
	copy_to_user_policy(xp, &upe->pol, dir);
	err = copy_to_user_tmpl(xp, skb);
	if (!err)
		err = copy_to_user_sec_ctx(xp, skb);
	if (!err)
		err = copy_to_user_policy_type(xp->type, skb);
	if (!err)
		err = xfrm_mark_put(skb, &xp->mark);
	if (!err)
		err = xfrm_if_id_put(skb, xp->if_id);
	if (!err && xp->xdo.dev)
		err = copy_user_offload(&xp->xdo, skb);
	if (err) {
		nlmsg_cancel(skb, nlh);
		return err;
	}
	upe->hard = !!hard;

	nlmsg_end(skb, nlh);
	return 0;
}

static int xfrm_exp_policy_notify(struct xfrm_policy *xp, int dir, const struct km_event *c)
{
	struct net *net = xp_net(xp);
	struct sk_buff *skb;
	int err;

	skb = nlmsg_new(xfrm_polexpire_msgsize(xp), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	err = build_polexpire(skb, xp, dir, c);
	BUG_ON(err < 0);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_EXPIRE);
}

static int xfrm_notify_policy(struct xfrm_policy *xp, int dir, const struct km_event *c)
{
	unsigned int len = nla_total_size(sizeof(struct xfrm_user_tmpl) * xp->xfrm_nr);
	struct net *net = xp_net(xp);
	struct xfrm_userpolicy_info *p;
	struct xfrm_userpolicy_id *id;
	struct nlmsghdr *nlh;
	struct sk_buff *skb;
	unsigned int headlen;
	int err;

	headlen = sizeof(*p);
	if (c->event == XFRM_MSG_DELPOLICY) {
		len += nla_total_size(headlen);
		headlen = sizeof(*id);
	}
	len += userpolicy_type_attrsize();
	len += nla_total_size(sizeof(struct xfrm_mark));
	len += NLMSG_ALIGN(headlen);

	skb = nlmsg_new(len, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	nlh = nlmsg_put(skb, c->portid, c->seq, c->event, headlen, 0);
	err = -EMSGSIZE;
	if (nlh == NULL)
		goto out_free_skb;

	p = nlmsg_data(nlh);
	if (c->event == XFRM_MSG_DELPOLICY) {
		struct nlattr *attr;

		id = nlmsg_data(nlh);
		memset(id, 0, sizeof(*id));
		id->dir = dir;
		if (c->data.byid)
			id->index = xp->index;
		else
			memcpy(&id->sel, &xp->selector, sizeof(id->sel));

		attr = nla_reserve(skb, XFRMA_POLICY, sizeof(*p));
		err = -EMSGSIZE;
		if (attr == NULL)
			goto out_free_skb;

		p = nla_data(attr);
	}

	copy_to_user_policy(xp, p, dir);
	err = copy_to_user_tmpl(xp, skb);
	if (!err)
		err = copy_to_user_policy_type(xp->type, skb);
	if (!err)
		err = xfrm_mark_put(skb, &xp->mark);
	if (!err)
		err = xfrm_if_id_put(skb, xp->if_id);
	if (!err && xp->xdo.dev)
		err = copy_user_offload(&xp->xdo, skb);
	if (err)
		goto out_free_skb;

	nlmsg_end(skb, nlh);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_POLICY);

out_free_skb:
	kfree_skb(skb);
	return err;
}

static int xfrm_notify_policy_flush(const struct km_event *c)
{
	struct net *net = c->net;
	struct nlmsghdr *nlh;
	struct sk_buff *skb;
	int err;

	skb = nlmsg_new(userpolicy_type_attrsize(), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	nlh = nlmsg_put(skb, c->portid, c->seq, XFRM_MSG_FLUSHPOLICY, 0, 0);
	err = -EMSGSIZE;
	if (nlh == NULL)
		goto out_free_skb;
	err = copy_to_user_policy_type(c->data.type, skb);
	if (err)
		goto out_free_skb;

	nlmsg_end(skb, nlh);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_POLICY);

out_free_skb:
	kfree_skb(skb);
	return err;
}

static int xfrm_send_policy_notify(struct xfrm_policy *xp, int dir, const struct km_event *c)
{

	switch (c->event) {
	case XFRM_MSG_NEWPOLICY:
	case XFRM_MSG_UPDPOLICY:
	case XFRM_MSG_DELPOLICY:
		return xfrm_notify_policy(xp, dir, c);
	case XFRM_MSG_FLUSHPOLICY:
		return xfrm_notify_policy_flush(c);
	case XFRM_MSG_POLEXPIRE:
		return xfrm_exp_policy_notify(xp, dir, c);
	default:
		printk(KERN_NOTICE "xfrm_user: Unknown Policy event %d\n",
		       c->event);
	}

	return 0;

}

static inline unsigned int xfrm_report_msgsize(void)
{
	return NLMSG_ALIGN(sizeof(struct xfrm_user_report));
}

static int build_report(struct sk_buff *skb, u8 proto,
			struct xfrm_selector *sel, xfrm_address_t *addr)
{
	struct xfrm_user_report *ur;
	struct nlmsghdr *nlh;

	nlh = nlmsg_put(skb, 0, 0, XFRM_MSG_REPORT, sizeof(*ur), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	ur = nlmsg_data(nlh);
	ur->proto = proto;
	memcpy(&ur->sel, sel, sizeof(ur->sel));

	if (addr) {
		int err = nla_put(skb, XFRMA_COADDR, sizeof(*addr), addr);
		if (err) {
			nlmsg_cancel(skb, nlh);
			return err;
		}
	}
	nlmsg_end(skb, nlh);
	return 0;
}

static int xfrm_send_report(struct net *net, u8 proto,
			    struct xfrm_selector *sel, xfrm_address_t *addr)
{
	struct sk_buff *skb;
	int err;

	skb = nlmsg_new(xfrm_report_msgsize(), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	err = build_report(skb, proto, sel, addr);
	BUG_ON(err < 0);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_REPORT);
}

static inline unsigned int xfrm_mapping_msgsize(void)
{
	return NLMSG_ALIGN(sizeof(struct xfrm_user_mapping));
}

static int build_mapping(struct sk_buff *skb, struct xfrm_state *x,
			 xfrm_address_t *new_saddr, __be16 new_sport)
{
	struct xfrm_user_mapping *um;
	struct nlmsghdr *nlh;

	nlh = nlmsg_put(skb, 0, 0, XFRM_MSG_MAPPING, sizeof(*um), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	um = nlmsg_data(nlh);

	memcpy(&um->id.daddr, &x->id.daddr, sizeof(um->id.daddr));
	um->id.spi = x->id.spi;
	um->id.family = x->props.family;
	um->id.proto = x->id.proto;
	memcpy(&um->new_saddr, new_saddr, sizeof(um->new_saddr));
	memcpy(&um->old_saddr, &x->props.saddr, sizeof(um->old_saddr));
	um->new_sport = new_sport;
	um->old_sport = x->encap->encap_sport;
	um->reqid = x->props.reqid;

	nlmsg_end(skb, nlh);
	return 0;
}

static int xfrm_send_mapping(struct xfrm_state *x, xfrm_address_t *ipaddr,
			     __be16 sport)
{
	struct net *net = xs_net(x);
	struct sk_buff *skb;
	int err;

	if (x->id.proto != IPPROTO_ESP)
		return -EINVAL;

	if (!x->encap)
		return -EINVAL;

	skb = nlmsg_new(xfrm_mapping_msgsize(), GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	err = build_mapping(skb, x, ipaddr, sport);
	BUG_ON(err < 0);

	return xfrm_nlmsg_multicast(net, skb, 0, XFRMNLGRP_MAPPING);
}

static bool xfrm_is_alive(const struct km_event *c)
{
	return (bool)xfrm_acquire_is_on(c->net);
}

static struct xfrm_mgr netlink_mgr = {
	.notify		= xfrm_send_state_notify,
	.acquire	= xfrm_send_acquire,
	.compile_policy	= xfrm_compile_policy,
	.notify_policy	= xfrm_send_policy_notify,
	.report		= xfrm_send_report,
	.migrate	= xfrm_send_migrate,
	.new_mapping	= xfrm_send_mapping,
	.is_alive	= xfrm_is_alive,
};

static int __net_init xfrm_user_net_init(struct net *net)
{
	struct sock *nlsk;
	struct netlink_kernel_cfg cfg = {
		.groups	= XFRMNLGRP_MAX,
		.input	= xfrm_netlink_rcv,
	};

	nlsk = netlink_kernel_create(net, NETLINK_XFRM, &cfg);
	if (nlsk == NULL)
		return -ENOMEM;
	net->xfrm.nlsk_stash = nlsk; /* Don't set to NULL */
	rcu_assign_pointer(net->xfrm.nlsk, nlsk);
	return 0;
}

static void __net_exit xfrm_user_net_pre_exit(struct net *net)
{
	RCU_INIT_POINTER(net->xfrm.nlsk, NULL);
}

static void __net_exit xfrm_user_net_exit(struct list_head *net_exit_list)
{
	struct net *net;

	list_for_each_entry(net, net_exit_list, exit_list)
		netlink_kernel_release(net->xfrm.nlsk_stash);
}

static struct pernet_operations xfrm_user_net_ops = {
	.init	    = xfrm_user_net_init,
	.pre_exit   = xfrm_user_net_pre_exit,
	.exit_batch = xfrm_user_net_exit,
};

static int __init xfrm_user_init(void)
{
	int rv;

	printk(KERN_INFO "Initializing XFRM netlink socket\n");

	rv = register_pernet_subsys(&xfrm_user_net_ops);
	if (rv < 0)
		return rv;
	xfrm_register_km(&netlink_mgr);
	return 0;
}

static void __exit xfrm_user_exit(void)
{
	xfrm_unregister_km(&netlink_mgr);
	unregister_pernet_subsys(&xfrm_user_net_ops);
}

module_init(xfrm_user_init);
module_exit(xfrm_user_exit);
MODULE_LICENSE("GPL");
MODULE_ALIAS_NET_PF_PROTO(PF_NETLINK, NETLINK_XFRM);