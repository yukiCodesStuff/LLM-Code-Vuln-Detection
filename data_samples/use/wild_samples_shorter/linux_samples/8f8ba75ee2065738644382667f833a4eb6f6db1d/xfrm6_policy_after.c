	return 0;
}

static void xfrm6_init_dst(struct net *net, struct xfrm_dst *xdst)
{
	struct rt6_info *rt = (struct rt6_info *)xdst;

	rt6_init_peer(rt, net->ipv6.peers);
}

static int xfrm6_init_path(struct xfrm_dst *path, struct dst_entry *dst,
			   int nfheader_len)
{
	if (dst->ops->family == AF_INET6) {
	.get_saddr = 		xfrm6_get_saddr,
	.decode_session =	_decode_session6,
	.get_tos =		xfrm6_get_tos,
	.init_dst =		xfrm6_init_dst,
	.init_path =		xfrm6_init_path,
	.fill_dst =		xfrm6_fill_dst,
	.blackhole_route =	ip6_blackhole_route,
};