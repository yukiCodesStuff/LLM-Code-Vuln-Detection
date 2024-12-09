	{
		struct ip_vs_timeout_user t;

		__ip_vs_get_timeouts(net, &t);
		if (copy_to_user(user, &t, sizeof(t)) != 0)
			ret = -EFAULT;
	}
	break;

	default:
		ret = -EINVAL;
	}

out:
	mutex_unlock(&__ip_vs_mutex);
	return ret;
}


static struct nf_sockopt_ops ip_vs_sockopts = {
	.pf		= PF_INET,
	.set_optmin	= IP_VS_BASE_CTL,
	.set_optmax	= IP_VS_SO_SET_MAX+1,
	.set		= do_ip_vs_set_ctl,
	.get_optmin	= IP_VS_BASE_CTL,
	.get_optmax	= IP_VS_SO_GET_MAX+1,
	.get		= do_ip_vs_get_ctl,
	.owner		= THIS_MODULE,
};

/*
 * Generic Netlink interface
 */

/* IPVS genetlink family */
static struct genl_family ip_vs_genl_family = {
	.id		= GENL_ID_GENERATE,
	.hdrsize	= 0,
	.name		= IPVS_GENL_NAME,
	.version	= IPVS_GENL_VERSION,
	.maxattr	= IPVS_CMD_MAX,
	.netnsok        = true,         /* Make ipvsadm to work on netns */
};

/* Policy used for first-level command attributes */
static const struct nla_policy ip_vs_cmd_policy[IPVS_CMD_ATTR_MAX + 1] = {
	[IPVS_CMD_ATTR_SERVICE]		= { .type = NLA_NESTED },
	[IPVS_CMD_ATTR_DEST]		= { .type = NLA_NESTED },
	[IPVS_CMD_ATTR_DAEMON]		= { .type = NLA_NESTED },
	[IPVS_CMD_ATTR_TIMEOUT_TCP]	= { .type = NLA_U32 },
	[IPVS_CMD_ATTR_TIMEOUT_TCP_FIN]	= { .type = NLA_U32 },
	[IPVS_CMD_ATTR_TIMEOUT_UDP]	= { .type = NLA_U32 },
};

/* Policy used for attributes in nested attribute IPVS_CMD_ATTR_DAEMON */
static const struct nla_policy ip_vs_daemon_policy[IPVS_DAEMON_ATTR_MAX + 1] = {
	[IPVS_DAEMON_ATTR_STATE]	= { .type = NLA_U32 },
	[IPVS_DAEMON_ATTR_MCAST_IFN]	= { .type = NLA_NUL_STRING,
					    .len = IP_VS_IFNAME_MAXLEN },
	[IPVS_DAEMON_ATTR_SYNC_ID]	= { .type = NLA_U32 },
};

/* Policy used for attributes in nested attribute IPVS_CMD_ATTR_SERVICE */
static const struct nla_policy ip_vs_svc_policy[IPVS_SVC_ATTR_MAX + 1] = {
	[IPVS_SVC_ATTR_AF]		= { .type = NLA_U16 },
	[IPVS_SVC_ATTR_PROTOCOL]	= { .type = NLA_U16 },
	[IPVS_SVC_ATTR_ADDR]		= { .type = NLA_BINARY,
					    .len = sizeof(union nf_inet_addr) },
	[IPVS_SVC_ATTR_PORT]		= { .type = NLA_U16 },
	[IPVS_SVC_ATTR_FWMARK]		= { .type = NLA_U32 },
	[IPVS_SVC_ATTR_SCHED_NAME]	= { .type = NLA_NUL_STRING,
					    .len = IP_VS_SCHEDNAME_MAXLEN },
	[IPVS_SVC_ATTR_PE_NAME]		= { .type = NLA_NUL_STRING,
					    .len = IP_VS_PENAME_MAXLEN },
	[IPVS_SVC_ATTR_FLAGS]		= { .type = NLA_BINARY,
					    .len = sizeof(struct ip_vs_flags) },
	[IPVS_SVC_ATTR_TIMEOUT]		= { .type = NLA_U32 },
	[IPVS_SVC_ATTR_NETMASK]		= { .type = NLA_U32 },
	[IPVS_SVC_ATTR_STATS]		= { .type = NLA_NESTED },
};

/* Policy used for attributes in nested attribute IPVS_CMD_ATTR_DEST */
static const struct nla_policy ip_vs_dest_policy[IPVS_DEST_ATTR_MAX + 1] = {
	[IPVS_DEST_ATTR_ADDR]		= { .type = NLA_BINARY,
					    .len = sizeof(union nf_inet_addr) },
	[IPVS_DEST_ATTR_PORT]		= { .type = NLA_U16 },
	[IPVS_DEST_ATTR_FWD_METHOD]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_WEIGHT]		= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_U_THRESH]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_L_THRESH]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_ACTIVE_CONNS]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_INACT_CONNS]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_PERSIST_CONNS]	= { .type = NLA_U32 },
	[IPVS_DEST_ATTR_STATS]		= { .type = NLA_NESTED },
};

