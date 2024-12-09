			void *user_icmph, size_t icmph_len) {
	u8 type, code;

	if (len > 0xFFFF)
		return -EMSGSIZE;

	/*
	 *	Check the flags.
	 */

	/* Mirror BSD error message compatibility */
	if (msg->msg_flags & MSG_OOB)
		return -EOPNOTSUPP;

	/*
	 *	Fetch the ICMP header provided by the userland.
	 *	iovec is modified! The ICMP header is consumed.
	 */
	if (memcpy_from_msg(user_icmph, msg, icmph_len))
		return -EFAULT;

	if (family == AF_INET) {
		type = ((struct icmphdr *) user_icmph)->type;
		code = ((struct icmphdr *) user_icmph)->code;
#if IS_ENABLED(CONFIG_IPV6)
	} else if (family == AF_INET6) {
		type = ((struct icmp6hdr *) user_icmph)->icmp6_type;
		code = ((struct icmp6hdr *) user_icmph)->icmp6_code;
#endif
	} else {
		BUG();
	}

	if (!ping_supported(family, type, code))
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL_GPL(ping_common_sendmsg);

static int ping_v4_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
	struct net *net = sock_net(sk);
	struct flowi4 fl4;
	struct inet_sock *inet = inet_sk(sk);
	struct ipcm_cookie ipc;
	struct icmphdr user_icmph;
	struct pingfakehdr pfh;
	struct rtable *rt = NULL;
	struct ip_options_data opt_copy;
	int free = 0;
	__be32 saddr, daddr, faddr;
	u8  tos;
	int err;

	pr_debug("ping_v4_sendmsg(sk=%p,sk->num=%u)\n", inet, inet->inet_num);

	err = ping_common_sendmsg(AF_INET, msg, len, &user_icmph,
				  sizeof(user_icmph));
	if (err)
		return err;

	/*
	 *	Get and verify the address.
	 */

	if (msg->msg_name) {
		DECLARE_SOCKADDR(struct sockaddr_in *, usin, msg->msg_name);
		if (msg->msg_namelen < sizeof(*usin))
			return -EINVAL;
		if (usin->sin_family != AF_INET)
			return -EAFNOSUPPORT;
		daddr = usin->sin_addr.s_addr;
		/* no remote port */
	} else {
		if (sk->sk_state != TCP_ESTABLISHED)
			return -EDESTADDRREQ;
		daddr = inet->inet_daddr;
		/* no remote port */
	}

	ipc.sockc.tsflags = sk->sk_tsflags;
	ipc.addr = inet->inet_saddr;
	ipc.opt = NULL;
	ipc.oif = sk->sk_bound_dev_if;
	ipc.tx_flags = 0;
	ipc.ttl = 0;
	ipc.tos = -1;

	if (msg->msg_controllen) {
		err = ip_cmsg_send(sk, msg, &ipc, false);
		if (unlikely(err)) {
			kfree(ipc.opt);
			return err;
		}
		if (ipc.opt)
			free = 1;
	}
	if (!ipc.opt) {
		struct ip_options_rcu *inet_opt;

		rcu_read_lock();
		inet_opt = rcu_dereference(inet->inet_opt);
		if (inet_opt) {
			memcpy(&opt_copy, inet_opt,
			       sizeof(*inet_opt) + inet_opt->opt.optlen);
			ipc.opt = &opt_copy.opt;
		}
		rcu_read_unlock();
	}

	sock_tx_timestamp(sk, ipc.sockc.tsflags, &ipc.tx_flags);

	saddr = ipc.addr;
	ipc.addr = faddr = daddr;

	if (ipc.opt && ipc.opt->opt.srr) {
		if (!daddr)
			return -EINVAL;
		faddr = ipc.opt->opt.faddr;
	}
	tos = get_rttos(&ipc, inet);
	if (sock_flag(sk, SOCK_LOCALROUTE) ||
	    (msg->msg_flags & MSG_DONTROUTE) ||
	    (ipc.opt && ipc.opt->opt.is_strictroute)) {
		tos |= RTO_ONLINK;
	}

	if (ipv4_is_multicast(daddr)) {
		if (!ipc.oif)
			ipc.oif = inet->mc_index;
		if (!saddr)
			saddr = inet->mc_addr;
	} else if (!ipc.oif)
		ipc.oif = inet->uc_index;

	flowi4_init_output(&fl4, ipc.oif, sk->sk_mark, tos,
			   RT_SCOPE_UNIVERSE, sk->sk_protocol,
			   inet_sk_flowi_flags(sk), faddr, saddr, 0, 0);

	security_sk_classify_flow(sk, flowi4_to_flowi(&fl4));
	rt = ip_route_output_flow(net, &fl4, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		rt = NULL;
		if (err == -ENETUNREACH)
			IP_INC_STATS(net, IPSTATS_MIB_OUTNOROUTES);
		goto out;
	}

	err = -EACCES;
	if ((rt->rt_flags & RTCF_BROADCAST) &&
	    !sock_flag(sk, SOCK_BROADCAST))
		goto out;

	if (msg->msg_flags & MSG_CONFIRM)
		goto do_confirm;
back_from_confirm:

	if (!ipc.addr)
		ipc.addr = fl4.daddr;

	lock_sock(sk);

	pfh.icmph.type = user_icmph.type; /* already checked */
	pfh.icmph.code = user_icmph.code; /* ditto */
	pfh.icmph.checksum = 0;
	pfh.icmph.un.echo.id = inet->inet_sport;
	pfh.icmph.un.echo.sequence = user_icmph.un.echo.sequence;
	pfh.msg = msg;
	pfh.wcheck = 0;
	pfh.family = AF_INET;

	err = ip_append_data(sk, &fl4, ping_getfrag, &pfh, len,
			0, &ipc, &rt, msg->msg_flags);
	if (err)
		ip_flush_pending_frames(sk);
	else
		err = ping_v4_push_pending_frames(sk, &pfh, &fl4);
	release_sock(sk);

out:
	ip_rt_put(rt);
	if (free)
		kfree(ipc.opt);
	if (!err) {
		icmp_out_count(sock_net(sk), user_icmph.type);
		return len;
	}
	return err;

do_confirm:
	dst_confirm(&rt->dst);
	if (!(msg->msg_flags & MSG_PROBE) || len)
		goto back_from_confirm;
	err = 0;
	goto out;
}

