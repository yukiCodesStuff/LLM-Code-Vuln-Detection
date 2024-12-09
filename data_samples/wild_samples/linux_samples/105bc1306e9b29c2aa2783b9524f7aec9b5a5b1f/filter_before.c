	if (!dev->netdev_ops->ndo_xdp_xmit) {
		return -EOPNOTSUPP;
	}

	err = xdp_ok_fwd_dev(dev, xdp->data_end - xdp->data);
	if (unlikely(err))
		return err;

	xdpf = convert_to_xdp_frame(xdp);
	if (unlikely(!xdpf))
		return -EOVERFLOW;

	sent = dev->netdev_ops->ndo_xdp_xmit(dev, 1, &xdpf, XDP_XMIT_FLUSH);
	if (sent <= 0)
		return sent;
	return 0;
}

static int __bpf_tx_xdp_map(struct net_device *dev_rx, void *fwd,
			    struct bpf_map *map,
			    struct xdp_buff *xdp,
			    u32 index)
{
	int err;

	switch (map->map_type) {
	case BPF_MAP_TYPE_DEVMAP: {
		struct bpf_dtab_netdev *dst = fwd;

		err = dev_map_enqueue(dst, xdp, dev_rx);
		if (err)
			return err;
		__dev_map_insert_ctx(map, index);
		break;
	}
	case BPF_MAP_TYPE_CPUMAP: {
		struct bpf_cpu_map_entry *rcpu = fwd;

		err = cpu_map_enqueue(rcpu, xdp, dev_rx);
		if (err)
			return err;
		__cpu_map_insert_ctx(map, index);
		break;
	}
	case BPF_MAP_TYPE_XSKMAP: {
		struct xdp_sock *xs = fwd;

		err = __xsk_map_redirect(map, xdp, xs);
		return err;
	}
	default:
		break;
	}
	case BPF_MAP_TYPE_DEVMAP: {
		struct bpf_dtab_netdev *dst = fwd;

		err = dev_map_enqueue(dst, xdp, dev_rx);
		if (err)
			return err;
		__dev_map_insert_ctx(map, index);
		break;
	}
	case BPF_MAP_TYPE_CPUMAP: {
		struct bpf_cpu_map_entry *rcpu = fwd;

		err = cpu_map_enqueue(rcpu, xdp, dev_rx);
		if (err)
			return err;
		__cpu_map_insert_ctx(map, index);
		break;
	}
	case BPF_MAP_TYPE_XSKMAP: {
		struct xdp_sock *xs = fwd;

		err = __xsk_map_redirect(map, xdp, xs);
		return err;
	}
	default:
		break;
	}
	return 0;
}

void xdp_do_flush_map(void)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	struct bpf_map *map = ri->map_to_flush;

	ri->map_to_flush = NULL;
	if (map) {
		switch (map->map_type) {
		case BPF_MAP_TYPE_DEVMAP:
			__dev_map_flush(map);
			break;
		case BPF_MAP_TYPE_CPUMAP:
			__cpu_map_flush(map);
			break;
		case BPF_MAP_TYPE_XSKMAP:
			__xsk_map_flush(map);
			break;
		default:
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(xdp_do_flush_map);

static void *__xdp_map_lookup_elem(struct bpf_map *map, u32 index)
{
	switch (map->map_type) {
	case BPF_MAP_TYPE_DEVMAP:
		return __dev_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_CPUMAP:
		return __cpu_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_XSKMAP:
		return __xsk_map_lookup_elem(map, index);
	default:
		return NULL;
	}
}

void bpf_clear_redirect_map(struct bpf_map *map)
{
	struct bpf_redirect_info *ri;
	int cpu;

	for_each_possible_cpu(cpu) {
		ri = per_cpu_ptr(&bpf_redirect_info, cpu);
		/* Avoid polluting remote cacheline due to writes if
		 * not needed. Once we pass this test, we need the
		 * cmpxchg() to make sure it hasn't been changed in
		 * the meantime by remote CPU.
		 */
		if (unlikely(READ_ONCE(ri->map) == map))
			cmpxchg(&ri->map, map, NULL);
	}
}

static int xdp_do_redirect_map(struct net_device *dev, struct xdp_buff *xdp,
			       struct bpf_prog *xdp_prog, struct bpf_map *map)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (!fwd) {
		err = -EINVAL;
		goto err;
	}
	if (ri->map_to_flush && ri->map_to_flush != map)
		xdp_do_flush_map();

	err = __bpf_tx_xdp_map(dev, fwd, map, xdp, index);
	if (unlikely(err))
		goto err;

	ri->map_to_flush = map;
	_trace_xdp_redirect_map(dev, xdp_prog, fwd, map, index);
	return 0;
err:
	_trace_xdp_redirect_map_err(dev, xdp_prog, fwd, map, index, err);
	return err;
}

int xdp_do_redirect(struct net_device *dev, struct xdp_buff *xdp,
		    struct bpf_prog *xdp_prog)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	struct bpf_map *map = READ_ONCE(ri->map);
	struct net_device *fwd;
	u32 index = ri->ifindex;
	int err;

	if (map)
		return xdp_do_redirect_map(dev, xdp, xdp_prog, map);

	fwd = dev_get_by_index_rcu(dev_net(dev), index);
	ri->ifindex = 0;
	if (unlikely(!fwd)) {
		err = -EINVAL;
		goto err;
	}

	err = __bpf_tx_xdp(fwd, NULL, xdp, 0);
	if (unlikely(err))
		goto err;

	_trace_xdp_redirect(dev, xdp_prog, index);
	return 0;
err:
	_trace_xdp_redirect_err(dev, xdp_prog, index, err);
	return err;
}
EXPORT_SYMBOL_GPL(xdp_do_redirect);

static int xdp_do_generic_redirect_map(struct net_device *dev,
				       struct sk_buff *skb,
				       struct xdp_buff *xdp,
				       struct bpf_prog *xdp_prog,
				       struct bpf_map *map)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err = 0;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (unlikely(!fwd)) {
		err = -EINVAL;
		goto err;
	}

	if (map->map_type == BPF_MAP_TYPE_DEVMAP) {
		struct bpf_dtab_netdev *dst = fwd;

		err = dev_map_generic_redirect(dst, skb, xdp_prog);
		if (unlikely(err))
			goto err;
	} else if (map->map_type == BPF_MAP_TYPE_XSKMAP) {
		struct xdp_sock *xs = fwd;

		err = xsk_generic_rcv(xs, xdp);
		if (err)
			goto err;
		consume_skb(skb);
	} else {
		/* TODO: Handle BPF_MAP_TYPE_CPUMAP */
		err = -EBADRQC;
		goto err;
	}
	case BPF_MAP_TYPE_CPUMAP: {
		struct bpf_cpu_map_entry *rcpu = fwd;

		err = cpu_map_enqueue(rcpu, xdp, dev_rx);
		if (err)
			return err;
		__cpu_map_insert_ctx(map, index);
		break;
	}
	case BPF_MAP_TYPE_XSKMAP: {
		struct xdp_sock *xs = fwd;

		err = __xsk_map_redirect(map, xdp, xs);
		return err;
	}
	default:
		break;
	}
	return 0;
}

void xdp_do_flush_map(void)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	struct bpf_map *map = ri->map_to_flush;

	ri->map_to_flush = NULL;
	if (map) {
		switch (map->map_type) {
		case BPF_MAP_TYPE_DEVMAP:
			__dev_map_flush(map);
			break;
		case BPF_MAP_TYPE_CPUMAP:
			__cpu_map_flush(map);
			break;
		case BPF_MAP_TYPE_XSKMAP:
			__xsk_map_flush(map);
			break;
		default:
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(xdp_do_flush_map);

static void *__xdp_map_lookup_elem(struct bpf_map *map, u32 index)
{
	switch (map->map_type) {
	case BPF_MAP_TYPE_DEVMAP:
		return __dev_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_CPUMAP:
		return __cpu_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_XSKMAP:
		return __xsk_map_lookup_elem(map, index);
	default:
		return NULL;
	}
}

void bpf_clear_redirect_map(struct bpf_map *map)
{
	struct bpf_redirect_info *ri;
	int cpu;

	for_each_possible_cpu(cpu) {
		ri = per_cpu_ptr(&bpf_redirect_info, cpu);
		/* Avoid polluting remote cacheline due to writes if
		 * not needed. Once we pass this test, we need the
		 * cmpxchg() to make sure it hasn't been changed in
		 * the meantime by remote CPU.
		 */
		if (unlikely(READ_ONCE(ri->map) == map))
			cmpxchg(&ri->map, map, NULL);
	}
}

static int xdp_do_redirect_map(struct net_device *dev, struct xdp_buff *xdp,
			       struct bpf_prog *xdp_prog, struct bpf_map *map)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (!fwd) {
		err = -EINVAL;
		goto err;
	}
	if (ri->map_to_flush && ri->map_to_flush != map)
		xdp_do_flush_map();

	err = __bpf_tx_xdp_map(dev, fwd, map, xdp, index);
	if (unlikely(err))
		goto err;

	ri->map_to_flush = map;
	_trace_xdp_redirect_map(dev, xdp_prog, fwd, map, index);
	return 0;
err:
	_trace_xdp_redirect_map_err(dev, xdp_prog, fwd, map, index, err);
	return err;
}

int xdp_do_redirect(struct net_device *dev, struct xdp_buff *xdp,
		    struct bpf_prog *xdp_prog)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	struct bpf_map *map = READ_ONCE(ri->map);
	struct net_device *fwd;
	u32 index = ri->ifindex;
	int err;

	if (map)
		return xdp_do_redirect_map(dev, xdp, xdp_prog, map);

	fwd = dev_get_by_index_rcu(dev_net(dev), index);
	ri->ifindex = 0;
	if (unlikely(!fwd)) {
		err = -EINVAL;
		goto err;
	}

	err = __bpf_tx_xdp(fwd, NULL, xdp, 0);
	if (unlikely(err))
		goto err;

	_trace_xdp_redirect(dev, xdp_prog, index);
	return 0;
err:
	_trace_xdp_redirect_err(dev, xdp_prog, index, err);
	return err;
}
EXPORT_SYMBOL_GPL(xdp_do_redirect);

static int xdp_do_generic_redirect_map(struct net_device *dev,
				       struct sk_buff *skb,
				       struct xdp_buff *xdp,
				       struct bpf_prog *xdp_prog,
				       struct bpf_map *map)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err = 0;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (unlikely(!fwd)) {
		err = -EINVAL;
		goto err;
	}

	if (map->map_type == BPF_MAP_TYPE_DEVMAP) {
		struct bpf_dtab_netdev *dst = fwd;

		err = dev_map_generic_redirect(dst, skb, xdp_prog);
		if (unlikely(err))
			goto err;
	} else if (map->map_type == BPF_MAP_TYPE_XSKMAP) {
		struct xdp_sock *xs = fwd;

		err = xsk_generic_rcv(xs, xdp);
		if (err)
			goto err;
		consume_skb(skb);
	} else {
		/* TODO: Handle BPF_MAP_TYPE_CPUMAP */
		err = -EBADRQC;
		goto err;
	}
		switch (map->map_type) {
		case BPF_MAP_TYPE_DEVMAP:
			__dev_map_flush(map);
			break;
		case BPF_MAP_TYPE_CPUMAP:
			__cpu_map_flush(map);
			break;
		case BPF_MAP_TYPE_XSKMAP:
			__xsk_map_flush(map);
			break;
		default:
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(xdp_do_flush_map);

static void *__xdp_map_lookup_elem(struct bpf_map *map, u32 index)
{
	switch (map->map_type) {
	case BPF_MAP_TYPE_DEVMAP:
		return __dev_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_CPUMAP:
		return __cpu_map_lookup_elem(map, index);
	case BPF_MAP_TYPE_XSKMAP:
		return __xsk_map_lookup_elem(map, index);
	default:
		return NULL;
	}
}
	for_each_possible_cpu(cpu) {
		ri = per_cpu_ptr(&bpf_redirect_info, cpu);
		/* Avoid polluting remote cacheline due to writes if
		 * not needed. Once we pass this test, we need the
		 * cmpxchg() to make sure it hasn't been changed in
		 * the meantime by remote CPU.
		 */
		if (unlikely(READ_ONCE(ri->map) == map))
			cmpxchg(&ri->map, map, NULL);
	}
}

static int xdp_do_redirect_map(struct net_device *dev, struct xdp_buff *xdp,
			       struct bpf_prog *xdp_prog, struct bpf_map *map)
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (!fwd) {
		err = -EINVAL;
		goto err;
	}
	if (ri->map_to_flush && ri->map_to_flush != map)
		xdp_do_flush_map();

	err = __bpf_tx_xdp_map(dev, fwd, map, xdp, index);
	if (unlikely(err))
		goto err;

	ri->map_to_flush = map;
	_trace_xdp_redirect_map(dev, xdp_prog, fwd, map, index);
	return 0;