static int ip_vs_genl_fill_stats(struct sk_buff *skb, int container_type,
				 struct ip_vs_stats *stats)
{
	struct ip_vs_stats_user ustats;
	struct nlattr *nl_stats = nla_nest_start(skb, container_type);
	if (!nl_stats)
		return -EMSGSIZE;

	ip_vs_copy_stats(&ustats, stats);

	if (nla_put_u32(skb, IPVS_STATS_ATTR_CONNS, ustats.conns) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_INPKTS, ustats.inpkts) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_OUTPKTS, ustats.outpkts) ||
	    nla_put_u64(skb, IPVS_STATS_ATTR_INBYTES, ustats.inbytes) ||
	    nla_put_u64(skb, IPVS_STATS_ATTR_OUTBYTES, ustats.outbytes) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_CPS, ustats.cps) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_INPPS, ustats.inpps) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_OUTPPS, ustats.outpps) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_INBPS, ustats.inbps) ||
	    nla_put_u32(skb, IPVS_STATS_ATTR_OUTBPS, ustats.outbps))
		goto nla_put_failure;
	nla_nest_end(skb, nl_stats);

	return 0;

nla_put_failure:
	nla_nest_cancel(skb, nl_stats);
	return -EMSGSIZE;
}

static int ip_vs_genl_fill_service(struct sk_buff *skb,
				   struct ip_vs_service *svc)
{
	struct nlattr *nl_service;
	struct ip_vs_flags flags = { .flags = svc->flags,
				     .mask = ~0 };

	nl_service = nla_nest_start(skb, IPVS_CMD_ATTR_SERVICE);
	if (!nl_service)
		return -EMSGSIZE;

	if (nla_put_u16(skb, IPVS_SVC_ATTR_AF, svc->af))
		goto nla_put_failure;
	if (svc->fwmark) {
		if (nla_put_u32(skb, IPVS_SVC_ATTR_FWMARK, svc->fwmark))
			goto nla_put_failure;
	} else {
		if (nla_put_u16(skb, IPVS_SVC_ATTR_PROTOCOL, svc->protocol) ||
		    nla_put(skb, IPVS_SVC_ATTR_ADDR, sizeof(svc->addr), &svc->addr) ||
		    nla_put_u16(skb, IPVS_SVC_ATTR_PORT, svc->port))
			goto nla_put_failure;
	}

	if (nla_put_string(skb, IPVS_SVC_ATTR_SCHED_NAME, svc->scheduler->name) ||
	    (svc->pe &&
	     nla_put_string(skb, IPVS_SVC_ATTR_PE_NAME, svc->pe->name)) ||
	    nla_put(skb, IPVS_SVC_ATTR_FLAGS, sizeof(flags), &flags) ||
	    nla_put_u32(skb, IPVS_SVC_ATTR_TIMEOUT, svc->timeout / HZ) ||
	    nla_put_u32(skb, IPVS_SVC_ATTR_NETMASK, svc->netmask))
		goto nla_put_failure;
	if (ip_vs_genl_fill_stats(skb, IPVS_SVC_ATTR_STATS, &svc->stats))
		goto nla_put_failure;

	nla_nest_end(skb, nl_service);

	return 0;

nla_put_failure:
	nla_nest_cancel(skb, nl_service);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_service(struct sk_buff *skb,
				   struct ip_vs_service *svc,
				   struct netlink_callback *cb)
{
	void *hdr;

	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).pid, cb->nlh->nlmsg_seq,
			  &ip_vs_genl_family, NLM_F_MULTI,
			  IPVS_CMD_NEW_SERVICE);
	if (!hdr)
		return -EMSGSIZE;

	if (ip_vs_genl_fill_service(skb, svc) < 0)
		goto nla_put_failure;

	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_services(struct sk_buff *skb,
				    struct netlink_callback *cb)
{
	int idx = 0, i;
	int start = cb->args[0];
	struct ip_vs_service *svc;
	struct net *net = skb_sknet(skb);

	mutex_lock(&__ip_vs_mutex);
	for (i = 0; i < IP_VS_SVC_TAB_SIZE; i++) {
		list_for_each_entry(svc, &ip_vs_svc_table[i], s_list) {
			if (++idx <= start || !net_eq(svc->net, net))
				continue;
			if (ip_vs_genl_dump_service(skb, svc, cb) < 0) {
				idx--;
				goto nla_put_failure;
			}
		}
	}

	for (i = 0; i < IP_VS_SVC_TAB_SIZE; i++) {
		list_for_each_entry(svc, &ip_vs_svc_fwm_table[i], f_list) {
			if (++idx <= start || !net_eq(svc->net, net))
				continue;
			if (ip_vs_genl_dump_service(skb, svc, cb) < 0) {
				idx--;
				goto nla_put_failure;
			}
		}
	}

nla_put_failure:
	mutex_unlock(&__ip_vs_mutex);
	cb->args[0] = idx;

	return skb->len;
}

static int ip_vs_genl_parse_service(struct net *net,
				    struct ip_vs_service_user_kern *usvc,
				    struct nlattr *nla, int full_entry,
				    struct ip_vs_service **ret_svc)
{
	struct nlattr *attrs[IPVS_SVC_ATTR_MAX + 1];
	struct nlattr *nla_af, *nla_port, *nla_fwmark, *nla_protocol, *nla_addr;
	struct ip_vs_service *svc;