int ping_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int noblock,
		 int flags, int *addr_len)
{
	struct inet_sock *isk = inet_sk(sk);
	int family = sk->sk_family;
	struct sk_buff *skb;
	int copied, err;

	pr_debug("ping_recvmsg(sk=%p,sk->num=%u)\n", isk, isk->inet_num);

	err = -EOPNOTSUPP;
	if (flags & MSG_OOB)
		goto out;

	if (flags & MSG_ERRQUEUE)
		return inet_recv_error(sk, msg, len, addr_len);

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb)
		goto out;

	copied = skb->len;
	if (copied > len) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	/* Don't bother checking the checksum */
	err = skb_copy_datagram_msg(skb, 0, msg, copied);
	if (err)
		goto done;

	sock_recv_timestamp(msg, sk, skb);

	/* Copy the address and add cmsg data. */
	if (family == AF_INET) {
		DECLARE_SOCKADDR(struct sockaddr_in *, sin, msg->msg_name);

		if (sin) {
			sin->sin_family = AF_INET;
			sin->sin_port = 0 /* skb->h.uh->source */;
			sin->sin_addr.s_addr = ip_hdr(skb)->saddr;
			memset(sin->sin_zero, 0, sizeof(sin->sin_zero));
			*addr_len = sizeof(*sin);
		}

		if (isk->cmsg_flags)
			ip_cmsg_recv(msg, skb);

#if IS_ENABLED(CONFIG_IPV6)
	} else if (family == AF_INET6) {
		struct ipv6_pinfo *np = inet6_sk(sk);
		struct ipv6hdr *ip6 = ipv6_hdr(skb);
		DECLARE_SOCKADDR(struct sockaddr_in6 *, sin6, msg->msg_name);

		if (sin6) {
			sin6->sin6_family = AF_INET6;
			sin6->sin6_port = 0;
			sin6->sin6_addr = ip6->saddr;
			sin6->sin6_flowinfo = 0;
			if (np->sndflow)
				sin6->sin6_flowinfo = ip6_flowinfo(ip6);
			sin6->sin6_scope_id =
				ipv6_iface_scope_id(&sin6->sin6_addr,
						    inet6_iif(skb));
			*addr_len = sizeof(*sin6);
		}

		if (inet6_sk(sk)->rxopt.all)
			pingv6_ops.ip6_datagram_recv_common_ctl(sk, msg, skb);
		if (skb->protocol == htons(ETH_P_IPV6) &&
		    inet6_sk(sk)->rxopt.all)
			pingv6_ops.ip6_datagram_recv_specific_ctl(sk, msg, skb);
		else if (skb->protocol == htons(ETH_P_IP) && isk->cmsg_flags)
			ip_cmsg_recv(msg, skb);
#endif
	} else {
		BUG();
	}

	err = copied;

