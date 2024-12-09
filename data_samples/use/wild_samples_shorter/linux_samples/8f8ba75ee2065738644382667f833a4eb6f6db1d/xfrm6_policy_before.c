	return 0;
}

static int xfrm6_init_path(struct xfrm_dst *path, struct dst_entry *dst,
			   int nfheader_len)
{
	if (dst->ops->family == AF_INET6) {
	.get_saddr = 		xfrm6_get_saddr,
	.decode_session =	_decode_session6,
	.get_tos =		xfrm6_get_tos,
	.init_path =		xfrm6_init_path,
	.fill_dst =		xfrm6_fill_dst,
	.blackhole_route =	ip6_blackhole_route,
};