	/* Parse mandatory identifying service fields first */
	if (nla == NULL ||
	    nla_parse_nested(attrs, IPVS_SVC_ATTR_MAX, nla, ip_vs_svc_policy))
		return -EINVAL;

	nla_af		= attrs[IPVS_SVC_ATTR_AF];
	nla_protocol	= attrs[IPVS_SVC_ATTR_PROTOCOL];
	nla_addr	= attrs[IPVS_SVC_ATTR_ADDR];
	nla_port	= attrs[IPVS_SVC_ATTR_PORT];
	nla_fwmark	= attrs[IPVS_SVC_ATTR_FWMARK];

	if (!(nla_af && (nla_fwmark || (nla_port && nla_protocol && nla_addr))))
		return -EINVAL;

	memset(usvc, 0, sizeof(*usvc));

	usvc->af = nla_get_u16(nla_af);
#ifdef CONFIG_IP_VS_IPV6
	if (usvc->af != AF_INET && usvc->af != AF_INET6)
#else
	if (usvc->af != AF_INET)
#endif
		return -EAFNOSUPPORT;

	if (nla_fwmark) {
		usvc->protocol = IPPROTO_TCP;
		usvc->fwmark = nla_get_u32(nla_fwmark);
	} else {
		usvc->protocol = nla_get_u16(nla_protocol);
		nla_memcpy(&usvc->addr, nla_addr, sizeof(usvc->addr));
		usvc->port = nla_get_u16(nla_port);
		usvc->fwmark = 0;
	}

	if (usvc->fwmark)
		svc = __ip_vs_svc_fwm_find(net, usvc->af, usvc->fwmark);
	else
		svc = __ip_vs_service_find(net, usvc->af, usvc->protocol,
					   &usvc->addr, usvc->port);
	*ret_svc = svc;

	/* If a full entry was requested, check for the additional fields */
	if (full_entry) {
		struct nlattr *nla_sched, *nla_flags, *nla_pe, *nla_timeout,
			      *nla_netmask;
		struct ip_vs_flags flags;

		nla_sched = attrs[IPVS_SVC_ATTR_SCHED_NAME];
		nla_pe = attrs[IPVS_SVC_ATTR_PE_NAME];
		nla_flags = attrs[IPVS_SVC_ATTR_FLAGS];
		nla_timeout = attrs[IPVS_SVC_ATTR_TIMEOUT];
		nla_netmask = attrs[IPVS_SVC_ATTR_NETMASK];

		if (!(nla_sched && nla_flags && nla_timeout && nla_netmask))
			return -EINVAL;

		nla_memcpy(&flags, nla_flags, sizeof(flags));

		/* prefill flags from service if it already exists */
		if (svc)
			usvc->flags = svc->flags;

		/* set new flags from userland */
		usvc->flags = (usvc->flags & ~flags.mask) |
			      (flags.flags & flags.mask);
		usvc->sched_name = nla_data(nla_sched);
		usvc->pe_name = nla_pe ? nla_data(nla_pe) : NULL;
		usvc->timeout = nla_get_u32(nla_timeout);
		usvc->netmask = nla_get_u32(nla_netmask);
	}

	return 0;
}

static struct ip_vs_service *ip_vs_genl_find_service(struct net *net,
						     struct nlattr *nla)
{
	struct ip_vs_service_user_kern usvc;
	struct ip_vs_service *svc;
	int ret;

	ret = ip_vs_genl_parse_service(net, &usvc, nla, 0, &svc);
	return ret ? ERR_PTR(ret) : svc;
}

static int ip_vs_genl_fill_dest(struct sk_buff *skb, struct ip_vs_dest *dest)
{
	struct nlattr *nl_dest;

	nl_dest = nla_nest_start(skb, IPVS_CMD_ATTR_DEST);
	if (!nl_dest)
		return -EMSGSIZE;

	if (nla_put(skb, IPVS_DEST_ATTR_ADDR, sizeof(dest->addr), &dest->addr) ||
	    nla_put_u16(skb, IPVS_DEST_ATTR_PORT, dest->port) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_FWD_METHOD,
			(atomic_read(&dest->conn_flags) &
			 IP_VS_CONN_F_FWD_MASK)) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_WEIGHT,
			atomic_read(&dest->weight)) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_U_THRESH, dest->u_threshold) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_L_THRESH, dest->l_threshold) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_ACTIVE_CONNS,
			atomic_read(&dest->activeconns)) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_INACT_CONNS,
			atomic_read(&dest->inactconns)) ||
	    nla_put_u32(skb, IPVS_DEST_ATTR_PERSIST_CONNS,
			atomic_read(&dest->persistconns)))
		goto nla_put_failure;
	if (ip_vs_genl_fill_stats(skb, IPVS_DEST_ATTR_STATS, &dest->stats))
		goto nla_put_failure;

	nla_nest_end(skb, nl_dest);

	return 0;

