		if (nlk_sk(osk)->portid == portid) {
			/* Bind collision, search negative portid values. */
			portid = rover--;
			if (rover > -4097)
				rover = -4097;
			netlink_table_ungrab();
			goto retry;
		}
	}
	netlink_table_ungrab();

	err = netlink_insert(sk, net, portid);
	if (err == -EADDRINUSE)
		goto retry;

	/* If 2 threads race to autobind, that is fine.  */
	if (err == -EBUSY)
		err = 0;

	return err;
}

/**
 * __netlink_ns_capable - General netlink message capability test
 * @nsp: NETLINK_CB of the socket buffer holding a netlink command from userspace.
 * @user_ns: The user namespace of the capability to use
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket we received the message
 * from had when the netlink socket was created and the sender of the
 * message has has the capability @cap in the user namespace @user_ns.
 */
bool __netlink_ns_capable(const struct netlink_skb_parms *nsp,
			struct user_namespace *user_ns, int cap)
{
	return sk_ns_capable(nsp->sk, user_ns, cap);
}
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	DECLARE_SOCKADDR(struct sockaddr_nl *, addr, msg->msg_name);
	u32 dst_portid;
	u32 dst_group;
	struct sk_buff *skb;
	int err;
	struct scm_cookie scm;

	if (msg->msg_flags&MSG_OOB)
		return -EOPNOTSUPP;

	if (NULL == siocb->scm)
		siocb->scm = &scm;

	err = scm_send(sock, msg, siocb->scm, true);
	if (err < 0)
		return err;

	if (msg->msg_namelen) {
		err = -EINVAL;
		if (addr->nl_family != AF_NETLINK)
			goto out;
		dst_portid = addr->nl_pid;
		dst_group = ffs(addr->nl_groups);
		err =  -EPERM;
		if ((dst_group || dst_portid) &&
		    !netlink_allowed(sock, NL_CFG_F_NONROOT_SEND))
			goto out;
	} else {
		dst_portid = nlk->dst_portid;
		dst_group = nlk->dst_group;
	}

	if (!nlk->portid) {
		err = netlink_autobind(sock);
		if (err)
			goto out;
	}

	if (netlink_tx_is_mmaped(sk) &&
	    msg->msg_iov->iov_base == NULL) {
		err = netlink_mmap_sendmsg(sk, msg, dst_portid, dst_group,
					   siocb);
		goto out;
	}

	err = -EMSGSIZE;
	if (len > sk->sk_sndbuf - 32)
		goto out;
	err = -ENOBUFS;
	skb = netlink_alloc_large_skb(len, dst_group);
	if (skb == NULL)
		goto out;

	NETLINK_CB(skb).portid	= nlk->portid;
	NETLINK_CB(skb).dst_group = dst_group;
	NETLINK_CB(skb).creds	= siocb->scm->creds;

	err = -EFAULT;
	if (memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len)) {
		kfree_skb(skb);
		goto out;
	}

	err = security_netlink_send(sk, skb);
	if (err) {
		kfree_skb(skb);
		goto out;
	}

	if (dst_group) {
		atomic_inc(&skb->users);
		netlink_broadcast(sk, skb, dst_portid, dst_group, GFP_KERNEL);
	}
	err = netlink_unicast(sk, skb, dst_portid, msg->msg_flags&MSG_DONTWAIT);

out:
	scm_destroy(siocb->scm);
	return err;
}
	if (msg->msg_namelen) {
		err = -EINVAL;
		if (addr->nl_family != AF_NETLINK)
			goto out;
		dst_portid = addr->nl_pid;
		dst_group = ffs(addr->nl_groups);
		err =  -EPERM;
		if ((dst_group || dst_portid) &&
		    !netlink_allowed(sock, NL_CFG_F_NONROOT_SEND))
			goto out;
	} else {
		dst_portid = nlk->dst_portid;
		dst_group = nlk->dst_group;
	}
	    msg->msg_iov->iov_base == NULL) {
		err = netlink_mmap_sendmsg(sk, msg, dst_portid, dst_group,
					   siocb);
		goto out;
	}

	err = -EMSGSIZE;
	if (len > sk->sk_sndbuf - 32)
		goto out;
	err = -ENOBUFS;
	skb = netlink_alloc_large_skb(len, dst_group);
	if (skb == NULL)
		goto out;

	NETLINK_CB(skb).portid	= nlk->portid;
	NETLINK_CB(skb).dst_group = dst_group;
	NETLINK_CB(skb).creds	= siocb->scm->creds;

	err = -EFAULT;
	if (memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len)) {
		kfree_skb(skb);
		goto out;
	}