done:
	skb_free_datagram(sk, skb);
out:
	pr_debug("ping_recvmsg -> %d\n", err);
	return err;
}
EXPORT_SYMBOL_GPL(ping_recvmsg);

int ping_queue_rcv_skb(struct sock *sk, struct sk_buff *skb)
{
	pr_debug("ping_queue_rcv_skb(sk=%p,sk->num=%d,skb=%p)\n",
		 inet_sk(sk), inet_sk(sk)->inet_num, skb);
	if (sock_queue_rcv_skb(sk, skb) < 0) {
		kfree_skb(skb);
		pr_debug("ping_queue_rcv_skb -> failed\n");
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(ping_queue_rcv_skb);


/*
 *	All we need to do is get the socket.
 */

bool ping_rcv(struct sk_buff *skb)
{
	struct sock *sk;
	struct net *net = dev_net(skb->dev);
	struct icmphdr *icmph = icmp_hdr(skb);

	/* We assume the packet has already been checked by icmp_rcv */

	pr_debug("ping_rcv(skb=%p,id=%04x,seq=%04x)\n",
		 skb, ntohs(icmph->un.echo.id), ntohs(icmph->un.echo.sequence));

	/* Push ICMP header back */
	skb_push(skb, skb->data - (u8 *)icmph);

	sk = ping_lookup(net, skb, ntohs(icmph->un.echo.id));
	if (sk) {
		struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);

		pr_debug("rcv on socket %p\n", sk);
		if (skb2)
			ping_queue_rcv_skb(sk, skb2);
		sock_put(sk);
		return true;
	}
	pr_debug("no socket, dropping\n");

	return false;
}
EXPORT_SYMBOL_GPL(ping_rcv);

struct proto ping_prot = {
	.name =		"PING",
	.owner =	THIS_MODULE,
	.init =		ping_init_sock,
	.close =	ping_close,
	.connect =	ip4_datagram_connect,
	.disconnect =	__udp_disconnect,
	.setsockopt =	ip_setsockopt,
	.getsockopt =	ip_getsockopt,
	.sendmsg =	ping_v4_sendmsg,
	.recvmsg =	ping_recvmsg,
	.bind =		ping_bind,
	.backlog_rcv =	ping_queue_rcv_skb,
	.release_cb =	ip4_datagram_release_cb,
	.hash =		ping_hash,
	.unhash =	ping_unhash,
	.get_port =	ping_get_port,
	.obj_size =	sizeof(struct inet_sock),
};
EXPORT_SYMBOL(ping_prot);

#ifdef CONFIG_PROC_FS

static struct sock *ping_get_first(struct seq_file *seq, int start)
{
	struct sock *sk;
	struct ping_iter_state *state = seq->private;
	struct net *net = seq_file_net(seq);

	for (state->bucket = start; state->bucket < PING_HTABLE_SIZE;
	     ++state->bucket) {
		struct hlist_nulls_node *node;
		struct hlist_nulls_head *hslot;

		hslot = &ping_table.hash[state->bucket];

		if (hlist_nulls_empty(hslot))
			continue;

		sk_nulls_for_each(sk, node, hslot) {
			if (net_eq(sock_net(sk), net) &&
			    sk->sk_family == state->family)
				goto found;
		}
	}
	sk = NULL;
found:
	return sk;
}

static struct sock *ping_get_next(struct seq_file *seq, struct sock *sk)
{
	struct ping_iter_state *state = seq->private;
	struct net *net = seq_file_net(seq);

	do {
		sk = sk_nulls_next(sk);
	} while (sk && (!net_eq(sock_net(sk), net)));

	if (!sk)
		return ping_get_first(seq, state->bucket + 1);
	return sk;
}

static struct sock *ping_get_idx(struct seq_file *seq, loff_t pos)
{
	struct sock *sk = ping_get_first(seq, 0);

	if (sk)
		while (pos && (sk = ping_get_next(seq, sk)) != NULL)
			--pos;
	return pos ? NULL : sk;
}

void *ping_seq_start(struct seq_file *seq, loff_t *pos, sa_family_t family)
	__acquires(ping_table.lock)
{
	struct ping_iter_state *state = seq->private;
	state->bucket = 0;
	state->family = family;

	read_lock_bh(&ping_table.lock);

	return *pos ? ping_get_idx(seq, *pos-1) : SEQ_START_TOKEN;
}
EXPORT_SYMBOL_GPL(ping_seq_start);

static void *ping_v4_seq_start(struct seq_file *seq, loff_t *pos)
{
	return ping_seq_start(seq, pos, AF_INET);
}

void *ping_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct sock *sk;

	if (v == SEQ_START_TOKEN)
		sk = ping_get_idx(seq, 0);
	else
		sk = ping_get_next(seq, v);

	++*pos;
	return sk;
}
EXPORT_SYMBOL_GPL(ping_seq_next);