err:
	_trace_xdp_redirect_map_err(dev, xdp_prog, fwd, map, index, err);
	return err;
}
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	u32 index = ri->ifindex;
	void *fwd = NULL;
	int err;

	ri->ifindex = 0;
	WRITE_ONCE(ri->map, NULL);

	fwd = __xdp_map_lookup_elem(map, index);
	if (!fwd) {
		err = -EINVAL;
		goto err;
	}
{
	struct bpf_redirect_info *ri = this_cpu_ptr(&bpf_redirect_info);
	struct bpf_map *map = READ_ONCE(ri->map);
	struct net_device *fwd;
	u32 index = ri->ifindex;
	int err;

	if (map)
		return xdp_do_redirect_map(dev, xdp, xdp_prog, map);

	fwd = dev_get_by_index_rcu(dev_net(dev), index);
	ri->ifindex = 0;
	if (unlikely(!fwd)) {
		err = -EINVAL;
		goto err;
	}
static const struct bpf_func_proto bpf_setsockopt_proto = {
	.func		= bpf_setsockopt,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_ANYTHING,
	.arg4_type	= ARG_PTR_TO_MEM,
	.arg5_type	= ARG_CONST_SIZE,
};

BPF_CALL_5(bpf_getsockopt, struct bpf_sock_ops_kern *, bpf_sock,
	   int, level, int, optname, char *, optval, int, optlen)
{
	struct inet_connection_sock *icsk;
	struct sock *sk = bpf_sock->sk;
	struct tcp_sock *tp;

	if (!sk_fullsock(sk))
		goto err_clear;
#ifdef CONFIG_INET
	if (level == SOL_TCP && sk->sk_prot->getsockopt == tcp_getsockopt) {
		switch (optname) {
		case TCP_CONGESTION:
			icsk = inet_csk(sk);

			if (!icsk->icsk_ca_ops || optlen <= 1)
				goto err_clear;
			strncpy(optval, icsk->icsk_ca_ops->name, optlen);
			optval[optlen - 1] = 0;
			break;
		case TCP_SAVED_SYN:
			tp = tcp_sk(sk);

			if (optlen <= 0 || !tp->saved_syn ||
			    optlen > tp->saved_syn[0])
				goto err_clear;
			memcpy(optval, tp->saved_syn + 1, optlen);
			break;
		default:
			goto err_clear;
		}
	} else if (level == SOL_IP) {
		struct inet_sock *inet = inet_sk(sk);

		if (optlen != sizeof(int) || sk->sk_family != AF_INET)
			goto err_clear;

		/* Only some options are supported */
		switch (optname) {
		case IP_TOS:
			*((int *)optval) = (int)inet->tos;
			break;
		default:
			goto err_clear;
		}
#if IS_ENABLED(CONFIG_IPV6)
	} else if (level == SOL_IPV6) {
		struct ipv6_pinfo *np = inet6_sk(sk);

		if (optlen != sizeof(int) || sk->sk_family != AF_INET6)
			goto err_clear;

		/* Only some options are supported */
		switch (optname) {
		case IPV6_TCLASS:
			*((int *)optval) = (int)np->tclass;
			break;
		default:
			goto err_clear;
		}
#endif
	} else {
		goto err_clear;
	}
	return 0;
#endif
err_clear:
	memset(optval, 0, optlen);
	return -EINVAL;
}

static const struct bpf_func_proto bpf_getsockopt_proto = {
	.func		= bpf_getsockopt,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_ANYTHING,
	.arg4_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg5_type	= ARG_CONST_SIZE,
};

BPF_CALL_2(bpf_sock_ops_cb_flags_set, struct bpf_sock_ops_kern *, bpf_sock,
	   int, argval)
{
	struct sock *sk = bpf_sock->sk;
	int val = argval & BPF_SOCK_OPS_ALL_CB_FLAGS;

	if (!IS_ENABLED(CONFIG_INET) || !sk_fullsock(sk))
		return -EINVAL;

	if (val)
		tcp_sk(sk)->bpf_sock_ops_cb_flags = val;

	return argval & (~BPF_SOCK_OPS_ALL_CB_FLAGS);
}

static const struct bpf_func_proto bpf_sock_ops_cb_flags_set_proto = {
	.func		= bpf_sock_ops_cb_flags_set,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
};

const struct ipv6_bpf_stub *ipv6_bpf_stub __read_mostly;
EXPORT_SYMBOL_GPL(ipv6_bpf_stub);

BPF_CALL_3(bpf_bind, struct bpf_sock_addr_kern *, ctx, struct sockaddr *, addr,
	   int, addr_len)
{
#ifdef CONFIG_INET
	struct sock *sk = ctx->sk;
	int err;

	/* Binding to port can be expensive so it's prohibited in the helper.
	 * Only binding to IP is supported.
	 */
	err = -EINVAL;
	if (addr->sa_family == AF_INET) {
		if (addr_len < sizeof(struct sockaddr_in))
			return err;
		if (((struct sockaddr_in *)addr)->sin_port != htons(0))
			return err;
		return __inet_bind(sk, addr, addr_len, true, false);
#if IS_ENABLED(CONFIG_IPV6)
	} else if (addr->sa_family == AF_INET6) {
		if (addr_len < SIN6_LEN_RFC2133)
			return err;
		if (((struct sockaddr_in6 *)addr)->sin6_port != htons(0))
			return err;
		/* ipv6_bpf_stub cannot be NULL, since it's called from
		 * bpf_cgroup_inet6_connect hook and ipv6 is already loaded
		 */
		return ipv6_bpf_stub->inet6_bind(sk, addr, addr_len, true, false);
#endif /* CONFIG_IPV6 */
	}
#endif /* CONFIG_INET */

	return -EAFNOSUPPORT;
}

static const struct bpf_func_proto bpf_bind_proto = {
	.func		= bpf_bind,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_PTR_TO_MEM,
	.arg3_type	= ARG_CONST_SIZE,
};

#ifdef CONFIG_XFRM
BPF_CALL_5(bpf_skb_get_xfrm_state, struct sk_buff *, skb, u32, index,
	   struct bpf_xfrm_state *, to, u32, size, u64, flags)
{
	const struct sec_path *sp = skb_sec_path(skb);
	const struct xfrm_state *x;

	if (!sp || unlikely(index >= sp->len || flags))
		goto err_clear;

	x = sp->xvec[index];

	if (unlikely(size != sizeof(struct bpf_xfrm_state)))
		goto err_clear;

	to->reqid = x->props.reqid;
	to->spi = x->id.spi;
	to->family = x->props.family;
	to->ext = 0;

	if (to->family == AF_INET6) {
		memcpy(to->remote_ipv6, x->props.saddr.a6,
		       sizeof(to->remote_ipv6));
	} else {
		to->remote_ipv4 = x->props.saddr.a4;
		memset(&to->remote_ipv6[1], 0, sizeof(__u32) * 3);
	}

	return 0;
err_clear:
	memset(to, 0, size);
	return -EINVAL;
}

static const struct bpf_func_proto bpf_skb_get_xfrm_state_proto = {
	.func		= bpf_skb_get_xfrm_state,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
	.arg5_type	= ARG_ANYTHING,
};
#endif

#if IS_ENABLED(CONFIG_INET) || IS_ENABLED(CONFIG_IPV6)
static int bpf_fib_set_fwd_params(struct bpf_fib_lookup *params,
				  const struct neighbour *neigh,
				  const struct net_device *dev)
{
	memcpy(params->dmac, neigh->ha, ETH_ALEN);
	memcpy(params->smac, dev->dev_addr, ETH_ALEN);
	params->h_vlan_TCI = 0;
	params->h_vlan_proto = 0;
	params->ifindex = dev->ifindex;

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_INET)
static int bpf_ipv4_fib_lookup(struct net *net, struct bpf_fib_lookup *params,
			       u32 flags, bool check_mtu)
{
	struct in_device *in_dev;
	struct neighbour *neigh;
	struct net_device *dev;
	struct fib_result res;
	struct fib_nh *nh;
	struct flowi4 fl4;
	int err;
	u32 mtu;

	dev = dev_get_by_index_rcu(net, params->ifindex);
	if (unlikely(!dev))
		return -ENODEV;

	/* verify forwarding is enabled on this interface */
	in_dev = __in_dev_get_rcu(dev);
	if (unlikely(!in_dev || !IN_DEV_FORWARD(in_dev)))
		return BPF_FIB_LKUP_RET_FWD_DISABLED;

	if (flags & BPF_FIB_LOOKUP_OUTPUT) {
		fl4.flowi4_iif = 1;
		fl4.flowi4_oif = params->ifindex;
	} else {
		fl4.flowi4_iif = params->ifindex;
		fl4.flowi4_oif = 0;
	}
	fl4.flowi4_tos = params->tos & IPTOS_RT_MASK;
	fl4.flowi4_scope = RT_SCOPE_UNIVERSE;
	fl4.flowi4_flags = 0;

	fl4.flowi4_proto = params->l4_protocol;
	fl4.daddr = params->ipv4_dst;
	fl4.saddr = params->ipv4_src;
	fl4.fl4_sport = params->sport;
	fl4.fl4_dport = params->dport;

	if (flags & BPF_FIB_LOOKUP_DIRECT) {
		u32 tbid = l3mdev_fib_table_rcu(dev) ? : RT_TABLE_MAIN;
		struct fib_table *tb;

		tb = fib_get_table(net, tbid);
		if (unlikely(!tb))
			return BPF_FIB_LKUP_RET_NOT_FWDED;

		err = fib_table_lookup(tb, &fl4, &res, FIB_LOOKUP_NOREF);
	} else {
		fl4.flowi4_mark = 0;
		fl4.flowi4_secid = 0;
		fl4.flowi4_tun_key.tun_id = 0;
		fl4.flowi4_uid = sock_net_uid(net, NULL);

		err = fib_lookup(net, &fl4, &res, FIB_LOOKUP_NOREF);
	}

	if (err) {
		/* map fib lookup errors to RTN_ type */
		if (err == -EINVAL)
			return BPF_FIB_LKUP_RET_BLACKHOLE;
		if (err == -EHOSTUNREACH)
			return BPF_FIB_LKUP_RET_UNREACHABLE;
		if (err == -EACCES)
			return BPF_FIB_LKUP_RET_PROHIBIT;

		return BPF_FIB_LKUP_RET_NOT_FWDED;
	}

	if (res.type != RTN_UNICAST)
		return BPF_FIB_LKUP_RET_NOT_FWDED;

	if (res.fi->fib_nhs > 1)
		fib_select_path(net, &res, &fl4, NULL);

	if (check_mtu) {
		mtu = ip_mtu_from_fib_result(&res, params->ipv4_dst);
		if (params->tot_len > mtu)
			return BPF_FIB_LKUP_RET_FRAG_NEEDED;
	}

	nh = &res.fi->fib_nh[res.nh_sel];

	/* do not handle lwt encaps right now */
	if (nh->nh_lwtstate)
		return BPF_FIB_LKUP_RET_UNSUPP_LWT;

	dev = nh->nh_dev;
	if (nh->nh_gw)
		params->ipv4_dst = nh->nh_gw;

	params->rt_metric = res.fi->fib_priority;

	/* xdp and cls_bpf programs are run in RCU-bh so
	 * rcu_read_lock_bh is not needed here
	 */
	neigh = __ipv4_neigh_lookup_noref(dev, (__force u32)params->ipv4_dst);
	if (!neigh)
		return BPF_FIB_LKUP_RET_NO_NEIGH;

	return bpf_fib_set_fwd_params(params, neigh, dev);
}
#endif

#if IS_ENABLED(CONFIG_IPV6)
static int bpf_ipv6_fib_lookup(struct net *net, struct bpf_fib_lookup *params,
			       u32 flags, bool check_mtu)
{
	struct in6_addr *src = (struct in6_addr *) params->ipv6_src;
	struct in6_addr *dst = (struct in6_addr *) params->ipv6_dst;
	struct neighbour *neigh;
	struct net_device *dev;
	struct inet6_dev *idev;
	struct fib6_info *f6i;
	struct flowi6 fl6;
	int strict = 0;
	int oif;
	u32 mtu;

	/* link local addresses are never forwarded */
	if (rt6_need_strict(dst) || rt6_need_strict(src))
		return BPF_FIB_LKUP_RET_NOT_FWDED;

	dev = dev_get_by_index_rcu(net, params->ifindex);
	if (unlikely(!dev))
		return -ENODEV;

	idev = __in6_dev_get_safely(dev);
	if (unlikely(!idev || !net->ipv6.devconf_all->forwarding))
		return BPF_FIB_LKUP_RET_FWD_DISABLED;

	if (flags & BPF_FIB_LOOKUP_OUTPUT) {
		fl6.flowi6_iif = 1;
		oif = fl6.flowi6_oif = params->ifindex;
	} else {
		oif = fl6.flowi6_iif = params->ifindex;
		fl6.flowi6_oif = 0;
		strict = RT6_LOOKUP_F_HAS_SADDR;
	}
	fl6.flowlabel = params->flowinfo;
	fl6.flowi6_scope = 0;
	fl6.flowi6_flags = 0;
	fl6.mp_hash = 0;

	fl6.flowi6_proto = params->l4_protocol;
	fl6.daddr = *dst;
	fl6.saddr = *src;
	fl6.fl6_sport = params->sport;
	fl6.fl6_dport = params->dport;

	if (flags & BPF_FIB_LOOKUP_DIRECT) {
		u32 tbid = l3mdev_fib_table_rcu(dev) ? : RT_TABLE_MAIN;
		struct fib6_table *tb;

		tb = ipv6_stub->fib6_get_table(net, tbid);
		if (unlikely(!tb))
			return BPF_FIB_LKUP_RET_NOT_FWDED;

		f6i = ipv6_stub->fib6_table_lookup(net, tb, oif, &fl6, strict);
	} else {
		fl6.flowi6_mark = 0;
		fl6.flowi6_secid = 0;
		fl6.flowi6_tun_key.tun_id = 0;
		fl6.flowi6_uid = sock_net_uid(net, NULL);

		f6i = ipv6_stub->fib6_lookup(net, oif, &fl6, strict);
	}

	if (unlikely(IS_ERR_OR_NULL(f6i) || f6i == net->ipv6.fib6_null_entry))
		return BPF_FIB_LKUP_RET_NOT_FWDED;

	if (unlikely(f6i->fib6_flags & RTF_REJECT)) {
		switch (f6i->fib6_type) {
		case RTN_BLACKHOLE:
			return BPF_FIB_LKUP_RET_BLACKHOLE;
		case RTN_UNREACHABLE:
			return BPF_FIB_LKUP_RET_UNREACHABLE;
		case RTN_PROHIBIT:
			return BPF_FIB_LKUP_RET_PROHIBIT;
		default:
			return BPF_FIB_LKUP_RET_NOT_FWDED;
		}
	}

	if (f6i->fib6_type != RTN_UNICAST)
		return BPF_FIB_LKUP_RET_NOT_FWDED;

	if (f6i->fib6_nsiblings && fl6.flowi6_oif == 0)
		f6i = ipv6_stub->fib6_multipath_select(net, f6i, &fl6,
						       fl6.flowi6_oif, NULL,
						       strict);

	if (check_mtu) {
		mtu = ipv6_stub->ip6_mtu_from_fib6(f6i, dst, src);
		if (params->tot_len > mtu)
			return BPF_FIB_LKUP_RET_FRAG_NEEDED;
	}

	if (f6i->fib6_nh.nh_lwtstate)
		return BPF_FIB_LKUP_RET_UNSUPP_LWT;

	if (f6i->fib6_flags & RTF_GATEWAY)
		*dst = f6i->fib6_nh.nh_gw;

	dev = f6i->fib6_nh.nh_dev;
	params->rt_metric = f6i->fib6_metric;

	/* xdp and cls_bpf programs are run in RCU-bh so rcu_read_lock_bh is
	 * not needed here. Can not use __ipv6_neigh_lookup_noref here
	 * because we need to get nd_tbl via the stub
	 */
	neigh = ___neigh_lookup_noref(ipv6_stub->nd_tbl, neigh_key_eq128,
				      ndisc_hashfn, dst, dev);
	if (!neigh)
		return BPF_FIB_LKUP_RET_NO_NEIGH;

	return bpf_fib_set_fwd_params(params, neigh, dev);
}
#endif

BPF_CALL_4(bpf_xdp_fib_lookup, struct xdp_buff *, ctx,
	   struct bpf_fib_lookup *, params, int, plen, u32, flags)
{
	if (plen < sizeof(*params))
		return -EINVAL;

	if (flags & ~(BPF_FIB_LOOKUP_DIRECT | BPF_FIB_LOOKUP_OUTPUT))
		return -EINVAL;

	switch (params->family) {
#if IS_ENABLED(CONFIG_INET)
	case AF_INET:
		return bpf_ipv4_fib_lookup(dev_net(ctx->rxq->dev), params,
					   flags, true);
#endif
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		return bpf_ipv6_fib_lookup(dev_net(ctx->rxq->dev), params,
					   flags, true);
#endif
	}
	return -EAFNOSUPPORT;
}

static const struct bpf_func_proto bpf_xdp_fib_lookup_proto = {
	.func		= bpf_xdp_fib_lookup,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type      = ARG_PTR_TO_CTX,
	.arg2_type      = ARG_PTR_TO_MEM,
	.arg3_type      = ARG_CONST_SIZE,
	.arg4_type	= ARG_ANYTHING,
};

BPF_CALL_4(bpf_skb_fib_lookup, struct sk_buff *, skb,
	   struct bpf_fib_lookup *, params, int, plen, u32, flags)
{
	struct net *net = dev_net(skb->dev);
	int rc = -EAFNOSUPPORT;

	if (plen < sizeof(*params))
		return -EINVAL;

	if (flags & ~(BPF_FIB_LOOKUP_DIRECT | BPF_FIB_LOOKUP_OUTPUT))
		return -EINVAL;

	switch (params->family) {
#if IS_ENABLED(CONFIG_INET)
	case AF_INET:
		rc = bpf_ipv4_fib_lookup(net, params, flags, false);
		break;
#endif
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		rc = bpf_ipv6_fib_lookup(net, params, flags, false);
		break;
#endif
	}

	if (!rc) {
		struct net_device *dev;

		dev = dev_get_by_index_rcu(net, params->ifindex);
		if (!is_skb_forwardable(dev, skb))
			rc = BPF_FIB_LKUP_RET_FRAG_NEEDED;
	}

	return rc;
}

static const struct bpf_func_proto bpf_skb_fib_lookup_proto = {
	.func		= bpf_skb_fib_lookup,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type      = ARG_PTR_TO_CTX,
	.arg2_type      = ARG_PTR_TO_MEM,
	.arg3_type      = ARG_CONST_SIZE,
	.arg4_type	= ARG_ANYTHING,
};

#if IS_ENABLED(CONFIG_IPV6_SEG6_BPF)
static int bpf_push_seg6_encap(struct sk_buff *skb, u32 type, void *hdr, u32 len)
{
	int err;
	struct ipv6_sr_hdr *srh = (struct ipv6_sr_hdr *)hdr;

	if (!seg6_validate_srh(srh, len))
		return -EINVAL;

	switch (type) {
	case BPF_LWT_ENCAP_SEG6_INLINE:
		if (skb->protocol != htons(ETH_P_IPV6))
			return -EBADMSG;

		err = seg6_do_srh_inline(skb, srh);
		break;
	case BPF_LWT_ENCAP_SEG6:
		skb_reset_inner_headers(skb);
		skb->encapsulation = 1;
		err = seg6_do_srh_encap(skb, srh, IPPROTO_IPV6);
		break;
	default:
		return -EINVAL;
	}

	bpf_compute_data_pointers(skb);
	if (err)
		return err;

	ipv6_hdr(skb)->payload_len = htons(skb->len - sizeof(struct ipv6hdr));
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	return seg6_lookup_nexthop(skb, NULL, 0);
}
#endif /* CONFIG_IPV6_SEG6_BPF */

BPF_CALL_4(bpf_lwt_push_encap, struct sk_buff *, skb, u32, type, void *, hdr,
	   u32, len)
{
	switch (type) {
#if IS_ENABLED(CONFIG_IPV6_SEG6_BPF)
	case BPF_LWT_ENCAP_SEG6:
	case BPF_LWT_ENCAP_SEG6_INLINE:
		return bpf_push_seg6_encap(skb, type, hdr, len);
#endif
	default:
		return -EINVAL;
	}
}

static const struct bpf_func_proto bpf_lwt_push_encap_proto = {
	.func		= bpf_lwt_push_encap,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_MEM,
	.arg4_type	= ARG_CONST_SIZE
};

#if IS_ENABLED(CONFIG_IPV6_SEG6_BPF)
BPF_CALL_4(bpf_lwt_seg6_store_bytes, struct sk_buff *, skb, u32, offset,
	   const void *, from, u32, len)
{
	struct seg6_bpf_srh_state *srh_state =
		this_cpu_ptr(&seg6_bpf_srh_states);
	struct ipv6_sr_hdr *srh = srh_state->srh;
	void *srh_tlvs, *srh_end, *ptr;
	int srhoff = 0;

	if (srh == NULL)
		return -EINVAL;

	srh_tlvs = (void *)((char *)srh + ((srh->first_segment + 1) << 4));
	srh_end = (void *)((char *)srh + sizeof(*srh) + srh_state->hdrlen);

	ptr = skb->data + offset;
	if (ptr >= srh_tlvs && ptr + len <= srh_end)
		srh_state->valid = false;
	else if (ptr < (void *)&srh->flags ||
		 ptr + len > (void *)&srh->segments)
		return -EFAULT;

	if (unlikely(bpf_try_make_writable(skb, offset + len)))
		return -EFAULT;
	if (ipv6_find_hdr(skb, &srhoff, IPPROTO_ROUTING, NULL, NULL) < 0)
		return -EINVAL;
	srh_state->srh = (struct ipv6_sr_hdr *)(skb->data + srhoff);

	memcpy(skb->data + offset, from, len);
	return 0;
}

static const struct bpf_func_proto bpf_lwt_seg6_store_bytes_proto = {
	.func		= bpf_lwt_seg6_store_bytes,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_MEM,
	.arg4_type	= ARG_CONST_SIZE
};

static void bpf_update_srh_state(struct sk_buff *skb)
{
	struct seg6_bpf_srh_state *srh_state =
		this_cpu_ptr(&seg6_bpf_srh_states);
	int srhoff = 0;

	if (ipv6_find_hdr(skb, &srhoff, IPPROTO_ROUTING, NULL, NULL) < 0) {
		srh_state->srh = NULL;
	} else {
		srh_state->srh = (struct ipv6_sr_hdr *)(skb->data + srhoff);
		srh_state->hdrlen = srh_state->srh->hdrlen << 3;
		srh_state->valid = true;
	}
}

BPF_CALL_4(bpf_lwt_seg6_action, struct sk_buff *, skb,
	   u32, action, void *, param, u32, param_len)
{
	struct seg6_bpf_srh_state *srh_state =
		this_cpu_ptr(&seg6_bpf_srh_states);
	int hdroff = 0;
	int err;

	switch (action) {
	case SEG6_LOCAL_ACTION_END_X:
		if (!seg6_bpf_has_valid_srh(skb))
			return -EBADMSG;
		if (param_len != sizeof(struct in6_addr))
			return -EINVAL;
		return seg6_lookup_nexthop(skb, (struct in6_addr *)param, 0);
	case SEG6_LOCAL_ACTION_END_T:
		if (!seg6_bpf_has_valid_srh(skb))
			return -EBADMSG;
		if (param_len != sizeof(int))
			return -EINVAL;
		return seg6_lookup_nexthop(skb, NULL, *(int *)param);
	case SEG6_LOCAL_ACTION_END_DT6:
		if (!seg6_bpf_has_valid_srh(skb))
			return -EBADMSG;
		if (param_len != sizeof(int))
			return -EINVAL;

		if (ipv6_find_hdr(skb, &hdroff, IPPROTO_IPV6, NULL, NULL) < 0)
			return -EBADMSG;
		if (!pskb_pull(skb, hdroff))
			return -EBADMSG;

		skb_postpull_rcsum(skb, skb_network_header(skb), hdroff);
		skb_reset_network_header(skb);
		skb_reset_transport_header(skb);
		skb->encapsulation = 0;

		bpf_compute_data_pointers(skb);
		bpf_update_srh_state(skb);
		return seg6_lookup_nexthop(skb, NULL, *(int *)param);
	case SEG6_LOCAL_ACTION_END_B6:
		if (srh_state->srh && !seg6_bpf_has_valid_srh(skb))
			return -EBADMSG;
		err = bpf_push_seg6_encap(skb, BPF_LWT_ENCAP_SEG6_INLINE,
					  param, param_len);
		if (!err)
			bpf_update_srh_state(skb);

		return err;
	case SEG6_LOCAL_ACTION_END_B6_ENCAP:
		if (srh_state->srh && !seg6_bpf_has_valid_srh(skb))
			return -EBADMSG;
		err = bpf_push_seg6_encap(skb, BPF_LWT_ENCAP_SEG6,
					  param, param_len);
		if (!err)
			bpf_update_srh_state(skb);

		return err;
	default:
		return -EINVAL;
	}
}

static const struct bpf_func_proto bpf_lwt_seg6_action_proto = {
	.func		= bpf_lwt_seg6_action,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_MEM,
	.arg4_type	= ARG_CONST_SIZE
};

BPF_CALL_3(bpf_lwt_seg6_adjust_srh, struct sk_buff *, skb, u32, offset,
	   s32, len)
{
	struct seg6_bpf_srh_state *srh_state =
		this_cpu_ptr(&seg6_bpf_srh_states);
	struct ipv6_sr_hdr *srh = srh_state->srh;
	void *srh_end, *srh_tlvs, *ptr;
	struct ipv6hdr *hdr;
	int srhoff = 0;
	int ret;

	if (unlikely(srh == NULL))
		return -EINVAL;

	srh_tlvs = (void *)((unsigned char *)srh + sizeof(*srh) +
			((srh->first_segment + 1) << 4));
	srh_end = (void *)((unsigned char *)srh + sizeof(*srh) +
			srh_state->hdrlen);
	ptr = skb->data + offset;

	if (unlikely(ptr < srh_tlvs || ptr > srh_end))
		return -EFAULT;
	if (unlikely(len < 0 && (void *)((char *)ptr - len) > srh_end))
		return -EFAULT;

	if (len > 0) {
		ret = skb_cow_head(skb, len);
		if (unlikely(ret < 0))
			return ret;

		ret = bpf_skb_net_hdr_push(skb, offset, len);
	} else {
		ret = bpf_skb_net_hdr_pop(skb, offset, -1 * len);
	}

	bpf_compute_data_pointers(skb);
	if (unlikely(ret < 0))
		return ret;

	hdr = (struct ipv6hdr *)skb->data;
	hdr->payload_len = htons(skb->len - sizeof(struct ipv6hdr));

	if (ipv6_find_hdr(skb, &srhoff, IPPROTO_ROUTING, NULL, NULL) < 0)
		return -EINVAL;
	srh_state->srh = (struct ipv6_sr_hdr *)(skb->data + srhoff);
	srh_state->hdrlen += len;
	srh_state->valid = false;
	return 0;
}

static const struct bpf_func_proto bpf_lwt_seg6_adjust_srh_proto = {
	.func		= bpf_lwt_seg6_adjust_srh,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_ANYTHING,
};
#endif /* CONFIG_IPV6_SEG6_BPF */

bool bpf_helper_changes_pkt_data(void *func)
{
	if (func == bpf_skb_vlan_push ||
	    func == bpf_skb_vlan_pop ||
	    func == bpf_skb_store_bytes ||
	    func == bpf_skb_change_proto ||
	    func == bpf_skb_change_head ||
	    func == sk_skb_change_head ||
	    func == bpf_skb_change_tail ||
	    func == sk_skb_change_tail ||
	    func == bpf_skb_adjust_room ||
	    func == bpf_skb_pull_data ||
	    func == sk_skb_pull_data ||
	    func == bpf_clone_redirect ||
	    func == bpf_l3_csum_replace ||
	    func == bpf_l4_csum_replace ||
	    func == bpf_xdp_adjust_head ||
	    func == bpf_xdp_adjust_meta ||
	    func == bpf_msg_pull_data ||
	    func == bpf_xdp_adjust_tail ||
#if IS_ENABLED(CONFIG_IPV6_SEG6_BPF)
	    func == bpf_lwt_seg6_store_bytes ||
	    func == bpf_lwt_seg6_adjust_srh ||
	    func == bpf_lwt_seg6_action ||
#endif
	    func == bpf_lwt_push_encap)
		return true;

	return false;
}

static const struct bpf_func_proto *
bpf_base_func_proto(enum bpf_func_id func_id)
{
	switch (func_id) {
	case BPF_FUNC_map_lookup_elem:
		return &bpf_map_lookup_elem_proto;
	case BPF_FUNC_map_update_elem:
		return &bpf_map_update_elem_proto;
	case BPF_FUNC_map_delete_elem:
		return &bpf_map_delete_elem_proto;
	case BPF_FUNC_get_prandom_u32:
		return &bpf_get_prandom_u32_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_raw_smp_processor_id_proto;
	case BPF_FUNC_get_numa_node_id:
		return &bpf_get_numa_node_id_proto;
	case BPF_FUNC_tail_call:
		return &bpf_tail_call_proto;
	case BPF_FUNC_ktime_get_ns:
		return &bpf_ktime_get_ns_proto;
	case BPF_FUNC_trace_printk:
		if (capable(CAP_SYS_ADMIN))
			return bpf_get_trace_printk_proto();
		/* else: fall through */
	default:
		return NULL;
	}
}

static const struct bpf_func_proto *
sock_filter_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	/* inet and inet6 sockets are created in a process
	 * context so there is always a valid uid/gid
	 */
	case BPF_FUNC_get_current_uid_gid:
		return &bpf_get_current_uid_gid_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
sock_addr_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	/* inet and inet6 sockets are created in a process
	 * context so there is always a valid uid/gid
	 */
	case BPF_FUNC_get_current_uid_gid:
		return &bpf_get_current_uid_gid_proto;
	case BPF_FUNC_bind:
		switch (prog->expected_attach_type) {
		case BPF_CGROUP_INET4_CONNECT:
		case BPF_CGROUP_INET6_CONNECT:
			return &bpf_bind_proto;
		default:
			return NULL;
		}
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_sock_addr_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
sk_filter_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_load_bytes_relative:
		return &bpf_skb_load_bytes_relative_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_proto;
	case BPF_FUNC_get_socket_uid:
		return &bpf_get_socket_uid_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
cg_skb_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return sk_filter_func_proto(func_id, prog);
	}
}

