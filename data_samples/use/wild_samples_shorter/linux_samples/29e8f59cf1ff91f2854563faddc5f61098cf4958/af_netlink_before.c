	return err;
}

static inline int netlink_capable(const struct socket *sock, unsigned int flag)
{
	return (nl_table[sock->sk->sk_protocol].flags & flag) ||
		ns_capable(sock_net(sock->sk)->user_ns, CAP_NET_ADMIN);
}

	/* Only superuser is allowed to listen multicasts */
	if (nladdr->nl_groups) {
		if (!netlink_capable(sock, NL_CFG_F_NONROOT_RECV))
			return -EPERM;
		err = netlink_realloc_groups(sk);
		if (err)
			return err;
		return -EINVAL;

	if ((nladdr->nl_groups || nladdr->nl_pid) &&
	    !netlink_capable(sock, NL_CFG_F_NONROOT_SEND))
		return -EPERM;

	if (!nlk->portid)
		err = netlink_autobind(sock);
		break;
	case NETLINK_ADD_MEMBERSHIP:
	case NETLINK_DROP_MEMBERSHIP: {
		if (!netlink_capable(sock, NL_CFG_F_NONROOT_RECV))
			return -EPERM;
		err = netlink_realloc_groups(sk);
		if (err)
			return err;
		dst_group = ffs(addr->nl_groups);
		err =  -EPERM;
		if ((dst_group || dst_portid) &&
		    !netlink_capable(sock, NL_CFG_F_NONROOT_SEND))
			goto out;
	} else {
		dst_portid = nlk->dst_portid;
		dst_group = nlk->dst_group;