nla_put_failure:
	nla_nest_cancel(skb, nl_dest);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_dest(struct sk_buff *skb, struct ip_vs_dest *dest,
				struct netlink_callback *cb)
{
	void *hdr;

	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).pid, cb->nlh->nlmsg_seq,
			  &ip_vs_genl_family, NLM_F_MULTI,
			  IPVS_CMD_NEW_DEST);
	if (!hdr)
		return -EMSGSIZE;

	if (ip_vs_genl_fill_dest(skb, dest) < 0)
		goto nla_put_failure;

	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_dests(struct sk_buff *skb,
				 struct netlink_callback *cb)
{
	int idx = 0;
	int start = cb->args[0];
	struct ip_vs_service *svc;
	struct ip_vs_dest *dest;
	struct nlattr *attrs[IPVS_CMD_ATTR_MAX + 1];
	struct net *net = skb_sknet(skb);

	mutex_lock(&__ip_vs_mutex);

	/* Try to find the service for which to dump destinations */
	if (nlmsg_parse(cb->nlh, GENL_HDRLEN, attrs,
			IPVS_CMD_ATTR_MAX, ip_vs_cmd_policy))
		goto out_err;


	svc = ip_vs_genl_find_service(net, attrs[IPVS_CMD_ATTR_SERVICE]);
	if (IS_ERR(svc) || svc == NULL)
		goto out_err;

	/* Dump the destinations */
	list_for_each_entry(dest, &svc->destinations, n_list) {
		if (++idx <= start)
			continue;
		if (ip_vs_genl_dump_dest(skb, dest, cb) < 0) {
			idx--;
			goto nla_put_failure;
		}
	}

nla_put_failure:
	cb->args[0] = idx;

out_err:
	mutex_unlock(&__ip_vs_mutex);

	return skb->len;
}

static int ip_vs_genl_parse_dest(struct ip_vs_dest_user_kern *udest,
				 struct nlattr *nla, int full_entry)
{
	struct nlattr *attrs[IPVS_DEST_ATTR_MAX + 1];
	struct nlattr *nla_addr, *nla_port;

	/* Parse mandatory identifying destination fields first */
	if (nla == NULL ||
	    nla_parse_nested(attrs, IPVS_DEST_ATTR_MAX, nla, ip_vs_dest_policy))
		return -EINVAL;

	nla_addr	= attrs[IPVS_DEST_ATTR_ADDR];
	nla_port	= attrs[IPVS_DEST_ATTR_PORT];

	if (!(nla_addr && nla_port))
		return -EINVAL;

	memset(udest, 0, sizeof(*udest));

	nla_memcpy(&udest->addr, nla_addr, sizeof(udest->addr));
	udest->port = nla_get_u16(nla_port);

	/* If a full entry was requested, check for the additional fields */
	if (full_entry) {
		struct nlattr *nla_fwd, *nla_weight, *nla_u_thresh,
			      *nla_l_thresh;

		nla_fwd		= attrs[IPVS_DEST_ATTR_FWD_METHOD];
		nla_weight	= attrs[IPVS_DEST_ATTR_WEIGHT];
		nla_u_thresh	= attrs[IPVS_DEST_ATTR_U_THRESH];
		nla_l_thresh	= attrs[IPVS_DEST_ATTR_L_THRESH];

		if (!(nla_fwd && nla_weight && nla_u_thresh && nla_l_thresh))
			return -EINVAL;

		udest->conn_flags = nla_get_u32(nla_fwd)
				    & IP_VS_CONN_F_FWD_MASK;
		udest->weight = nla_get_u32(nla_weight);
		udest->u_threshold = nla_get_u32(nla_u_thresh);
		udest->l_threshold = nla_get_u32(nla_l_thresh);
	}

	return 0;
}

static int ip_vs_genl_fill_daemon(struct sk_buff *skb, __be32 state,
				  const char *mcast_ifn, __be32 syncid)
{
	struct nlattr *nl_daemon;

	nl_daemon = nla_nest_start(skb, IPVS_CMD_ATTR_DAEMON);
	if (!nl_daemon)
		return -EMSGSIZE;

	if (nla_put_u32(skb, IPVS_DAEMON_ATTR_STATE, state) ||
	    nla_put_string(skb, IPVS_DAEMON_ATTR_MCAST_IFN, mcast_ifn) ||
	    nla_put_u32(skb, IPVS_DAEMON_ATTR_SYNC_ID, syncid))
		goto nla_put_failure;
	nla_nest_end(skb, nl_daemon);

	return 0;

nla_put_failure:
	nla_nest_cancel(skb, nl_daemon);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_daemon(struct sk_buff *skb, __be32 state,
				  const char *mcast_ifn, __be32 syncid,
				  struct netlink_callback *cb)
{
	void *hdr;
	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).pid, cb->nlh->nlmsg_seq,
			  &ip_vs_genl_family, NLM_F_MULTI,
			  IPVS_CMD_NEW_DAEMON);
	if (!hdr)
		return -EMSGSIZE;

	if (ip_vs_genl_fill_daemon(skb, state, mcast_ifn, syncid))
		goto nla_put_failure;

	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

static int ip_vs_genl_dump_daemons(struct sk_buff *skb,
				   struct netlink_callback *cb)
{
	struct net *net = skb_sknet(skb);
	struct netns_ipvs *ipvs = net_ipvs(net);

