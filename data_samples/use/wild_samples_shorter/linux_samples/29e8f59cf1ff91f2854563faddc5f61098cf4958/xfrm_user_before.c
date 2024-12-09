	link = &xfrm_dispatch[type];

	/* All operations require privileges, even GET */
	if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
		return -EPERM;

	if ((type == (XFRM_MSG_GETSA - XFRM_MSG_BASE) ||
	     type == (XFRM_MSG_GETPOLICY - XFRM_MSG_BASE)) &&