static const struct bpf_func_proto *
tc_cls_act_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_store_bytes:
		return &bpf_skb_store_bytes_proto;
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_load_bytes_relative:
		return &bpf_skb_load_bytes_relative_proto;
	case BPF_FUNC_skb_pull_data:
		return &bpf_skb_pull_data_proto;
	case BPF_FUNC_csum_diff:
		return &bpf_csum_diff_proto;
	case BPF_FUNC_csum_update:
		return &bpf_csum_update_proto;
	case BPF_FUNC_l3_csum_replace:
		return &bpf_l3_csum_replace_proto;
	case BPF_FUNC_l4_csum_replace:
		return &bpf_l4_csum_replace_proto;
	case BPF_FUNC_clone_redirect:
		return &bpf_clone_redirect_proto;
	case BPF_FUNC_get_cgroup_classid:
		return &bpf_get_cgroup_classid_proto;
	case BPF_FUNC_skb_vlan_push:
		return &bpf_skb_vlan_push_proto;
	case BPF_FUNC_skb_vlan_pop:
		return &bpf_skb_vlan_pop_proto;
	case BPF_FUNC_skb_change_proto:
		return &bpf_skb_change_proto_proto;
	case BPF_FUNC_skb_change_type:
		return &bpf_skb_change_type_proto;
	case BPF_FUNC_skb_adjust_room:
		return &bpf_skb_adjust_room_proto;
	case BPF_FUNC_skb_change_tail:
		return &bpf_skb_change_tail_proto;
	case BPF_FUNC_skb_get_tunnel_key:
		return &bpf_skb_get_tunnel_key_proto;
	case BPF_FUNC_skb_set_tunnel_key:
		return bpf_get_skb_set_tunnel_proto(func_id);
	case BPF_FUNC_skb_get_tunnel_opt:
		return &bpf_skb_get_tunnel_opt_proto;
	case BPF_FUNC_skb_set_tunnel_opt:
		return bpf_get_skb_set_tunnel_proto(func_id);
	case BPF_FUNC_redirect:
		return &bpf_redirect_proto;
	case BPF_FUNC_get_route_realm:
		return &bpf_get_route_realm_proto;
	case BPF_FUNC_get_hash_recalc:
		return &bpf_get_hash_recalc_proto;
	case BPF_FUNC_set_hash_invalid:
		return &bpf_set_hash_invalid_proto;
	case BPF_FUNC_set_hash:
		return &bpf_set_hash_proto;
	case BPF_FUNC_perf_event_output:
		return &bpf_skb_event_output_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_skb_under_cgroup:
		return &bpf_skb_under_cgroup_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_proto;
	case BPF_FUNC_get_socket_uid:
		return &bpf_get_socket_uid_proto;
	case BPF_FUNC_fib_lookup:
		return &bpf_skb_fib_lookup_proto;