void ping_seq_stop(struct seq_file *seq, void *v)
	__releases(ping_table.lock)
{
	read_unlock_bh(&ping_table.lock);
}
EXPORT_SYMBOL_GPL(ping_seq_stop);

static void ping_v4_format_sock(struct sock *sp, struct seq_file *f,
		int bucket)
{
	struct inet_sock *inet = inet_sk(sp);
	__be32 dest = inet->inet_daddr;
	__be32 src = inet->inet_rcv_saddr;
	__u16 destp = ntohs(inet->inet_dport);
	__u16 srcp = ntohs(inet->inet_sport);

	seq_printf(f, "%5d: %08X:%04X %08X:%04X"
		" %02X %08X:%08X %02X:%08lX %08X %5u %8d %lu %d %pK %d",
		bucket, src, srcp, dest, destp, sp->sk_state,
		sk_wmem_alloc_get(sp),
		sk_rmem_alloc_get(sp),
		0, 0L, 0,
		from_kuid_munged(seq_user_ns(f), sock_i_uid(sp)),
		0, sock_i_ino(sp),
		atomic_read(&sp->sk_refcnt), sp,
		atomic_read(&sp->sk_drops));
}

static int ping_v4_seq_show(struct seq_file *seq, void *v)
{
	seq_setwidth(seq, 127);
	if (v == SEQ_START_TOKEN)
		seq_puts(seq, "  sl  local_address rem_address   st tx_queue "
			   "rx_queue tr tm->when retrnsmt   uid  timeout "
			   "inode ref pointer drops");
	else {
		struct ping_iter_state *state = seq->private;

		ping_v4_format_sock(v, seq, state->bucket);
	}
	seq_pad(seq, '\n');
	return 0;
}

static int ping_seq_open(struct inode *inode, struct file *file)
{
	struct ping_seq_afinfo *afinfo = PDE_DATA(inode);
	return seq_open_net(inode, file, &afinfo->seq_ops,
			   sizeof(struct ping_iter_state));
}

const struct file_operations ping_seq_fops = {
	.open		= ping_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_net,
};
EXPORT_SYMBOL_GPL(ping_seq_fops);

static struct ping_seq_afinfo ping_v4_seq_afinfo = {
	.name		= "icmp",
	.family		= AF_INET,
	.seq_fops	= &ping_seq_fops,
	.seq_ops	= {
		.start		= ping_v4_seq_start,
		.show		= ping_v4_seq_show,
		.next		= ping_seq_next,
		.stop		= ping_seq_stop,
	},
};

int ping_proc_register(struct net *net, struct ping_seq_afinfo *afinfo)
{
	struct proc_dir_entry *p;
	p = proc_create_data(afinfo->name, S_IRUGO, net->proc_net,
			     afinfo->seq_fops, afinfo);
	if (!p)
		return -ENOMEM;
	return 0;
}
EXPORT_SYMBOL_GPL(ping_proc_register);

void ping_proc_unregister(struct net *net, struct ping_seq_afinfo *afinfo)
{
	remove_proc_entry(afinfo->name, net->proc_net);
}
EXPORT_SYMBOL_GPL(ping_proc_unregister);

static int __net_init ping_v4_proc_init_net(struct net *net)
{
	return ping_proc_register(net, &ping_v4_seq_afinfo);
}

static void __net_exit ping_v4_proc_exit_net(struct net *net)
{
	ping_proc_unregister(net, &ping_v4_seq_afinfo);
}

static struct pernet_operations ping_v4_net_ops = {
	.init = ping_v4_proc_init_net,
	.exit = ping_v4_proc_exit_net,
};

int __init ping_proc_init(void)
{
	return register_pernet_subsys(&ping_v4_net_ops);
}

void ping_proc_exit(void)
{
	unregister_pernet_subsys(&ping_v4_net_ops);
}

#endif

void __init ping_init(void)
{
	int i;

	for (i = 0; i < PING_HTABLE_SIZE; i++)
		INIT_HLIST_NULLS_HEAD(&ping_table.hash[i], i);
	rwlock_init(&ping_table.lock);
}