	mutex_lock(&ipvs->sync_mutex);
	if ((ipvs->sync_state & IP_VS_STATE_MASTER) && !cb->args[0]) {
		if (ip_vs_genl_dump_daemon(skb, IP_VS_STATE_MASTER,
					   ipvs->master_mcast_ifn,
					   ipvs->master_syncid, cb) < 0)
			goto nla_put_failure;

		cb->args[0] = 1;
	}

	if ((ipvs->sync_state & IP_VS_STATE_BACKUP) && !cb->args[1]) {
		if (ip_vs_genl_dump_daemon(skb, IP_VS_STATE_BACKUP,
					   ipvs->backup_mcast_ifn,
					   ipvs->backup_syncid, cb) < 0)
			goto nla_put_failure;

		cb->args[1] = 1;
	}

nla_put_failure:
	mutex_unlock(&ipvs->sync_mutex);

	return skb->len;
}

static int ip_vs_genl_new_daemon(struct net *net, struct nlattr **attrs)
{
	if (!(attrs[IPVS_DAEMON_ATTR_STATE] &&
	      attrs[IPVS_DAEMON_ATTR_MCAST_IFN] &&
	      attrs[IPVS_DAEMON_ATTR_SYNC_ID]))
		return -EINVAL;

	return start_sync_thread(net,
				 nla_get_u32(attrs[IPVS_DAEMON_ATTR_STATE]),
				 nla_data(attrs[IPVS_DAEMON_ATTR_MCAST_IFN]),
				 nla_get_u32(attrs[IPVS_DAEMON_ATTR_SYNC_ID]));
}

static int ip_vs_genl_del_daemon(struct net *net, struct nlattr **attrs)
{
	if (!attrs[IPVS_DAEMON_ATTR_STATE])
		return -EINVAL;

	return stop_sync_thread(net,
				nla_get_u32(attrs[IPVS_DAEMON_ATTR_STATE]));
}

static int ip_vs_genl_set_config(struct net *net, struct nlattr **attrs)
{
	struct ip_vs_timeout_user t;

	__ip_vs_get_timeouts(net, &t);

	if (attrs[IPVS_CMD_ATTR_TIMEOUT_TCP])
		t.tcp_timeout = nla_get_u32(attrs[IPVS_CMD_ATTR_TIMEOUT_TCP]);

	if (attrs[IPVS_CMD_ATTR_TIMEOUT_TCP_FIN])
		t.tcp_fin_timeout =
			nla_get_u32(attrs[IPVS_CMD_ATTR_TIMEOUT_TCP_FIN]);

	if (attrs[IPVS_CMD_ATTR_TIMEOUT_UDP])
		t.udp_timeout = nla_get_u32(attrs[IPVS_CMD_ATTR_TIMEOUT_UDP]);

	return ip_vs_set_timeout(net, &t);
}

static int ip_vs_genl_set_daemon(struct sk_buff *skb, struct genl_info *info)
{
	int ret = 0, cmd;
	struct net *net;
	struct netns_ipvs *ipvs;

	net = skb_sknet(skb);
	ipvs = net_ipvs(net);
	cmd = info->genlhdr->cmd;

	if (cmd == IPVS_CMD_NEW_DAEMON || cmd == IPVS_CMD_DEL_DAEMON) {
		struct nlattr *daemon_attrs[IPVS_DAEMON_ATTR_MAX + 1];

		mutex_lock(&ipvs->sync_mutex);
		if (!info->attrs[IPVS_CMD_ATTR_DAEMON] ||
		    nla_parse_nested(daemon_attrs, IPVS_DAEMON_ATTR_MAX,
				     info->attrs[IPVS_CMD_ATTR_DAEMON],
				     ip_vs_daemon_policy)) {
			ret = -EINVAL;
			goto out;
		}

		if (cmd == IPVS_CMD_NEW_DAEMON)
			ret = ip_vs_genl_new_daemon(net, daemon_attrs);
		else
			ret = ip_vs_genl_del_daemon(net, daemon_attrs);
out:
		mutex_unlock(&ipvs->sync_mutex);
	}
	return ret;
}