#ifdef CONFIG_XFRM
	case BPF_FUNC_skb_get_xfrm_state:
		return &bpf_skb_get_xfrm_state_proto;
#endif
#ifdef CONFIG_SOCK_CGROUP_DATA
	case BPF_FUNC_skb_cgroup_id:
		return &bpf_skb_cgroup_id_proto;
	case BPF_FUNC_skb_ancestor_cgroup_id:
		return &bpf_skb_ancestor_cgroup_id_proto;
#endif
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
xdp_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_perf_event_output:
		return &bpf_xdp_event_output_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_csum_diff:
		return &bpf_csum_diff_proto;
	case BPF_FUNC_xdp_adjust_head:
		return &bpf_xdp_adjust_head_proto;
	case BPF_FUNC_xdp_adjust_meta:
		return &bpf_xdp_adjust_meta_proto;
	case BPF_FUNC_redirect:
		return &bpf_xdp_redirect_proto;
	case BPF_FUNC_redirect_map:
		return &bpf_xdp_redirect_map_proto;
	case BPF_FUNC_xdp_adjust_tail:
		return &bpf_xdp_adjust_tail_proto;
	case BPF_FUNC_fib_lookup:
		return &bpf_xdp_fib_lookup_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
sock_ops_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_setsockopt:
		return &bpf_setsockopt_proto;
	case BPF_FUNC_getsockopt:
		return &bpf_getsockopt_proto;
	case BPF_FUNC_sock_ops_cb_flags_set:
		return &bpf_sock_ops_cb_flags_set_proto;
	case BPF_FUNC_sock_map_update:
		return &bpf_sock_map_update_proto;
	case BPF_FUNC_sock_hash_update:
		return &bpf_sock_hash_update_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_sock_ops_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
sk_msg_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_msg_redirect_map:
		return &bpf_msg_redirect_map_proto;
	case BPF_FUNC_msg_redirect_hash:
		return &bpf_msg_redirect_hash_proto;
	case BPF_FUNC_msg_apply_bytes:
		return &bpf_msg_apply_bytes_proto;
	case BPF_FUNC_msg_cork_bytes:
		return &bpf_msg_cork_bytes_proto;
	case BPF_FUNC_msg_pull_data:
		return &bpf_msg_pull_data_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
sk_skb_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_store_bytes:
		return &bpf_skb_store_bytes_proto;
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &sk_skb_pull_data_proto;
	case BPF_FUNC_skb_change_tail:
		return &sk_skb_change_tail_proto;
	case BPF_FUNC_skb_change_head:
		return &sk_skb_change_head_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_proto;
	case BPF_FUNC_get_socket_uid:
		return &bpf_get_socket_uid_proto;
	case BPF_FUNC_sk_redirect_map:
		return &bpf_sk_redirect_map_proto;
	case BPF_FUNC_sk_redirect_hash:
		return &bpf_sk_redirect_hash_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
lwt_out_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &bpf_skb_pull_data_proto;
	case BPF_FUNC_csum_diff:
		return &bpf_csum_diff_proto;
	case BPF_FUNC_get_cgroup_classid:
		return &bpf_get_cgroup_classid_proto;
	case BPF_FUNC_get_route_realm:
		return &bpf_get_route_realm_proto;
	case BPF_FUNC_get_hash_recalc:
		return &bpf_get_hash_recalc_proto;
	case BPF_FUNC_perf_event_output:
		return &bpf_skb_event_output_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_skb_under_cgroup:
		return &bpf_skb_under_cgroup_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
lwt_in_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_lwt_push_encap:
		return &bpf_lwt_push_encap_proto;
	default:
		return lwt_out_func_proto(func_id, prog);
	}
}

static const struct bpf_func_proto *
lwt_xmit_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_get_tunnel_key:
		return &bpf_skb_get_tunnel_key_proto;
	case BPF_FUNC_skb_set_tunnel_key:
		return bpf_get_skb_set_tunnel_proto(func_id);
	case BPF_FUNC_skb_get_tunnel_opt:
		return &bpf_skb_get_tunnel_opt_proto;
	case BPF_FUNC_skb_set_tunnel_opt:
		return bpf_get_skb_set_tunnel_proto(func_id);
	case BPF_FUNC_redirect:
		return &bpf_redirect_proto;
	case BPF_FUNC_clone_redirect:
		return &bpf_clone_redirect_proto;
	case BPF_FUNC_skb_change_tail:
		return &bpf_skb_change_tail_proto;
	case BPF_FUNC_skb_change_head:
		return &bpf_skb_change_head_proto;
	case BPF_FUNC_skb_store_bytes:
		return &bpf_skb_store_bytes_proto;
	case BPF_FUNC_csum_update:
		return &bpf_csum_update_proto;
	case BPF_FUNC_l3_csum_replace:
		return &bpf_l3_csum_replace_proto;
	case BPF_FUNC_l4_csum_replace:
		return &bpf_l4_csum_replace_proto;
	case BPF_FUNC_set_hash_invalid:
		return &bpf_set_hash_invalid_proto;
	default:
		return lwt_out_func_proto(func_id, prog);
	}
}

static const struct bpf_func_proto *
lwt_seg6local_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
#if IS_ENABLED(CONFIG_IPV6_SEG6_BPF)
	case BPF_FUNC_lwt_seg6_store_bytes:
		return &bpf_lwt_seg6_store_bytes_proto;
	case BPF_FUNC_lwt_seg6_action:
		return &bpf_lwt_seg6_action_proto;
	case BPF_FUNC_lwt_seg6_adjust_srh:
		return &bpf_lwt_seg6_adjust_srh_proto;
#endif
	default:
		return lwt_out_func_proto(func_id, prog);
	}
}

static bool bpf_skb_is_valid_access(int off, int size, enum bpf_access_type type,
				    const struct bpf_prog *prog,
				    struct bpf_insn_access_aux *info)
{
	const int size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct __sk_buff))
		return false;

	/* The verifier guarantees that size > 0. */
	if (off % size != 0)
		return false;

	switch (off) {
	case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
		if (off + size > offsetofend(struct __sk_buff, cb[4]))
			return false;
		break;
	case bpf_ctx_range_till(struct __sk_buff, remote_ip6[0], remote_ip6[3]):
	case bpf_ctx_range_till(struct __sk_buff, local_ip6[0], local_ip6[3]):
	case bpf_ctx_range_till(struct __sk_buff, remote_ip4, remote_ip4):
	case bpf_ctx_range_till(struct __sk_buff, local_ip4, local_ip4):
	case bpf_ctx_range(struct __sk_buff, data):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, data_end):
		if (size != size_default)
			return false;
		break;
	default:
		/* Only narrow read access allowed for now. */
		if (type == BPF_WRITE) {
			if (size != size_default)
				return false;
		} else {
			bpf_ctx_record_field_size(info, size_default);
			if (!bpf_ctx_narrow_access_ok(off, size, size_default))
				return false;
		}
	}

	return true;
}

static bool sk_filter_is_valid_access(int off, int size,
				      enum bpf_access_type type,
				      const struct bpf_prog *prog,
				      struct bpf_insn_access_aux *info)
{
	switch (off) {
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, data_end):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static bool lwt_is_valid_access(int off, int size,
				enum bpf_access_type type,
				const struct bpf_prog *prog,
				struct bpf_insn_access_aux *info)
{
	switch (off) {
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, mark):
		case bpf_ctx_range(struct __sk_buff, priority):
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	}

	switch (off) {
	case bpf_ctx_range(struct __sk_buff, data):
		info->reg_type = PTR_TO_PACKET;
		break;
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

/* Attach type specific accesses */
static bool __sock_filter_check_attach_type(int off,
					    enum bpf_access_type access_type,
					    enum bpf_attach_type attach_type)
{
	switch (off) {
	case offsetof(struct bpf_sock, bound_dev_if):
	case offsetof(struct bpf_sock, mark):
	case offsetof(struct bpf_sock, priority):
		switch (attach_type) {
		case BPF_CGROUP_INET_SOCK_CREATE:
			goto full_access;
		default:
			return false;
		}
	case bpf_ctx_range(struct bpf_sock, src_ip4):
		switch (attach_type) {
		case BPF_CGROUP_INET4_POST_BIND:
			goto read_only;
		default:
			return false;
		}
	case bpf_ctx_range_till(struct bpf_sock, src_ip6[0], src_ip6[3]):
		switch (attach_type) {
		case BPF_CGROUP_INET6_POST_BIND:
			goto read_only;
		default:
			return false;
		}
	case bpf_ctx_range(struct bpf_sock, src_port):
		switch (attach_type) {
		case BPF_CGROUP_INET4_POST_BIND:
		case BPF_CGROUP_INET6_POST_BIND:
			goto read_only;
		default:
			return false;
		}
	}
read_only:
	return access_type == BPF_READ;
full_access:
	return true;
}

static bool __sock_filter_check_size(int off, int size,
				     struct bpf_insn_access_aux *info)
{
	const int size_default = sizeof(__u32);

	switch (off) {
	case bpf_ctx_range(struct bpf_sock, src_ip4):
	case bpf_ctx_range_till(struct bpf_sock, src_ip6[0], src_ip6[3]):
		bpf_ctx_record_field_size(info, size_default);
		return bpf_ctx_narrow_access_ok(off, size, size_default);
	}

	return size == size_default;
}

static bool sock_filter_is_valid_access(int off, int size,
					enum bpf_access_type type,
					const struct bpf_prog *prog,
					struct bpf_insn_access_aux *info)
{
	if (off < 0 || off >= sizeof(struct bpf_sock))
		return false;
	if (off % size != 0)
		return false;
	if (!__sock_filter_check_attach_type(off, type,
					     prog->expected_attach_type))
		return false;
	if (!__sock_filter_check_size(off, size, info))
		return false;
	return true;
}

static int bpf_unclone_prologue(struct bpf_insn *insn_buf, bool direct_write,
				const struct bpf_prog *prog, int drop_verdict)
{
	struct bpf_insn *insn = insn_buf;

	if (!direct_write)
		return 0;

	/* if (!skb->cloned)
	 *       goto start;
	 *
	 * (Fast-path, otherwise approximation that we might be
	 *  a clone, do the rest in helper.)
	 */
	*insn++ = BPF_LDX_MEM(BPF_B, BPF_REG_6, BPF_REG_1, CLONED_OFFSET());
	*insn++ = BPF_ALU32_IMM(BPF_AND, BPF_REG_6, CLONED_MASK);
	*insn++ = BPF_JMP_IMM(BPF_JEQ, BPF_REG_6, 0, 7);

	/* ret = bpf_skb_pull_data(skb, 0); */
	*insn++ = BPF_MOV64_REG(BPF_REG_6, BPF_REG_1);
	*insn++ = BPF_ALU64_REG(BPF_XOR, BPF_REG_2, BPF_REG_2);
	*insn++ = BPF_RAW_INSN(BPF_JMP | BPF_CALL, 0, 0, 0,
			       BPF_FUNC_skb_pull_data);
	/* if (!ret)
	 *      goto restore;
	 * return TC_ACT_SHOT;
	 */
	*insn++ = BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 2);
	*insn++ = BPF_ALU32_IMM(BPF_MOV, BPF_REG_0, drop_verdict);
	*insn++ = BPF_EXIT_INSN();

	/* restore: */
	*insn++ = BPF_MOV64_REG(BPF_REG_1, BPF_REG_6);
	/* start: */
	*insn++ = prog->insnsi[0];

	return insn - insn_buf;
}

static int bpf_gen_ld_abs(const struct bpf_insn *orig,
			  struct bpf_insn *insn_buf)
{
	bool indirect = BPF_MODE(orig->code) == BPF_IND;
	struct bpf_insn *insn = insn_buf;

	/* We're guaranteed here that CTX is in R6. */
	*insn++ = BPF_MOV64_REG(BPF_REG_1, BPF_REG_CTX);
	if (!indirect) {
		*insn++ = BPF_MOV64_IMM(BPF_REG_2, orig->imm);
	} else {
		*insn++ = BPF_MOV64_REG(BPF_REG_2, orig->src_reg);
		if (orig->imm)
			*insn++ = BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, orig->imm);
	}

	switch (BPF_SIZE(orig->code)) {
	case BPF_B:
		*insn++ = BPF_EMIT_CALL(bpf_skb_load_helper_8_no_cache);
		break;
	case BPF_H:
		*insn++ = BPF_EMIT_CALL(bpf_skb_load_helper_16_no_cache);
		break;
	case BPF_W:
		*insn++ = BPF_EMIT_CALL(bpf_skb_load_helper_32_no_cache);
		break;
	}

	*insn++ = BPF_JMP_IMM(BPF_JSGE, BPF_REG_0, 0, 2);
	*insn++ = BPF_ALU32_REG(BPF_XOR, BPF_REG_0, BPF_REG_0);
	*insn++ = BPF_EXIT_INSN();

	return insn - insn_buf;
}

static int tc_cls_act_prologue(struct bpf_insn *insn_buf, bool direct_write,
			       const struct bpf_prog *prog)
{
	return bpf_unclone_prologue(insn_buf, direct_write, prog, TC_ACT_SHOT);
}

static bool tc_cls_act_is_valid_access(int off, int size,
				       enum bpf_access_type type,
				       const struct bpf_prog *prog,
				       struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, mark):
		case bpf_ctx_range(struct __sk_buff, tc_index):
		case bpf_ctx_range(struct __sk_buff, priority):
		case bpf_ctx_range(struct __sk_buff, tc_classid):
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	}

	switch (off) {
	case bpf_ctx_range(struct __sk_buff, data):
		info->reg_type = PTR_TO_PACKET;
		break;
	case bpf_ctx_range(struct __sk_buff, data_meta):
		info->reg_type = PTR_TO_PACKET_META;
		break;
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static bool __is_valid_xdp_access(int off, int size)
{
	if (off < 0 || off >= sizeof(struct xdp_md))
		return false;
	if (off % size != 0)
		return false;
	if (size != sizeof(__u32))
		return false;

	return true;
}

static bool xdp_is_valid_access(int off, int size,
				enum bpf_access_type type,
				const struct bpf_prog *prog,
				struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE) {
		if (bpf_prog_is_dev_bound(prog->aux)) {
			switch (off) {
			case offsetof(struct xdp_md, rx_queue_index):
				return __is_valid_xdp_access(off, size);
			}
		}
		return false;
	}

	switch (off) {
	case offsetof(struct xdp_md, data):
		info->reg_type = PTR_TO_PACKET;
		break;
	case offsetof(struct xdp_md, data_meta):
		info->reg_type = PTR_TO_PACKET_META;
		break;
	case offsetof(struct xdp_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	}

	return __is_valid_xdp_access(off, size);
}

void bpf_warn_invalid_xdp_action(u32 act)
{
	const u32 act_max = XDP_REDIRECT;

	WARN_ONCE(1, "%s XDP return value %u, expect packet loss!\n",
		  act > act_max ? "Illegal" : "Driver unsupported",
		  act);
}
EXPORT_SYMBOL_GPL(bpf_warn_invalid_xdp_action);

static bool sock_addr_is_valid_access(int off, int size,
				      enum bpf_access_type type,
				      const struct bpf_prog *prog,
				      struct bpf_insn_access_aux *info)
{
	const int size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct bpf_sock_addr))
		return false;
	if (off % size != 0)
		return false;

	/* Disallow access to IPv6 fields from IPv4 contex and vise
	 * versa.
	 */
	switch (off) {
	case bpf_ctx_range(struct bpf_sock_addr, user_ip4):
		switch (prog->expected_attach_type) {
		case BPF_CGROUP_INET4_BIND:
		case BPF_CGROUP_INET4_CONNECT:
		case BPF_CGROUP_UDP4_SENDMSG:
			break;
		default:
			return false;
		}
		break;
	case bpf_ctx_range_till(struct bpf_sock_addr, user_ip6[0], user_ip6[3]):
		switch (prog->expected_attach_type) {
		case BPF_CGROUP_INET6_BIND:
		case BPF_CGROUP_INET6_CONNECT:
		case BPF_CGROUP_UDP6_SENDMSG:
			break;
		default:
			return false;
		}
		break;
	case bpf_ctx_range(struct bpf_sock_addr, msg_src_ip4):
		switch (prog->expected_attach_type) {
		case BPF_CGROUP_UDP4_SENDMSG:
			break;
		default:
			return false;
		}
		break;
	case bpf_ctx_range_till(struct bpf_sock_addr, msg_src_ip6[0],
				msg_src_ip6[3]):
		switch (prog->expected_attach_type) {
		case BPF_CGROUP_UDP6_SENDMSG:
			break;
		default:
			return false;
		}
		break;
	}

	switch (off) {
	case bpf_ctx_range(struct bpf_sock_addr, user_ip4):
	case bpf_ctx_range_till(struct bpf_sock_addr, user_ip6[0], user_ip6[3]):
	case bpf_ctx_range(struct bpf_sock_addr, msg_src_ip4):
	case bpf_ctx_range_till(struct bpf_sock_addr, msg_src_ip6[0],
				msg_src_ip6[3]):
		/* Only narrow read access allowed for now. */
		if (type == BPF_READ) {
			bpf_ctx_record_field_size(info, size_default);
			if (!bpf_ctx_narrow_access_ok(off, size, size_default))
				return false;
		} else {
			if (size != size_default)
				return false;
		}
		break;
	case bpf_ctx_range(struct bpf_sock_addr, user_port):
		if (size != size_default)
			return false;
		break;
	default:
		if (type == BPF_READ) {
			if (size != size_default)
				return false;
		} else {
			return false;
		}
	}

	return true;
}

static bool sock_ops_is_valid_access(int off, int size,
				     enum bpf_access_type type,
				     const struct bpf_prog *prog,
				     struct bpf_insn_access_aux *info)
{
	const int size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct bpf_sock_ops))
		return false;

	/* The verifier guarantees that size > 0. */
	if (off % size != 0)
		return false;

	if (type == BPF_WRITE) {
		switch (off) {
		case offsetof(struct bpf_sock_ops, reply):
		case offsetof(struct bpf_sock_ops, sk_txhash):
			if (size != size_default)
				return false;
			break;
		default:
			return false;
		}
	} else {
		switch (off) {
		case bpf_ctx_range_till(struct bpf_sock_ops, bytes_received,
					bytes_acked):
			if (size != sizeof(__u64))
				return false;
			break;
		default:
			if (size != size_default)
				return false;
			break;
		}
	}

	return true;
}

static int sk_skb_prologue(struct bpf_insn *insn_buf, bool direct_write,
			   const struct bpf_prog *prog)
{
	return bpf_unclone_prologue(insn_buf, direct_write, prog, SK_DROP);
}

static bool sk_skb_is_valid_access(int off, int size,
				   enum bpf_access_type type,
				   const struct bpf_prog *prog,
				   struct bpf_insn_access_aux *info)
{
	switch (off) {
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, tc_index):
		case bpf_ctx_range(struct __sk_buff, priority):
			break;
		default:
			return false;
		}
	}

	switch (off) {
	case bpf_ctx_range(struct __sk_buff, mark):
		return false;
	case bpf_ctx_range(struct __sk_buff, data):
		info->reg_type = PTR_TO_PACKET;
		break;
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static bool sk_msg_is_valid_access(int off, int size,
				   enum bpf_access_type type,
				   const struct bpf_prog *prog,
				   struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE)
		return false;

	switch (off) {
	case offsetof(struct sk_msg_md, data):
		info->reg_type = PTR_TO_PACKET;
		if (size != sizeof(__u64))
			return false;
		break;
	case offsetof(struct sk_msg_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		if (size != sizeof(__u64))
			return false;
		break;
	default:
		if (size != sizeof(__u32))
			return false;
	}

	if (off < 0 || off >= sizeof(struct sk_msg_md))
		return false;
	if (off % size != 0)
		return false;

	return true;
}

static u32 bpf_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, len):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, len, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, protocol):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, protocol, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_proto):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_proto, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, priority):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, skb_iif, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 1);
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, hash):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, hash, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, mark):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, pkt_type):
		*target_size = 1;
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->src_reg,
				      PKT_TYPE_OFFSET());
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, PKT_TYPE_MAX);
#ifdef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 5);
#endif
		break;

	case offsetof(struct __sk_buff, queue_mapping):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, queue_mapping, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_present):
	case offsetof(struct __sk_buff, vlan_tci):
		BUILD_BUG_ON(VLAN_TAG_PRESENT != 0x1000);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_tci, 2,
						     target_size));
		if (si->off == offsetof(struct __sk_buff, vlan_tci)) {
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg,
						~VLAN_TAG_PRESENT);
		} else {
			*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 12);
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, 1);
		}
		break;

	case offsetof(struct __sk_buff, cb[0]) ...
	     offsetofend(struct __sk_buff, cb[4]) - 1:
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, data) < 20);
		BUILD_BUG_ON((offsetof(struct sk_buff, cb) +
			      offsetof(struct qdisc_skb_cb, data)) %
			     sizeof(__u64));

		prog->cb_access = 1;
		off  = si->off;
		off -= offsetof(struct __sk_buff, cb[0]);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, data);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_classid):
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, tc_classid) != 2);

		off  = si->off;
		off -= offsetof(struct __sk_buff, tc_classid);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, tc_classid);
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, data));
		break;

	case offsetof(struct __sk_buff, data_meta):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_meta);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_meta);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_index):
#ifdef CONFIG_NET_SCHED
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
#else
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_MOV64_REG(si->dst_reg, si->dst_reg);
		else
			*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, napi_id):
#if defined(CONFIG_NET_RX_BUSY_POLL)
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, napi_id, 4,
						     target_size));
		*insn++ = BPF_JMP_IMM(BPF_JGE, si->dst_reg, MIN_NAPI_ID, 1);
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#else
		*target_size = 4;
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_family,
						     2, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_daddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_rcv_saddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip6[0]) ...
	     offsetof(struct __sk_buff, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, remote_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, local_ip6[0]) ...
	     offsetof(struct __sk_buff, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, local_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_dport,
						     2, target_size));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct __sk_buff, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 sock_filter_convert_ctx_access(enum bpf_access_type type,
					  const struct bpf_insn *si,
					  struct bpf_insn *insn_buf,
					  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock, bound_dev_if):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_bound_dev_if) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_bound_dev_if));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_bound_dev_if));
		break;

	case offsetof(struct bpf_sock, mark):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_mark) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_mark));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_mark));
		break;

	case offsetof(struct bpf_sock, priority):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_priority) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_priority));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_priority));
		break;

	case offsetof(struct bpf_sock, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_family));
		break;

	case offsetof(struct bpf_sock, type):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock, protocol):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock, src_ip4):
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_rcv_saddr,
				       FIELD_SIZEOF(struct sock_common,
						    skc_rcv_saddr),
				       target_size));
		break;

	case bpf_ctx_range_till(struct bpf_sock, src_ip6[0], src_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		off = si->off;
		off -= offsetof(struct bpf_sock, src_ip6[0]);
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(
				struct sock_common,
				skc_v6_rcv_saddr.s6_addr32[0],
				FIELD_SIZEOF(struct sock_common,
					     skc_v6_rcv_saddr.s6_addr32[0]),
				target_size) + off);
#else
		(void)off;
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock, src_port):
		*insn++ = BPF_LDX_MEM(
			BPF_FIELD_SIZEOF(struct sock_common, skc_num),
			si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_num,
				       FIELD_SIZEOF(struct sock_common,
						    skc_num),
				       target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 tc_cls_act_convert_ctx_access(enum bpf_access_type type,
					 const struct bpf_insn *si,
					 struct bpf_insn *insn_buf,
					 struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 xdp_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct xdp_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data));
		break;
	case offsetof(struct xdp_md, data_meta):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_meta),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_meta));
		break;
	case offsetof(struct xdp_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_end));
		break;
	case offsetof(struct xdp_md, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_rxq_info, dev),
				      si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct net_device, ifindex));
		break;
	case offsetof(struct xdp_md, rx_queue_index):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info,
					       queue_index));
		break;
	}

	return insn - insn_buf;
}

/* SOCK_ADDR_LOAD_NESTED_FIELD() loads Nested Field S.F.NF where S is type of
 * context Structure, F is Field in context structure that contains a pointer
 * to Nested Structure of type NS that has the field NF.
 *
 * SIZE encodes the load size (BPF_B, BPF_H, etc). It's up to caller to make
 * sure that SIZE is not greater than actual size of S.F.NF.
 *
 * If offset OFF is provided, the load happens from that offset relative to
 * offset of NF.
 */
#define SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF)	       \
	do {								       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), si->dst_reg,     \
				      si->src_reg, offsetof(S, F));	       \
		*insn++ = BPF_LDX_MEM(					       \
			SIZE, si->dst_reg, si->dst_reg,			       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
	} while (0)

#define SOCK_ADDR_LOAD_NESTED_FIELD(S, NS, F, NF)			       \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF,		       \
					     BPF_FIELD_SIZEOF(NS, NF), 0)

/* SOCK_ADDR_STORE_NESTED_FIELD_OFF() has semantic similar to
 * SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF() but for store operation.
 *
 * It doesn't support SIZE argument though since narrow stores are not
 * supported for now.
 *
 * In addition it uses Temporary Field TF (member of struct S) as the 3rd
 * "register" since two registers available in convert_ctx_access are not
 * enough: we can't override neither SRC, since it contains value to store, nor
 * DST since it contains pointer to context that may be used by later
 * instructions. But we need a temporary place to save pointer to nested
 * structure whose field we want to store to.
 */
#define SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF, TF)		       \
	do {								       \
		int tmp_reg = BPF_REG_9;				       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, tmp_reg,	       \
				      offsetof(S, TF));			       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), tmp_reg,	       \
				      si->dst_reg, offsetof(S, F));	       \
		*insn++ = BPF_STX_MEM(					       \
			BPF_FIELD_SIZEOF(NS, NF), tmp_reg, si->src_reg,	       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
		*insn++ = BPF_LDX_MEM(BPF_DW, tmp_reg, si->dst_reg,	       \
				      offsetof(S, TF));			       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF, \
						      TF)		       \
	do {								       \
		if (type == BPF_WRITE) {				       \
			SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF,    \
							 TF);		       \
		} else {						       \
			SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(		       \
				S, NS, F, NF, SIZE, OFF);  \
		}							       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(S, NS, F, NF, TF)		       \
	SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(			       \
		S, NS, F, NF, BPF_FIELD_SIZEOF(NS, NF), 0, TF)

static u32 sock_addr_convert_ctx_access(enum bpf_access_type type,
					const struct bpf_insn *si,
					struct bpf_insn *insn_buf,
					struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_addr, user_family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sockaddr, uaddr, sa_family);
		break;

	case offsetof(struct bpf_sock_addr, user_ip4):
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in, uaddr,
			sin_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, user_ip6[0], user_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, user_ip6[0]);
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in6, uaddr,
			sin6_addr.s6_addr32[0], BPF_SIZE(si->code), off,
			tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, user_port):
		/* To get port we need to know sa_family first and then treat
		 * sockaddr as either sockaddr_in or sockaddr_in6.
		 * Though we can simplify since port field has same offset and
		 * size in both structures.
		 * Here we check this invariant and use just one of the
		 * structures if it's true.
		 */
		BUILD_BUG_ON(offsetof(struct sockaddr_in, sin_port) !=
			     offsetof(struct sockaddr_in6, sin6_port));
		BUILD_BUG_ON(FIELD_SIZEOF(struct sockaddr_in, sin_port) !=
			     FIELD_SIZEOF(struct sockaddr_in6, sin6_port));
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(struct bpf_sock_addr_kern,
						     struct sockaddr_in6, uaddr,
						     sin6_port, tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sock, sk, sk_family);
		break;

	case offsetof(struct bpf_sock_addr, type):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, protocol):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, msg_src_ip4):
		/* Treat t_ctx as struct in_addr for msg_src_ip4. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in_addr, t_ctx,
			s_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, msg_src_ip6[0],
				msg_src_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, msg_src_ip6[0]);
		/* Treat t_ctx as struct in6_addr for msg_src_ip6. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in6_addr, t_ctx,
			s6_addr32[0], BPF_SIZE(si->code), off, tmp_reg);
		break;
	}

	return insn - insn_buf;
}

static u32 sock_ops_convert_ctx_access(enum bpf_access_type type,
				       const struct bpf_insn *si,
				       struct bpf_insn *insn_buf,
				       struct bpf_prog *prog,
				       u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_ops, op) ...
	     offsetof(struct bpf_sock_ops, replylong[3]):
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, op) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, op));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, reply) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, reply));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, replylong) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, replylong));
		off = si->off;
		off -= offsetof(struct bpf_sock_ops, op);
		off += offsetof(struct bpf_sock_ops_kern, op);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		break;

	case offsetof(struct bpf_sock_ops, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct bpf_sock_ops, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;

	case offsetof(struct bpf_sock_ops, is_fullsock):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern,
						is_fullsock),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern,
					       is_fullsock));
		break;

	case offsetof(struct bpf_sock_ops, state):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_state) != 1);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_state));
		break;

	case offsetof(struct bpf_sock_ops, rtt_min):
		BUILD_BUG_ON(FIELD_SIZEOF(struct tcp_sock, rtt_min) !=
			     sizeof(struct minmax));
		BUILD_BUG_ON(sizeof(struct minmax) <
			     sizeof(struct minmax_sample));

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct tcp_sock, rtt_min) +
				      FIELD_SIZEOF(struct minmax_sample, t));
		break;

/* Helper macro for adding read access to tcp_sock or sock fields. */
#define SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 2);	      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(OBJ,		      \
						       OBJ_FIELD),	      \
				      si->dst_reg, si->dst_reg,		      \
				      offsetof(OBJ, OBJ_FIELD));	      \
	} while (0)