static int ip_vs_genl_set_cmd(struct sk_buff *skb, struct genl_info *info)
{
	struct ip_vs_service *svc = NULL;
	struct ip_vs_service_user_kern usvc;
	struct ip_vs_dest_user_kern udest;
	int ret = 0, cmd;
	int need_full_svc = 0, need_full_dest = 0;
	struct net *net;

	net = skb_sknet(skb);
	cmd = info->genlhdr->cmd;

	mutex_lock(&__ip_vs_mutex);

	if (cmd == IPVS_CMD_FLUSH) {
		ret = ip_vs_flush(net);
		goto out;
	} else if (cmd == IPVS_CMD_SET_CONFIG) {
		ret = ip_vs_genl_set_config(net, info->attrs);
		goto out;
	} else if (cmd == IPVS_CMD_ZERO &&
		   !info->attrs[IPVS_CMD_ATTR_SERVICE]) {
		ret = ip_vs_zero_all(net);
		goto out;
	}

	/* All following commands require a service argument, so check if we
	 * received a valid one. We need a full service specification when
	 * adding / editing a service. Only identifying members otherwise. */
	if (cmd == IPVS_CMD_NEW_SERVICE || cmd == IPVS_CMD_SET_SERVICE)
		need_full_svc = 1;

	ret = ip_vs_genl_parse_service(net, &usvc,
				       info->attrs[IPVS_CMD_ATTR_SERVICE],
				       need_full_svc, &svc);
	if (ret)
		goto out;

	/* Unless we're adding a new service, the service must already exist */
	if ((cmd != IPVS_CMD_NEW_SERVICE) && (svc == NULL)) {
		ret = -ESRCH;
		goto out;
	}

	/* Destination commands require a valid destination argument. For
	 * adding / editing a destination, we need a full destination
	 * specification. */
	if (cmd == IPVS_CMD_NEW_DEST || cmd == IPVS_CMD_SET_DEST ||
	    cmd == IPVS_CMD_DEL_DEST) {
		if (cmd != IPVS_CMD_DEL_DEST)
			need_full_dest = 1;

		ret = ip_vs_genl_parse_dest(&udest,
					    info->attrs[IPVS_CMD_ATTR_DEST],
					    need_full_dest);
		if (ret)
			goto out;
	}

	switch (cmd) {
	case IPVS_CMD_NEW_SERVICE:
		if (svc == NULL)
			ret = ip_vs_add_service(net, &usvc, &svc);
		else
			ret = -EEXIST;
		break;
	case IPVS_CMD_SET_SERVICE:
		ret = ip_vs_edit_service(svc, &usvc);
		break;
	case IPVS_CMD_DEL_SERVICE:
		ret = ip_vs_del_service(svc);
		/* do not use svc, it can be freed */
		break;
	case IPVS_CMD_NEW_DEST:
		ret = ip_vs_add_dest(svc, &udest);
		break;
	case IPVS_CMD_SET_DEST:
		ret = ip_vs_edit_dest(svc, &udest);
		break;
	case IPVS_CMD_DEL_DEST:
		ret = ip_vs_del_dest(svc, &udest);
		break;
	case IPVS_CMD_ZERO:
		ret = ip_vs_zero_service(svc);
		break;
	default:
		ret = -EINVAL;
	}

out:
	mutex_unlock(&__ip_vs_mutex);

	return ret;
}

static int ip_vs_genl_get_cmd(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *msg;
	void *reply;
	int ret, cmd, reply_cmd;
	struct net *net;

	net = skb_sknet(skb);
	cmd = info->genlhdr->cmd;

	if (cmd == IPVS_CMD_GET_SERVICE)
		reply_cmd = IPVS_CMD_NEW_SERVICE;
	else if (cmd == IPVS_CMD_GET_INFO)
		reply_cmd = IPVS_CMD_SET_INFO;
	else if (cmd == IPVS_CMD_GET_CONFIG)
		reply_cmd = IPVS_CMD_SET_CONFIG;
	else {
		pr_err("unknown Generic Netlink command\n");
		return -EINVAL;
	}

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	mutex_lock(&__ip_vs_mutex);

	reply = genlmsg_put_reply(msg, info, &ip_vs_genl_family, 0, reply_cmd);
	if (reply == NULL)
		goto nla_put_failure;

	switch (cmd) {
	case IPVS_CMD_GET_SERVICE:
	{
		struct ip_vs_service *svc;

		svc = ip_vs_genl_find_service(net,
					      info->attrs[IPVS_CMD_ATTR_SERVICE]);
		if (IS_ERR(svc)) {
			ret = PTR_ERR(svc);
			goto out_err;
		} else if (svc) {
			ret = ip_vs_genl_fill_service(msg, svc);
			if (ret)
				goto nla_put_failure;
		} else {
			ret = -ESRCH;
			goto out_err;
		}

		break;
	}

	case IPVS_CMD_GET_CONFIG:
	{
		struct ip_vs_timeout_user t;

		__ip_vs_get_timeouts(net, &t);
#ifdef CONFIG_IP_VS_PROTO_TCP
		if (nla_put_u32(msg, IPVS_CMD_ATTR_TIMEOUT_TCP,
				t.tcp_timeout) ||
		    nla_put_u32(msg, IPVS_CMD_ATTR_TIMEOUT_TCP_FIN,
				t.tcp_fin_timeout))
			goto nla_put_failure;
#endif
#ifdef CONFIG_IP_VS_PROTO_UDP
		if (nla_put_u32(msg, IPVS_CMD_ATTR_TIMEOUT_UDP, t.udp_timeout))
			goto nla_put_failure;
#endif

		break;
	}

	case IPVS_CMD_GET_INFO:
		if (nla_put_u32(msg, IPVS_INFO_ATTR_VERSION,
				IP_VS_VERSION_CODE) ||
		    nla_put_u32(msg, IPVS_INFO_ATTR_CONN_TAB_SIZE,
				ip_vs_conn_tab_size))
			goto nla_put_failure;
		break;
	}

	genlmsg_end(msg, reply);
	ret = genlmsg_reply(msg, info);
	goto out;

nla_put_failure:
	pr_err("not enough space in Netlink message\n");
	ret = -EMSGSIZE;

out_err:
	nlmsg_free(msg);
out:
	mutex_unlock(&__ip_vs_mutex);

	return ret;
}


static struct genl_ops ip_vs_genl_ops[] __read_mostly = {
	{
		.cmd	= IPVS_CMD_NEW_SERVICE,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_SET_SERVICE,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_DEL_SERVICE,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_GET_SERVICE,
		.flags	= GENL_ADMIN_PERM,
		.doit	= ip_vs_genl_get_cmd,
		.dumpit	= ip_vs_genl_dump_services,
		.policy	= ip_vs_cmd_policy,
	},
	{
		.cmd	= IPVS_CMD_NEW_DEST,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_SET_DEST,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_DEL_DEST,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_GET_DEST,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.dumpit	= ip_vs_genl_dump_dests,
	},
	{
		.cmd	= IPVS_CMD_NEW_DAEMON,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_daemon,
	},
	{
		.cmd	= IPVS_CMD_DEL_DAEMON,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_daemon,
	},
	{
		.cmd	= IPVS_CMD_GET_DAEMON,
		.flags	= GENL_ADMIN_PERM,
		.dumpit	= ip_vs_genl_dump_daemons,
	},
	{
		.cmd	= IPVS_CMD_SET_CONFIG,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_GET_CONFIG,
		.flags	= GENL_ADMIN_PERM,
		.doit	= ip_vs_genl_get_cmd,
	},
	{
		.cmd	= IPVS_CMD_GET_INFO,
		.flags	= GENL_ADMIN_PERM,
		.doit	= ip_vs_genl_get_cmd,
	},
	{
		.cmd	= IPVS_CMD_ZERO,
		.flags	= GENL_ADMIN_PERM,
		.policy	= ip_vs_cmd_policy,
		.doit	= ip_vs_genl_set_cmd,
	},
	{
		.cmd	= IPVS_CMD_FLUSH,
		.flags	= GENL_ADMIN_PERM,
		.doit	= ip_vs_genl_set_cmd,
	},
};

static int __init ip_vs_genl_register(void)
{
	return genl_register_family_with_ops(&ip_vs_genl_family,
		ip_vs_genl_ops, ARRAY_SIZE(ip_vs_genl_ops));
}

static void ip_vs_genl_unregister(void)
{
	genl_unregister_family(&ip_vs_genl_family);
}

/* End of Generic Netlink interface definitions */

/*
 * per netns intit/exit func.
 */
#ifdef CONFIG_SYSCTL
int __net_init ip_vs_control_net_init_sysctl(struct net *net)
{
	int idx;
	struct netns_ipvs *ipvs = net_ipvs(net);
	struct ctl_table *tbl;

	atomic_set(&ipvs->dropentry, 0);
	spin_lock_init(&ipvs->dropentry_lock);
	spin_lock_init(&ipvs->droppacket_lock);
	spin_lock_init(&ipvs->securetcp_lock);

	if (!net_eq(net, &init_net)) {
		tbl = kmemdup(vs_vars, sizeof(vs_vars), GFP_KERNEL);
		if (tbl == NULL)
			return -ENOMEM;
	} else
		tbl = vs_vars;
	/* Initialize sysctl defaults */
	idx = 0;
	ipvs->sysctl_amemthresh = 1024;
	tbl[idx++].data = &ipvs->sysctl_amemthresh;
	ipvs->sysctl_am_droprate = 10;
	tbl[idx++].data = &ipvs->sysctl_am_droprate;
	tbl[idx++].data = &ipvs->sysctl_drop_entry;
	tbl[idx++].data = &ipvs->sysctl_drop_packet;
#ifdef CONFIG_IP_VS_NFCT
	tbl[idx++].data = &ipvs->sysctl_conntrack;
#endif
	tbl[idx++].data = &ipvs->sysctl_secure_tcp;
	ipvs->sysctl_snat_reroute = 1;
	tbl[idx++].data = &ipvs->sysctl_snat_reroute;
	ipvs->sysctl_sync_ver = 1;
	tbl[idx++].data = &ipvs->sysctl_sync_ver;
	ipvs->sysctl_sync_ports = 1;
	tbl[idx++].data = &ipvs->sysctl_sync_ports;
	ipvs->sysctl_sync_qlen_max = nr_free_buffer_pages() / 32;
	tbl[idx++].data = &ipvs->sysctl_sync_qlen_max;
	ipvs->sysctl_sync_sock_size = 0;
	tbl[idx++].data = &ipvs->sysctl_sync_sock_size;
	tbl[idx++].data = &ipvs->sysctl_cache_bypass;
	tbl[idx++].data = &ipvs->sysctl_expire_nodest_conn;
	tbl[idx++].data = &ipvs->sysctl_expire_quiescent_template;
	ipvs->sysctl_sync_threshold[0] = DEFAULT_SYNC_THRESHOLD;
	ipvs->sysctl_sync_threshold[1] = DEFAULT_SYNC_PERIOD;
	tbl[idx].data = &ipvs->sysctl_sync_threshold;
	tbl[idx++].maxlen = sizeof(ipvs->sysctl_sync_threshold);
	ipvs->sysctl_sync_refresh_period = DEFAULT_SYNC_REFRESH_PERIOD;
	tbl[idx++].data = &ipvs->sysctl_sync_refresh_period;
	ipvs->sysctl_sync_retries = clamp_t(int, DEFAULT_SYNC_RETRIES, 0, 3);
	tbl[idx++].data = &ipvs->sysctl_sync_retries;
	tbl[idx++].data = &ipvs->sysctl_nat_icmp_send;


	ipvs->sysctl_hdr = register_net_sysctl(net, "net/ipv4/vs", tbl);
	if (ipvs->sysctl_hdr == NULL) {
		if (!net_eq(net, &init_net))
			kfree(tbl);
		return -ENOMEM;
	}
	ip_vs_start_estimator(net, &ipvs->tot_stats);
	ipvs->sysctl_tbl = tbl;
	/* Schedule defense work */
	INIT_DELAYED_WORK(&ipvs->defense_work, defense_work_handler);
	schedule_delayed_work(&ipvs->defense_work, DEFENSE_TIMER_PERIOD);

	return 0;
}