/* Helper macro for adding write access to tcp_sock or sock fields.
 * The macro is called with two registers, dst_reg which contains a pointer
 * to ctx (context) and src_reg which contains the value that should be
 * stored. However, we need an additional register since we cannot overwrite
 * dst_reg because it may be used later in the program.
 * Instead we "borrow" one of the other register. We first save its value
 * into a new (temp) field in bpf_sock_ops_kern, use it, and then restore
 * it at the end of the macro.
 */
#define SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		int reg = BPF_REG_9;					      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, reg, 0, 2);		      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_STX_MEM(BPF_FIELD_SIZEOF(OBJ, OBJ_FIELD),	      \
				      reg, si->src_reg,			      \
				      offsetof(OBJ, OBJ_FIELD));	      \
		*insn++ = BPF_LDX_MEM(BPF_DW, reg, si->dst_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
	} while (0)

#define SOCK_OPS_GET_OR_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ, TYPE)	      \
	do {								      \
		if (TYPE == BPF_WRITE)					      \
			SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
		else							      \
			SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
	} while (0)

	case offsetof(struct bpf_sock_ops, snd_cwnd):
		SOCK_OPS_GET_FIELD(snd_cwnd, snd_cwnd, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, srtt_us):
		SOCK_OPS_GET_FIELD(srtt_us, srtt_us, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bpf_sock_ops_cb_flags):
		SOCK_OPS_GET_FIELD(bpf_sock_ops_cb_flags, bpf_sock_ops_cb_flags,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_ssthresh):
		SOCK_OPS_GET_FIELD(snd_ssthresh, snd_ssthresh, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rcv_nxt):
		SOCK_OPS_GET_FIELD(rcv_nxt, rcv_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_nxt):
		SOCK_OPS_GET_FIELD(snd_nxt, snd_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_una):
		SOCK_OPS_GET_FIELD(snd_una, snd_una, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, mss_cache):
		SOCK_OPS_GET_FIELD(mss_cache, mss_cache, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, ecn_flags):
		SOCK_OPS_GET_FIELD(ecn_flags, ecn_flags, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_delivered):
		SOCK_OPS_GET_FIELD(rate_delivered, rate_delivered,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_interval_us):
		SOCK_OPS_GET_FIELD(rate_interval_us, rate_interval_us,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, packets_out):
		SOCK_OPS_GET_FIELD(packets_out, packets_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, retrans_out):
		SOCK_OPS_GET_FIELD(retrans_out, retrans_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, total_retrans):
		SOCK_OPS_GET_FIELD(total_retrans, total_retrans,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_in):
		SOCK_OPS_GET_FIELD(segs_in, segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_in):
		SOCK_OPS_GET_FIELD(data_segs_in, data_segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_out):
		SOCK_OPS_GET_FIELD(segs_out, segs_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_out):
		SOCK_OPS_GET_FIELD(data_segs_out, data_segs_out,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, lost_out):
		SOCK_OPS_GET_FIELD(lost_out, lost_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sacked_out):
		SOCK_OPS_GET_FIELD(sacked_out, sacked_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sk_txhash):
		SOCK_OPS_GET_OR_SET_FIELD(sk_txhash, sk_txhash,
					  struct sock, type);
		break;

	case offsetof(struct bpf_sock_ops, bytes_received):
		SOCK_OPS_GET_FIELD(bytes_received, bytes_received,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bytes_acked):
		SOCK_OPS_GET_FIELD(bytes_acked, bytes_acked, struct tcp_sock);
		break;

	}
	return insn - insn_buf;
}

static u32 sk_skb_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct tcp_skb_cb, bpf.data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 sk_msg_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
#if IS_ENABLED(CONFIG_IPV6)
	int off;
#endif

	switch (si->off) {
	case offsetof(struct sk_msg_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data));
		break;
	case offsetof(struct sk_msg_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data_end));
		break;
	case offsetof(struct sk_msg_md, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct sk_msg_md, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct sk_msg_md, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct sk_msg_md, remote_ip6[0]) ...
	     offsetof(struct sk_msg_md, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, local_ip6[0]) ...
	     offsetof(struct sk_msg_md, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct sk_msg_md, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_filter_verifier_ops = {
	.get_func_proto		= sk_filter_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops sk_filter_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops tc_cls_act_verifier_ops = {
	.get_func_proto		= tc_cls_act_func_proto,
	.is_valid_access	= tc_cls_act_is_valid_access,
	.convert_ctx_access	= tc_cls_act_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops tc_cls_act_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops xdp_verifier_ops = {
	.get_func_proto		= xdp_func_proto,
	.is_valid_access	= xdp_is_valid_access,
	.convert_ctx_access	= xdp_convert_ctx_access,
};

const struct bpf_prog_ops xdp_prog_ops = {
	.test_run		= bpf_prog_test_run_xdp,
};

const struct bpf_verifier_ops cg_skb_verifier_ops = {
	.get_func_proto		= cg_skb_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops cg_skb_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_in_verifier_ops = {
	.get_func_proto		= lwt_in_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_in_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_out_verifier_ops = {
	.get_func_proto		= lwt_out_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_out_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_xmit_verifier_ops = {
	.get_func_proto		= lwt_xmit_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
};

const struct bpf_prog_ops lwt_xmit_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_seg6local_verifier_ops = {
	.get_func_proto		= lwt_seg6local_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_seg6local_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops cg_sock_verifier_ops = {
	.get_func_proto		= sock_filter_func_proto,
	.is_valid_access	= sock_filter_is_valid_access,
	.convert_ctx_access	= sock_filter_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_prog_ops = {
};

const struct bpf_verifier_ops cg_sock_addr_verifier_ops = {
	.get_func_proto		= sock_addr_func_proto,
	.is_valid_access	= sock_addr_is_valid_access,
	.convert_ctx_access	= sock_addr_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_addr_prog_ops = {
};

const struct bpf_verifier_ops sock_ops_verifier_ops = {
	.get_func_proto		= sock_ops_func_proto,
	.is_valid_access	= sock_ops_is_valid_access,
	.convert_ctx_access	= sock_ops_convert_ctx_access,
};

const struct bpf_prog_ops sock_ops_prog_ops = {
};

const struct bpf_verifier_ops sk_skb_verifier_ops = {
	.get_func_proto		= sk_skb_func_proto,
	.is_valid_access	= sk_skb_is_valid_access,
	.convert_ctx_access	= sk_skb_convert_ctx_access,
	.gen_prologue		= sk_skb_prologue,
};

const struct bpf_prog_ops sk_skb_prog_ops = {
};

const struct bpf_verifier_ops sk_msg_verifier_ops = {
	.get_func_proto		= sk_msg_func_proto,
	.is_valid_access	= sk_msg_is_valid_access,
	.convert_ctx_access	= sk_msg_convert_ctx_access,
};

const struct bpf_prog_ops sk_msg_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;

	if (sock_flag(sk, SOCK_FILTER_LOCKED))
		return -EPERM;

	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (filter) {
		RCU_INIT_POINTER(sk->sk_filter, NULL);
		sk_filter_uncharge(sk, filter);
		ret = 0;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sk_detach_filter);

int sk_get_filter(struct sock *sk, struct sock_filter __user *ubuf,
		  unsigned int len)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	int ret = 0;

	lock_sock(sk);
	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (!filter)
		goto out;

	/* We're copying the filter that has been originally attached,
	 * so no conversion/decode needed anymore. eBPF programs that
	 * have no original program cannot be dumped through this.
	 */
	ret = -EACCES;
	fprog = filter->prog->orig_prog;
	if (!fprog)
		goto out;

	ret = fprog->len;
	if (!len)
		/* User space only enquires number of filter blocks. */
		goto out;

	ret = -EINVAL;
	if (len < fprog->len)
		goto out;

	ret = -EFAULT;
	if (copy_to_user(ubuf, fprog->filter, bpf_classic_proglen(fprog)))
		goto out;

	/* Instead of bytes, the API requests to return the number
	 * of filter blocks.
	 */
	ret = fprog->len;
out:
	release_sock(sk);
	return ret;
}

#ifdef CONFIG_INET
struct sk_reuseport_kern {
	struct sk_buff *skb;
	struct sock *sk;
	struct sock *selected_sk;
	void *data_end;
	u32 hash;
	u32 reuseport_id;
	bool bind_inany;
};

static void bpf_init_reuseport_kern(struct sk_reuseport_kern *reuse_kern,
				    struct sock_reuseport *reuse,
				    struct sock *sk, struct sk_buff *skb,
				    u32 hash)
{
	reuse_kern->skb = skb;
	reuse_kern->sk = sk;
	reuse_kern->selected_sk = NULL;
	reuse_kern->data_end = skb->data + skb_headlen(skb);
	reuse_kern->hash = hash;
	reuse_kern->reuseport_id = reuse->reuseport_id;
	reuse_kern->bind_inany = reuse->bind_inany;
}

struct sock *bpf_run_sk_reuseport(struct sock_reuseport *reuse, struct sock *sk,
				  struct bpf_prog *prog, struct sk_buff *skb,
				  u32 hash)
{
	struct sk_reuseport_kern reuse_kern;
	enum sk_action action;

	bpf_init_reuseport_kern(&reuse_kern, reuse, sk, skb, hash);
	action = BPF_PROG_RUN(prog, &reuse_kern);

	if (action == SK_PASS)
		return reuse_kern.selected_sk;
	else
		return ERR_PTR(-ECONNREFUSED);
}

BPF_CALL_4(sk_select_reuseport, struct sk_reuseport_kern *, reuse_kern,
	   struct bpf_map *, map, void *, key, u32, flags)
{
	struct sock_reuseport *reuse;
	struct sock *selected_sk;

	selected_sk = map->ops->map_lookup_elem(map, key);
	if (!selected_sk)
		return -ENOENT;

	reuse = rcu_dereference(selected_sk->sk_reuseport_cb);
	if (!reuse)
		/* selected_sk is unhashed (e.g. by close()) after the
		 * above map_lookup_elem().  Treat selected_sk has already
		 * been removed from the map.
		 */
		return -ENOENT;

	if (unlikely(reuse->reuseport_id != reuse_kern->reuseport_id)) {
		struct sock *sk;

		if (unlikely(!reuse_kern->reuseport_id))
			/* There is a small race between adding the
			 * sk to the map and setting the
			 * reuse_kern->reuseport_id.
			 * Treat it as the sk has not been added to
			 * the bpf map yet.
			 */
			return -ENOENT;

		sk = reuse_kern->sk;
		if (sk->sk_protocol != selected_sk->sk_protocol)
			return -EPROTOTYPE;
		else if (sk->sk_family != selected_sk->sk_family)
			return -EAFNOSUPPORT;

		/* Catch all. Likely bound to a different sockaddr. */
		return -EBADFD;
	}

	reuse_kern->selected_sk = selected_sk;

	return 0;
}

static const struct bpf_func_proto sk_select_reuseport_proto = {
	.func           = sk_select_reuseport,
	.gpl_only       = false,
	.ret_type       = RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type      = ARG_CONST_MAP_PTR,
	.arg3_type      = ARG_PTR_TO_MAP_KEY,
	.arg4_type	= ARG_ANYTHING,
};

BPF_CALL_4(sk_reuseport_load_bytes,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len)
{
	return ____bpf_skb_load_bytes(reuse_kern->skb, offset, to, len);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_proto = {
	.func		= sk_reuseport_load_bytes,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
};

BPF_CALL_5(sk_reuseport_load_bytes_relative,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len, u32, start_header)
{
	return ____bpf_skb_load_bytes_relative(reuse_kern->skb, offset, to,
					       len, start_header);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_relative_proto = {
	.func		= sk_reuseport_load_bytes_relative,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
	.arg5_type	= ARG_ANYTHING,
};

static const struct bpf_func_proto *
sk_reuseport_func_proto(enum bpf_func_id func_id,
			const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_sk_select_reuseport:
		return &sk_select_reuseport_proto;
	case BPF_FUNC_skb_load_bytes:
		return &sk_reuseport_load_bytes_proto;
	case BPF_FUNC_skb_load_bytes_relative:
		return &sk_reuseport_load_bytes_relative_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static bool
sk_reuseport_is_valid_access(int off, int size,
			     enum bpf_access_type type,
			     const struct bpf_prog *prog,
			     struct bpf_insn_access_aux *info)
{
	const u32 size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct sk_reuseport_md) ||
	    off % size || type != BPF_READ)
		return false;

	switch (off) {
	case offsetof(struct sk_reuseport_md, data):
		info->reg_type = PTR_TO_PACKET;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, hash):
		return size == size_default;

	/* Fields that allow narrowing */
	case offsetof(struct sk_reuseport_md, eth_protocol):
		if (size < FIELD_SIZEOF(struct sk_buff, protocol))
			return false;
		/* fall through */
	case offsetof(struct sk_reuseport_md, ip_protocol):
	case offsetof(struct sk_reuseport_md, bind_inany):
	case offsetof(struct sk_reuseport_md, len):
		bpf_ctx_record_field_size(info, size_default);
		return bpf_ctx_narrow_access_ok(off, size, size_default);

	default:
		return false;
	}
}

#define SK_REUSEPORT_LOAD_FIELD(F) ({					\
	*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_reuseport_kern, F), \
			      si->dst_reg, si->src_reg,			\
			      bpf_target_off(struct sk_reuseport_kern, F, \
					     FIELD_SIZEOF(struct sk_reuseport_kern, F), \
					     target_size));		\
	})

#define SK_REUSEPORT_LOAD_SKB_FIELD(SKB_FIELD)				\
	SOCK_ADDR_LOAD_NESTED_FIELD(struct sk_reuseport_kern,		\
				    struct sk_buff,			\
				    skb,				\
				    SKB_FIELD)

#define SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(SK_FIELD, BPF_SIZE, EXTRA_OFF) \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(struct sk_reuseport_kern,	\
					     struct sock,		\
					     sk,			\
					     SK_FIELD, BPF_SIZE, EXTRA_OFF)

static u32 sk_reuseport_convert_ctx_access(enum bpf_access_type type,
					   const struct bpf_insn *si,
					   struct bpf_insn *insn_buf,
					   struct bpf_prog *prog,
					   u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct sk_reuseport_md, data):
		SK_REUSEPORT_LOAD_SKB_FIELD(data);
		break;

	case offsetof(struct sk_reuseport_md, len):
		SK_REUSEPORT_LOAD_SKB_FIELD(len);
		break;

	case offsetof(struct sk_reuseport_md, eth_protocol):
		SK_REUSEPORT_LOAD_SKB_FIELD(protocol);
		break;

	case offsetof(struct sk_reuseport_md, ip_protocol):
		BUILD_BUG_ON(HWEIGHT32(SK_FL_PROTO_MASK) != BITS_PER_BYTE);
		SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(__sk_flags_offset,
						    BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		/* SK_FL_PROTO_MASK and SK_FL_PROTO_SHIFT are endian
		 * aware.  No further narrowing or masking is needed.
		 */
		*target_size = 1;
		break;

	case offsetof(struct sk_reuseport_md, data_end):
		SK_REUSEPORT_LOAD_FIELD(data_end);
		break;

	case offsetof(struct sk_reuseport_md, hash):
		SK_REUSEPORT_LOAD_FIELD(hash);
		break;

	case offsetof(struct sk_reuseport_md, bind_inany):
		SK_REUSEPORT_LOAD_FIELD(bind_inany);
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_reuseport_verifier_ops = {
	.get_func_proto		= sk_reuseport_func_proto,
	.is_valid_access	= sk_reuseport_is_valid_access,
	.convert_ctx_access	= sk_reuseport_convert_ctx_access,
};

const struct bpf_prog_ops sk_reuseport_prog_ops = {
};
#endif /* CONFIG_INET */
	switch (func_id) {
	case BPF_FUNC_skb_store_bytes:
		return &bpf_skb_store_bytes_proto;
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &sk_skb_pull_data_proto;
	case BPF_FUNC_skb_change_tail:
		return &sk_skb_change_tail_proto;
	case BPF_FUNC_skb_change_head:
		return &sk_skb_change_head_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_proto;
	case BPF_FUNC_get_socket_uid:
		return &bpf_get_socket_uid_proto;
	case BPF_FUNC_sk_redirect_map:
		return &bpf_sk_redirect_map_proto;
	case BPF_FUNC_sk_redirect_hash:
		return &bpf_sk_redirect_hash_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
lwt_out_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &bpf_skb_pull_data_proto;
	case BPF_FUNC_csum_diff:
		return &bpf_csum_diff_proto;
	case BPF_FUNC_get_cgroup_classid:
		return &bpf_get_cgroup_classid_proto;
	case BPF_FUNC_get_route_realm:
		return &bpf_get_route_realm_proto;
	case BPF_FUNC_get_hash_recalc:
		return &bpf_get_hash_recalc_proto;
	case BPF_FUNC_perf_event_output:
		return &bpf_skb_event_output_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_skb_under_cgroup:
		return &bpf_skb_under_cgroup_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
	case bpf_ctx_range(struct __sk_buff, data_end):
		if (size != size_default)
			return false;
		break;
	default:
		/* Only narrow read access allowed for now. */
		if (type == BPF_WRITE) {
			if (size != size_default)
				return false;
		} else {
			bpf_ctx_record_field_size(info, size_default);
			if (!bpf_ctx_narrow_access_ok(off, size, size_default))
				return false;
		}
	}
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, data_end):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, mark):
		case bpf_ctx_range(struct __sk_buff, priority):
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static bool __is_valid_xdp_access(int off, int size)
{
	if (off < 0 || off >= sizeof(struct xdp_md))
		return false;
	if (off % size != 0)
		return false;
	if (size != sizeof(__u32))
		return false;

	return true;
}

static bool xdp_is_valid_access(int off, int size,
				enum bpf_access_type type,
				const struct bpf_prog *prog,
				struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE) {
		if (bpf_prog_is_dev_bound(prog->aux)) {
			switch (off) {
			case offsetof(struct xdp_md, rx_queue_index):
				return __is_valid_xdp_access(off, size);
			}
		}
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, tc_index):
		case bpf_ctx_range(struct __sk_buff, priority):
			break;
		default:
			return false;
		}
	case offsetof(struct sk_msg_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		if (size != sizeof(__u64))
			return false;
		break;
	default:
		if (size != sizeof(__u32))
			return false;
	}

	if (off < 0 || off >= sizeof(struct sk_msg_md))
		return false;
	if (off % size != 0)
		return false;

	return true;
}