void __net_exit ip_vs_control_net_cleanup_sysctl(struct net *net)
{
	struct netns_ipvs *ipvs = net_ipvs(net);

	cancel_delayed_work_sync(&ipvs->defense_work);
	cancel_work_sync(&ipvs->defense_work.work);
	unregister_net_sysctl_table(ipvs->sysctl_hdr);
}

#else

int __net_init ip_vs_control_net_init_sysctl(struct net *net) { return 0; }
void __net_exit ip_vs_control_net_cleanup_sysctl(struct net *net) { }

#endif

static struct notifier_block ip_vs_dst_notifier = {
	.notifier_call = ip_vs_dst_event,
};

int __net_init ip_vs_control_net_init(struct net *net)
{
	int idx;
	struct netns_ipvs *ipvs = net_ipvs(net);

	rwlock_init(&ipvs->rs_lock);

	/* Initialize rs_table */
	for (idx = 0; idx < IP_VS_RTAB_SIZE; idx++)
		INIT_LIST_HEAD(&ipvs->rs_table[idx]);

	INIT_LIST_HEAD(&ipvs->dest_trash);
	atomic_set(&ipvs->ftpsvc_counter, 0);
	atomic_set(&ipvs->nullsvc_counter, 0);

	/* procfs stats */
	ipvs->tot_stats.cpustats = alloc_percpu(struct ip_vs_cpu_stats);
	if (!ipvs->tot_stats.cpustats)
		return -ENOMEM;

	spin_lock_init(&ipvs->tot_stats.lock);

	proc_net_fops_create(net, "ip_vs", 0, &ip_vs_info_fops);
	proc_net_fops_create(net, "ip_vs_stats", 0, &ip_vs_stats_fops);
	proc_net_fops_create(net, "ip_vs_stats_percpu", 0,
			     &ip_vs_stats_percpu_fops);

	if (ip_vs_control_net_init_sysctl(net))
		goto err;

	return 0;

err:
	free_percpu(ipvs->tot_stats.cpustats);
	return -ENOMEM;
}

void __net_exit ip_vs_control_net_cleanup(struct net *net)
{
	struct netns_ipvs *ipvs = net_ipvs(net);

	ip_vs_trash_cleanup(net);
	ip_vs_stop_estimator(net, &ipvs->tot_stats);
	ip_vs_control_net_cleanup_sysctl(net);
	proc_net_remove(net, "ip_vs_stats_percpu");
	proc_net_remove(net, "ip_vs_stats");
	proc_net_remove(net, "ip_vs");
	free_percpu(ipvs->tot_stats.cpustats);
}

int __init ip_vs_register_nl_ioctl(void)
{
	int ret;

	ret = nf_register_sockopt(&ip_vs_sockopts);
	if (ret) {
		pr_err("cannot register sockopt.\n");
		goto err_sock;
	}

	ret = ip_vs_genl_register();
	if (ret) {
		pr_err("cannot register Generic Netlink interface.\n");
		goto err_genl;
	}
	return 0;

err_genl:
	nf_unregister_sockopt(&ip_vs_sockopts);
err_sock:
	return ret;
}

void ip_vs_unregister_nl_ioctl(void)
{
	ip_vs_genl_unregister();
	nf_unregister_sockopt(&ip_vs_sockopts);
}

int __init ip_vs_control_init(void)
{
	int idx;
	int ret;

	EnterFunction(2);

	/* Initialize svc_table, ip_vs_svc_fwm_table, rs_table */
	for (idx = 0; idx < IP_VS_SVC_TAB_SIZE; idx++) {
		INIT_LIST_HEAD(&ip_vs_svc_table[idx]);
		INIT_LIST_HEAD(&ip_vs_svc_fwm_table[idx]);
	}

	smp_wmb();	/* Do we really need it now ? */

	ret = register_netdevice_notifier(&ip_vs_dst_notifier);
	if (ret < 0)
		return ret;

	LeaveFunction(2);
	return 0;
}


void ip_vs_control_cleanup(void)
{
	EnterFunction(2);
	unregister_netdevice_notifier(&ip_vs_dst_notifier);
	LeaveFunction(2);
}