static u32 bpf_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, len):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, len, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, protocol):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, protocol, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_proto):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_proto, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, priority):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, skb_iif, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 1);
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, hash):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, hash, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, mark):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, pkt_type):
		*target_size = 1;
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->src_reg,
				      PKT_TYPE_OFFSET());
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, PKT_TYPE_MAX);
#ifdef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 5);
#endif
		break;

	case offsetof(struct __sk_buff, queue_mapping):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, queue_mapping, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_present):
	case offsetof(struct __sk_buff, vlan_tci):
		BUILD_BUG_ON(VLAN_TAG_PRESENT != 0x1000);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_tci, 2,
						     target_size));
		if (si->off == offsetof(struct __sk_buff, vlan_tci)) {
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg,
						~VLAN_TAG_PRESENT);
		} else {
			*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 12);
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, 1);
		}
		break;

	case offsetof(struct __sk_buff, cb[0]) ...
	     offsetofend(struct __sk_buff, cb[4]) - 1:
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, data) < 20);
		BUILD_BUG_ON((offsetof(struct sk_buff, cb) +
			      offsetof(struct qdisc_skb_cb, data)) %
			     sizeof(__u64));

		prog->cb_access = 1;
		off  = si->off;
		off -= offsetof(struct __sk_buff, cb[0]);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, data);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_classid):
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, tc_classid) != 2);

		off  = si->off;
		off -= offsetof(struct __sk_buff, tc_classid);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, tc_classid);
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, data));
		break;

	case offsetof(struct __sk_buff, data_meta):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_meta);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_meta);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_index):
#ifdef CONFIG_NET_SCHED
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
#else
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_MOV64_REG(si->dst_reg, si->dst_reg);
		else
			*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, napi_id):
#if defined(CONFIG_NET_RX_BUSY_POLL)
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, napi_id, 4,
						     target_size));
		*insn++ = BPF_JMP_IMM(BPF_JGE, si->dst_reg, MIN_NAPI_ID, 1);
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#else
		*target_size = 4;
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_family,
						     2, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_daddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_rcv_saddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip6[0]) ...
	     offsetof(struct __sk_buff, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, remote_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, local_ip6[0]) ...
	     offsetof(struct __sk_buff, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, local_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_dport,
						     2, target_size));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct __sk_buff, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;
	}

	return insn - insn_buf;
}
	case offsetof(struct __sk_buff, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 sock_filter_convert_ctx_access(enum bpf_access_type type,
					  const struct bpf_insn *si,
					  struct bpf_insn *insn_buf,
					  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock, bound_dev_if):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_bound_dev_if) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_bound_dev_if));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_bound_dev_if));
		break;

	case offsetof(struct bpf_sock, mark):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_mark) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_mark));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_mark));
		break;

	case offsetof(struct bpf_sock, priority):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_priority) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_priority));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_priority));
		break;

	case offsetof(struct bpf_sock, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_family));
		break;

	case offsetof(struct bpf_sock, type):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock, protocol):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock, src_ip4):
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_rcv_saddr,
				       FIELD_SIZEOF(struct sock_common,
						    skc_rcv_saddr),
				       target_size));
		break;

	case bpf_ctx_range_till(struct bpf_sock, src_ip6[0], src_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		off = si->off;
		off -= offsetof(struct bpf_sock, src_ip6[0]);
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(
				struct sock_common,
				skc_v6_rcv_saddr.s6_addr32[0],
				FIELD_SIZEOF(struct sock_common,
					     skc_v6_rcv_saddr.s6_addr32[0]),
				target_size) + off);
#else
		(void)off;
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock, src_port):
		*insn++ = BPF_LDX_MEM(
			BPF_FIELD_SIZEOF(struct sock_common, skc_num),
			si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_num,
				       FIELD_SIZEOF(struct sock_common,
						    skc_num),
				       target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 tc_cls_act_convert_ctx_access(enum bpf_access_type type,
					 const struct bpf_insn *si,
					 struct bpf_insn *insn_buf,
					 struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 xdp_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct xdp_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data));
		break;
	case offsetof(struct xdp_md, data_meta):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_meta),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_meta));
		break;
	case offsetof(struct xdp_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_end));
		break;
	case offsetof(struct xdp_md, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_rxq_info, dev),
				      si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct net_device, ifindex));
		break;
	case offsetof(struct xdp_md, rx_queue_index):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info,
					       queue_index));
		break;
	}

	return insn - insn_buf;
}

/* SOCK_ADDR_LOAD_NESTED_FIELD() loads Nested Field S.F.NF where S is type of
 * context Structure, F is Field in context structure that contains a pointer
 * to Nested Structure of type NS that has the field NF.
 *
 * SIZE encodes the load size (BPF_B, BPF_H, etc). It's up to caller to make
 * sure that SIZE is not greater than actual size of S.F.NF.
 *
 * If offset OFF is provided, the load happens from that offset relative to
 * offset of NF.
 */
#define SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF)	       \
	do {								       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), si->dst_reg,     \
				      si->src_reg, offsetof(S, F));	       \
		*insn++ = BPF_LDX_MEM(					       \
			SIZE, si->dst_reg, si->dst_reg,			       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
	} while (0)

#define SOCK_ADDR_LOAD_NESTED_FIELD(S, NS, F, NF)			       \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF,		       \
					     BPF_FIELD_SIZEOF(NS, NF), 0)

/* SOCK_ADDR_STORE_NESTED_FIELD_OFF() has semantic similar to
 * SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF() but for store operation.
 *
 * It doesn't support SIZE argument though since narrow stores are not
 * supported for now.
 *
 * In addition it uses Temporary Field TF (member of struct S) as the 3rd
 * "register" since two registers available in convert_ctx_access are not
 * enough: we can't override neither SRC, since it contains value to store, nor
 * DST since it contains pointer to context that may be used by later
 * instructions. But we need a temporary place to save pointer to nested
 * structure whose field we want to store to.
 */
#define SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF, TF)		       \
	do {								       \
		int tmp_reg = BPF_REG_9;				       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, tmp_reg,	       \
				      offsetof(S, TF));			       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), tmp_reg,	       \
				      si->dst_reg, offsetof(S, F));	       \
		*insn++ = BPF_STX_MEM(					       \
			BPF_FIELD_SIZEOF(NS, NF), tmp_reg, si->src_reg,	       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
		*insn++ = BPF_LDX_MEM(BPF_DW, tmp_reg, si->dst_reg,	       \
				      offsetof(S, TF));			       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF, \
						      TF)		       \
	do {								       \
		if (type == BPF_WRITE) {				       \
			SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF,    \
							 TF);		       \
		} else {						       \
			SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(		       \
				S, NS, F, NF, SIZE, OFF);  \
		}							       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(S, NS, F, NF, TF)		       \
	SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(			       \
		S, NS, F, NF, BPF_FIELD_SIZEOF(NS, NF), 0, TF)

static u32 sock_addr_convert_ctx_access(enum bpf_access_type type,
					const struct bpf_insn *si,
					struct bpf_insn *insn_buf,
					struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_addr, user_family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sockaddr, uaddr, sa_family);
		break;

	case offsetof(struct bpf_sock_addr, user_ip4):
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in, uaddr,
			sin_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, user_ip6[0], user_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, user_ip6[0]);
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in6, uaddr,
			sin6_addr.s6_addr32[0], BPF_SIZE(si->code), off,
			tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, user_port):
		/* To get port we need to know sa_family first and then treat
		 * sockaddr as either sockaddr_in or sockaddr_in6.
		 * Though we can simplify since port field has same offset and
		 * size in both structures.
		 * Here we check this invariant and use just one of the
		 * structures if it's true.
		 */
		BUILD_BUG_ON(offsetof(struct sockaddr_in, sin_port) !=
			     offsetof(struct sockaddr_in6, sin6_port));
		BUILD_BUG_ON(FIELD_SIZEOF(struct sockaddr_in, sin_port) !=
			     FIELD_SIZEOF(struct sockaddr_in6, sin6_port));
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(struct bpf_sock_addr_kern,
						     struct sockaddr_in6, uaddr,
						     sin6_port, tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sock, sk, sk_family);
		break;

	case offsetof(struct bpf_sock_addr, type):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, protocol):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, msg_src_ip4):
		/* Treat t_ctx as struct in_addr for msg_src_ip4. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in_addr, t_ctx,
			s_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, msg_src_ip6[0],
				msg_src_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, msg_src_ip6[0]);
		/* Treat t_ctx as struct in6_addr for msg_src_ip6. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in6_addr, t_ctx,
			s6_addr32[0], BPF_SIZE(si->code), off, tmp_reg);
		break;
	}

	return insn - insn_buf;
}

static u32 sock_ops_convert_ctx_access(enum bpf_access_type type,
				       const struct bpf_insn *si,
				       struct bpf_insn *insn_buf,
				       struct bpf_prog *prog,
				       u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_ops, op) ...
	     offsetof(struct bpf_sock_ops, replylong[3]):
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, op) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, op));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, reply) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, reply));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, replylong) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, replylong));
		off = si->off;
		off -= offsetof(struct bpf_sock_ops, op);
		off += offsetof(struct bpf_sock_ops_kern, op);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		break;

	case offsetof(struct bpf_sock_ops, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct bpf_sock_ops, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;

	case offsetof(struct bpf_sock_ops, is_fullsock):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern,
						is_fullsock),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern,
					       is_fullsock));
		break;

	case offsetof(struct bpf_sock_ops, state):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_state) != 1);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_state));
		break;

	case offsetof(struct bpf_sock_ops, rtt_min):
		BUILD_BUG_ON(FIELD_SIZEOF(struct tcp_sock, rtt_min) !=
			     sizeof(struct minmax));
		BUILD_BUG_ON(sizeof(struct minmax) <
			     sizeof(struct minmax_sample));

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct tcp_sock, rtt_min) +
				      FIELD_SIZEOF(struct minmax_sample, t));
		break;

/* Helper macro for adding read access to tcp_sock or sock fields. */
#define SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 2);	      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(OBJ,		      \
						       OBJ_FIELD),	      \
				      si->dst_reg, si->dst_reg,		      \
				      offsetof(OBJ, OBJ_FIELD));	      \
	} while (0)

/* Helper macro for adding write access to tcp_sock or sock fields.
 * The macro is called with two registers, dst_reg which contains a pointer
 * to ctx (context) and src_reg which contains the value that should be
 * stored. However, we need an additional register since we cannot overwrite
 * dst_reg because it may be used later in the program.
 * Instead we "borrow" one of the other register. We first save its value
 * into a new (temp) field in bpf_sock_ops_kern, use it, and then restore
 * it at the end of the macro.
 */
#define SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		int reg = BPF_REG_9;					      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, reg, 0, 2);		      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_STX_MEM(BPF_FIELD_SIZEOF(OBJ, OBJ_FIELD),	      \
				      reg, si->src_reg,			      \
				      offsetof(OBJ, OBJ_FIELD));	      \
		*insn++ = BPF_LDX_MEM(BPF_DW, reg, si->dst_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
	} while (0)

#define SOCK_OPS_GET_OR_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ, TYPE)	      \
	do {								      \
		if (TYPE == BPF_WRITE)					      \
			SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
		else							      \
			SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
	} while (0)

	case offsetof(struct bpf_sock_ops, snd_cwnd):
		SOCK_OPS_GET_FIELD(snd_cwnd, snd_cwnd, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, srtt_us):
		SOCK_OPS_GET_FIELD(srtt_us, srtt_us, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bpf_sock_ops_cb_flags):
		SOCK_OPS_GET_FIELD(bpf_sock_ops_cb_flags, bpf_sock_ops_cb_flags,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_ssthresh):
		SOCK_OPS_GET_FIELD(snd_ssthresh, snd_ssthresh, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rcv_nxt):
		SOCK_OPS_GET_FIELD(rcv_nxt, rcv_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_nxt):
		SOCK_OPS_GET_FIELD(snd_nxt, snd_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_una):
		SOCK_OPS_GET_FIELD(snd_una, snd_una, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, mss_cache):
		SOCK_OPS_GET_FIELD(mss_cache, mss_cache, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, ecn_flags):
		SOCK_OPS_GET_FIELD(ecn_flags, ecn_flags, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_delivered):
		SOCK_OPS_GET_FIELD(rate_delivered, rate_delivered,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_interval_us):
		SOCK_OPS_GET_FIELD(rate_interval_us, rate_interval_us,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, packets_out):
		SOCK_OPS_GET_FIELD(packets_out, packets_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, retrans_out):
		SOCK_OPS_GET_FIELD(retrans_out, retrans_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, total_retrans):
		SOCK_OPS_GET_FIELD(total_retrans, total_retrans,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_in):
		SOCK_OPS_GET_FIELD(segs_in, segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_in):
		SOCK_OPS_GET_FIELD(data_segs_in, data_segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_out):
		SOCK_OPS_GET_FIELD(segs_out, segs_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_out):
		SOCK_OPS_GET_FIELD(data_segs_out, data_segs_out,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, lost_out):
		SOCK_OPS_GET_FIELD(lost_out, lost_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sacked_out):
		SOCK_OPS_GET_FIELD(sacked_out, sacked_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sk_txhash):
		SOCK_OPS_GET_OR_SET_FIELD(sk_txhash, sk_txhash,
					  struct sock, type);
		break;

	case offsetof(struct bpf_sock_ops, bytes_received):
		SOCK_OPS_GET_FIELD(bytes_received, bytes_received,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bytes_acked):
		SOCK_OPS_GET_FIELD(bytes_acked, bytes_acked, struct tcp_sock);
		break;

	}
	return insn - insn_buf;
}

static u32 sk_skb_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct tcp_skb_cb, bpf.data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 sk_msg_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
#if IS_ENABLED(CONFIG_IPV6)
	int off;
#endif

	switch (si->off) {
	case offsetof(struct sk_msg_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data));
		break;
	case offsetof(struct sk_msg_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data_end));
		break;
	case offsetof(struct sk_msg_md, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct sk_msg_md, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct sk_msg_md, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct sk_msg_md, remote_ip6[0]) ...
	     offsetof(struct sk_msg_md, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, local_ip6[0]) ...
	     offsetof(struct sk_msg_md, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct sk_msg_md, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_filter_verifier_ops = {
	.get_func_proto		= sk_filter_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops sk_filter_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops tc_cls_act_verifier_ops = {
	.get_func_proto		= tc_cls_act_func_proto,
	.is_valid_access	= tc_cls_act_is_valid_access,
	.convert_ctx_access	= tc_cls_act_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops tc_cls_act_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops xdp_verifier_ops = {
	.get_func_proto		= xdp_func_proto,
	.is_valid_access	= xdp_is_valid_access,
	.convert_ctx_access	= xdp_convert_ctx_access,
};

const struct bpf_prog_ops xdp_prog_ops = {
	.test_run		= bpf_prog_test_run_xdp,
};

const struct bpf_verifier_ops cg_skb_verifier_ops = {
	.get_func_proto		= cg_skb_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops cg_skb_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_in_verifier_ops = {
	.get_func_proto		= lwt_in_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_in_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_out_verifier_ops = {
	.get_func_proto		= lwt_out_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_out_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_xmit_verifier_ops = {
	.get_func_proto		= lwt_xmit_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
};

const struct bpf_prog_ops lwt_xmit_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_seg6local_verifier_ops = {
	.get_func_proto		= lwt_seg6local_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_seg6local_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops cg_sock_verifier_ops = {
	.get_func_proto		= sock_filter_func_proto,
	.is_valid_access	= sock_filter_is_valid_access,
	.convert_ctx_access	= sock_filter_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_prog_ops = {
};

const struct bpf_verifier_ops cg_sock_addr_verifier_ops = {
	.get_func_proto		= sock_addr_func_proto,
	.is_valid_access	= sock_addr_is_valid_access,
	.convert_ctx_access	= sock_addr_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_addr_prog_ops = {
};

const struct bpf_verifier_ops sock_ops_verifier_ops = {
	.get_func_proto		= sock_ops_func_proto,
	.is_valid_access	= sock_ops_is_valid_access,
	.convert_ctx_access	= sock_ops_convert_ctx_access,
};

const struct bpf_prog_ops sock_ops_prog_ops = {
};

const struct bpf_verifier_ops sk_skb_verifier_ops = {
	.get_func_proto		= sk_skb_func_proto,
	.is_valid_access	= sk_skb_is_valid_access,
	.convert_ctx_access	= sk_skb_convert_ctx_access,
	.gen_prologue		= sk_skb_prologue,
};

const struct bpf_prog_ops sk_skb_prog_ops = {
};

const struct bpf_verifier_ops sk_msg_verifier_ops = {
	.get_func_proto		= sk_msg_func_proto,
	.is_valid_access	= sk_msg_is_valid_access,
	.convert_ctx_access	= sk_msg_convert_ctx_access,
};

const struct bpf_prog_ops sk_msg_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;

	if (sock_flag(sk, SOCK_FILTER_LOCKED))
		return -EPERM;

	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (filter) {
		RCU_INIT_POINTER(sk->sk_filter, NULL);
		sk_filter_uncharge(sk, filter);
		ret = 0;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sk_detach_filter);

int sk_get_filter(struct sock *sk, struct sock_filter __user *ubuf,
		  unsigned int len)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	int ret = 0;

	lock_sock(sk);
	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (!filter)
		goto out;

	/* We're copying the filter that has been originally attached,
	 * so no conversion/decode needed anymore. eBPF programs that
	 * have no original program cannot be dumped through this.
	 */
	ret = -EACCES;
	fprog = filter->prog->orig_prog;
	if (!fprog)
		goto out;

	ret = fprog->len;
	if (!len)
		/* User space only enquires number of filter blocks. */
		goto out;

	ret = -EINVAL;
	if (len < fprog->len)
		goto out;

	ret = -EFAULT;
	if (copy_to_user(ubuf, fprog->filter, bpf_classic_proglen(fprog)))
		goto out;

	/* Instead of bytes, the API requests to return the number
	 * of filter blocks.
	 */
	ret = fprog->len;
out:
	release_sock(sk);
	return ret;
}

#ifdef CONFIG_INET
struct sk_reuseport_kern {
	struct sk_buff *skb;
	struct sock *sk;
	struct sock *selected_sk;
	void *data_end;
	u32 hash;
	u32 reuseport_id;
	bool bind_inany;
};

static void bpf_init_reuseport_kern(struct sk_reuseport_kern *reuse_kern,
				    struct sock_reuseport *reuse,
				    struct sock *sk, struct sk_buff *skb,
				    u32 hash)
{
	reuse_kern->skb = skb;
	reuse_kern->sk = sk;
	reuse_kern->selected_sk = NULL;
	reuse_kern->data_end = skb->data + skb_headlen(skb);
	reuse_kern->hash = hash;
	reuse_kern->reuseport_id = reuse->reuseport_id;
	reuse_kern->bind_inany = reuse->bind_inany;
}

struct sock *bpf_run_sk_reuseport(struct sock_reuseport *reuse, struct sock *sk,
				  struct bpf_prog *prog, struct sk_buff *skb,
				  u32 hash)
{
	struct sk_reuseport_kern reuse_kern;
	enum sk_action action;

	bpf_init_reuseport_kern(&reuse_kern, reuse, sk, skb, hash);
	action = BPF_PROG_RUN(prog, &reuse_kern);

	if (action == SK_PASS)
		return reuse_kern.selected_sk;
	else
		return ERR_PTR(-ECONNREFUSED);
}

BPF_CALL_4(sk_select_reuseport, struct sk_reuseport_kern *, reuse_kern,
	   struct bpf_map *, map, void *, key, u32, flags)
{
	struct sock_reuseport *reuse;
	struct sock *selected_sk;

	selected_sk = map->ops->map_lookup_elem(map, key);
	if (!selected_sk)
		return -ENOENT;

	reuse = rcu_dereference(selected_sk->sk_reuseport_cb);
	if (!reuse)
		/* selected_sk is unhashed (e.g. by close()) after the
		 * above map_lookup_elem().  Treat selected_sk has already
		 * been removed from the map.
		 */
		return -ENOENT;

	if (unlikely(reuse->reuseport_id != reuse_kern->reuseport_id)) {
		struct sock *sk;

		if (unlikely(!reuse_kern->reuseport_id))
			/* There is a small race between adding the
			 * sk to the map and setting the
			 * reuse_kern->reuseport_id.
			 * Treat it as the sk has not been added to
			 * the bpf map yet.
			 */
			return -ENOENT;

		sk = reuse_kern->sk;
		if (sk->sk_protocol != selected_sk->sk_protocol)
			return -EPROTOTYPE;
		else if (sk->sk_family != selected_sk->sk_family)
			return -EAFNOSUPPORT;

		/* Catch all. Likely bound to a different sockaddr. */
		return -EBADFD;
	}

	reuse_kern->selected_sk = selected_sk;

	return 0;
}

static const struct bpf_func_proto sk_select_reuseport_proto = {
	.func           = sk_select_reuseport,
	.gpl_only       = false,
	.ret_type       = RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type      = ARG_CONST_MAP_PTR,
	.arg3_type      = ARG_PTR_TO_MAP_KEY,
	.arg4_type	= ARG_ANYTHING,
};

BPF_CALL_4(sk_reuseport_load_bytes,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len)
{
	return ____bpf_skb_load_bytes(reuse_kern->skb, offset, to, len);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_proto = {
	.func		= sk_reuseport_load_bytes,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
};

BPF_CALL_5(sk_reuseport_load_bytes_relative,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len, u32, start_header)
{
	return ____bpf_skb_load_bytes_relative(reuse_kern->skb, offset, to,
					       len, start_header);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_relative_proto = {
	.func		= sk_reuseport_load_bytes_relative,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
	.arg5_type	= ARG_ANYTHING,
};

static const struct bpf_func_proto *
sk_reuseport_func_proto(enum bpf_func_id func_id,
			const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_sk_select_reuseport:
		return &sk_select_reuseport_proto;
	case BPF_FUNC_skb_load_bytes:
		return &sk_reuseport_load_bytes_proto;
	case BPF_FUNC_skb_load_bytes_relative:
		return &sk_reuseport_load_bytes_relative_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static bool
sk_reuseport_is_valid_access(int off, int size,
			     enum bpf_access_type type,
			     const struct bpf_prog *prog,
			     struct bpf_insn_access_aux *info)
{
	const u32 size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct sk_reuseport_md) ||
	    off % size || type != BPF_READ)
		return false;

	switch (off) {
	case offsetof(struct sk_reuseport_md, data):
		info->reg_type = PTR_TO_PACKET;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, hash):
		return size == size_default;

	/* Fields that allow narrowing */
	case offsetof(struct sk_reuseport_md, eth_protocol):
		if (size < FIELD_SIZEOF(struct sk_buff, protocol))
			return false;
		/* fall through */
	case offsetof(struct sk_reuseport_md, ip_protocol):
	case offsetof(struct sk_reuseport_md, bind_inany):
	case offsetof(struct sk_reuseport_md, len):
		bpf_ctx_record_field_size(info, size_default);
		return bpf_ctx_narrow_access_ok(off, size, size_default);

	default:
		return false;
	}
}

#define SK_REUSEPORT_LOAD_FIELD(F) ({					\
	*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_reuseport_kern, F), \
			      si->dst_reg, si->src_reg,			\
			      bpf_target_off(struct sk_reuseport_kern, F, \
					     FIELD_SIZEOF(struct sk_reuseport_kern, F), \
					     target_size));		\
	})

#define SK_REUSEPORT_LOAD_SKB_FIELD(SKB_FIELD)				\
	SOCK_ADDR_LOAD_NESTED_FIELD(struct sk_reuseport_kern,		\
				    struct sk_buff,			\
				    skb,				\
				    SKB_FIELD)

#define SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(SK_FIELD, BPF_SIZE, EXTRA_OFF) \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(struct sk_reuseport_kern,	\
					     struct sock,		\
					     sk,			\
					     SK_FIELD, BPF_SIZE, EXTRA_OFF)

static u32 sk_reuseport_convert_ctx_access(enum bpf_access_type type,
					   const struct bpf_insn *si,
					   struct bpf_insn *insn_buf,
					   struct bpf_prog *prog,
					   u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct sk_reuseport_md, data):
		SK_REUSEPORT_LOAD_SKB_FIELD(data);
		break;

	case offsetof(struct sk_reuseport_md, len):
		SK_REUSEPORT_LOAD_SKB_FIELD(len);
		break;

	case offsetof(struct sk_reuseport_md, eth_protocol):
		SK_REUSEPORT_LOAD_SKB_FIELD(protocol);
		break;

	case offsetof(struct sk_reuseport_md, ip_protocol):
		BUILD_BUG_ON(HWEIGHT32(SK_FL_PROTO_MASK) != BITS_PER_BYTE);
		SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(__sk_flags_offset,
						    BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		/* SK_FL_PROTO_MASK and SK_FL_PROTO_SHIFT are endian
		 * aware.  No further narrowing or masking is needed.
		 */
		*target_size = 1;
		break;

	case offsetof(struct sk_reuseport_md, data_end):
		SK_REUSEPORT_LOAD_FIELD(data_end);
		break;

	case offsetof(struct sk_reuseport_md, hash):
		SK_REUSEPORT_LOAD_FIELD(hash);
		break;

	case offsetof(struct sk_reuseport_md, bind_inany):
		SK_REUSEPORT_LOAD_FIELD(bind_inany);
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_reuseport_verifier_ops = {
	.get_func_proto		= sk_reuseport_func_proto,
	.is_valid_access	= sk_reuseport_is_valid_access,
	.convert_ctx_access	= sk_reuseport_convert_ctx_access,
};

const struct bpf_prog_ops sk_reuseport_prog_ops = {
};
#endif /* CONFIG_INET */
const struct bpf_prog_ops sk_msg_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;

	if (sock_flag(sk, SOCK_FILTER_LOCKED))
		return -EPERM;

	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (filter) {
		RCU_INIT_POINTER(sk->sk_filter, NULL);
		sk_filter_uncharge(sk, filter);
		ret = 0;
	}

	return ret;
}