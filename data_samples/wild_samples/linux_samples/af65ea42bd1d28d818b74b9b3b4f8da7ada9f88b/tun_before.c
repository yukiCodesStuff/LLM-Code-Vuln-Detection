{
	unsigned int datasize = xdp->data_end - xdp->data;
	struct tun_xdp_hdr *hdr = xdp->data_hard_start;
	struct virtio_net_hdr *gso = &hdr->gso;
	struct bpf_prog *xdp_prog;
	struct sk_buff *skb = NULL;
	struct sk_buff_head *queue;
	u32 rxhash = 0, act;
	int buflen = hdr->buflen;
	int ret = 0;
	bool skb_xdp = false;
	struct page *page;

	xdp_prog = rcu_dereference(tun->xdp_prog);
	if (xdp_prog) {
		if (gso->gso_type) {
			skb_xdp = true;
			goto build;
		}

		xdp_init_buff(xdp, buflen, &tfile->xdp_rxq);
		xdp_set_data_meta_invalid(xdp);

		act = bpf_prog_run_xdp(xdp_prog, xdp);
		ret = tun_xdp_act(tun, xdp_prog, xdp, act);
		if (ret < 0) {
			put_page(virt_to_head_page(xdp->data));
			return ret;
		}

		switch (ret) {
		case XDP_REDIRECT:
			*flush = true;
			fallthrough;
		case XDP_TX:
			return 0;
		case XDP_PASS:
			break;
		default:
			page = virt_to_head_page(xdp->data);
			if (tpage->page == page) {
				++tpage->count;
			} else {
				tun_put_page(tpage);
				tpage->page = page;
				tpage->count = 1;
			}
			return 0;
		}
	}

build:
	skb = build_skb(xdp->data_hard_start, buflen);
	if (!skb) {
		ret = -ENOMEM;
		goto out;
	}

	skb_reserve(skb, xdp->data - xdp->data_hard_start);
	skb_put(skb, xdp->data_end - xdp->data);

	if (virtio_net_hdr_to_skb(skb, gso, tun_is_little_endian(tun))) {
		atomic_long_inc(&tun->rx_frame_errors);
		kfree_skb(skb);
		ret = -EINVAL;
		goto out;
	}

	skb->protocol = eth_type_trans(skb, tun->dev);
	skb_reset_network_header(skb);
	skb_probe_transport_header(skb);
	skb_record_rx_queue(skb, tfile->queue_index);

	if (skb_xdp) {
		ret = do_xdp_generic(xdp_prog, &skb);
		if (ret != XDP_PASS) {
			ret = 0;
			goto out;
		}
	}

	if (!rcu_dereference(tun->steering_prog) && tun->numqueues > 1 &&
	    !tfile->detached)
		rxhash = __skb_get_hash_symmetric(skb);

	if (tfile->napi_enabled) {
		queue = &tfile->sk.sk_write_queue;
		spin_lock(&queue->lock);

		if (unlikely(tfile->detached)) {
			spin_unlock(&queue->lock);
			kfree_skb(skb);
			return -EBUSY;
		}

		__skb_queue_tail(queue, skb);
		spin_unlock(&queue->lock);
		ret = 1;
	} else {
		netif_receive_skb(skb);
		ret = 0;
	}

	/* No need to disable preemption here since this function is
	 * always called with bh disabled
	 */
	dev_sw_netstats_rx_add(tun->dev, datasize);

	if (rxhash)
		tun_flow_update(tun, rxhash, tfile);

out:
	return ret;
}

static int tun_sendmsg(struct socket *sock, struct msghdr *m, size_t total_len)
{
	int ret, i;
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun = tun_get(tfile);
	struct tun_msg_ctl *ctl = m->msg_control;
	struct xdp_buff *xdp;

	if (!tun)
		return -EBADFD;

	if (m->msg_controllen == sizeof(struct tun_msg_ctl) &&
	    ctl && ctl->type == TUN_MSG_PTR) {
		struct bpf_net_context __bpf_net_ctx, *bpf_net_ctx;
		struct tun_page tpage;
		int n = ctl->num;
		int flush = 0, queued = 0;

		memset(&tpage, 0, sizeof(tpage));

		local_bh_disable();
		rcu_read_lock();
		bpf_net_ctx = bpf_net_ctx_set(&__bpf_net_ctx);

		for (i = 0; i < n; i++) {
			xdp = &((struct xdp_buff *)ctl->ptr)[i];
			ret = tun_xdp_one(tun, tfile, xdp, &flush, &tpage);
			if (ret > 0)
				queued += ret;
		}

		if (flush)
			xdp_do_flush();

		if (tfile->napi_enabled && queued > 0)
			napi_schedule(&tfile->napi);

		bpf_net_ctx_clear(bpf_net_ctx);
		rcu_read_unlock();
		local_bh_enable();

		tun_put_page(&tpage);

		ret = total_len;
		goto out;
	}

	ret = tun_get_user(tun, tfile, ctl ? ctl->ptr : NULL, &m->msg_iter,
			   m->msg_flags & MSG_DONTWAIT,
			   m->msg_flags & MSG_MORE);
out:
	tun_put(tun);
	return ret;
}

static int tun_recvmsg(struct socket *sock, struct msghdr *m, size_t total_len,
		       int flags)
{
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun = tun_get(tfile);
	void *ptr = m->msg_control;
	int ret;

	if (!tun) {
		ret = -EBADFD;
		goto out_free;
	}

	if (flags & ~(MSG_DONTWAIT|MSG_TRUNC|MSG_ERRQUEUE)) {
		ret = -EINVAL;
		goto out_put_tun;
	}
	if (flags & MSG_ERRQUEUE) {
		ret = sock_recv_errqueue(sock->sk, m, total_len,
					 SOL_PACKET, TUN_TX_TIMESTAMP);
		goto out;
	}
	ret = tun_do_read(tun, tfile, &m->msg_iter, flags & MSG_DONTWAIT, ptr);
	if (ret > (ssize_t)total_len) {
		m->msg_flags |= MSG_TRUNC;
		ret = flags & MSG_TRUNC ? ret : total_len;
	}
out:
	tun_put(tun);
	return ret;

out_put_tun:
	tun_put(tun);
out_free:
	tun_ptr_free(ptr);
	return ret;
}

static int tun_ptr_peek_len(void *ptr)
{
	if (likely(ptr)) {
		if (tun_is_xdp_frame(ptr)) {
			struct xdp_frame *xdpf = tun_ptr_to_xdp(ptr);

			return xdpf->len;
		}
		return __skb_array_len_with_tag(ptr);
	} else {
		return 0;
	}
}

static int tun_peek_len(struct socket *sock)
{
	struct tun_file *tfile = container_of(sock, struct tun_file, socket);
	struct tun_struct *tun;
	int ret = 0;

	tun = tun_get(tfile);
	if (!tun)
		return 0;

	ret = PTR_RING_PEEK_CALL(&tfile->tx_ring, tun_ptr_peek_len);
	tun_put(tun);

	return ret;
}

/* Ops structure to mimic raw sockets with tun */
static const struct proto_ops tun_socket_ops = {
	.peek_len = tun_peek_len,
	.sendmsg = tun_sendmsg,
	.recvmsg = tun_recvmsg,
};

static struct proto tun_proto = {
	.name		= "tun",
	.owner		= THIS_MODULE,
	.obj_size	= sizeof(struct tun_file),
};

static int tun_flags(struct tun_struct *tun)
{
	return tun->flags & (TUN_FEATURES | IFF_PERSIST | IFF_TUN | IFF_TAP);
}

static ssize_t tun_flags_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return sysfs_emit(buf, "0x%x\n", tun_flags(tun));
}

static ssize_t owner_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return uid_valid(tun->owner)?
		sysfs_emit(buf, "%u\n",
			   from_kuid_munged(current_user_ns(), tun->owner)) :
		sysfs_emit(buf, "-1\n");
}

static ssize_t group_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return gid_valid(tun->group) ?
		sysfs_emit(buf, "%u\n",
			   from_kgid_munged(current_user_ns(), tun->group)) :
		sysfs_emit(buf, "-1\n");
}

static DEVICE_ATTR_RO(tun_flags);
static DEVICE_ATTR_RO(owner);
static DEVICE_ATTR_RO(group);

static struct attribute *tun_dev_attrs[] = {
	&dev_attr_tun_flags.attr,
	&dev_attr_owner.attr,
	&dev_attr_group.attr,
	NULL
};

static const struct attribute_group tun_attr_group = {
	.attrs = tun_dev_attrs
};

static int tun_set_iff(struct net *net, struct file *file, struct ifreq *ifr)
{
	struct tun_struct *tun;
	struct tun_file *tfile = file->private_data;
	struct net_device *dev;
	int err;

	if (tfile->detached)
		return -EINVAL;

	if ((ifr->ifr_flags & IFF_NAPI_FRAGS)) {
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (!(ifr->ifr_flags & IFF_NAPI) ||
		    (ifr->ifr_flags & TUN_TYPE_MASK) != IFF_TAP)
			return -EINVAL;
	}

	dev = __dev_get_by_name(net, ifr->ifr_name);
	if (dev) {
		if (ifr->ifr_flags & IFF_TUN_EXCL)
			return -EBUSY;
		if ((ifr->ifr_flags & IFF_TUN) && dev->netdev_ops == &tun_netdev_ops)
			tun = netdev_priv(dev);
		else if ((ifr->ifr_flags & IFF_TAP) && dev->netdev_ops == &tap_netdev_ops)
			tun = netdev_priv(dev);
		else
			return -EINVAL;

		if (!!(ifr->ifr_flags & IFF_MULTI_QUEUE) !=
		    !!(tun->flags & IFF_MULTI_QUEUE))
			return -EINVAL;

		if (tun_not_capable(tun))
			return -EPERM;
		err = security_tun_dev_open(tun->security);
		if (err < 0)
			return err;

		err = tun_attach(tun, file, ifr->ifr_flags & IFF_NOFILTER,
				 ifr->ifr_flags & IFF_NAPI,
				 ifr->ifr_flags & IFF_NAPI_FRAGS, true);
		if (err < 0)
			return err;

		if (tun->flags & IFF_MULTI_QUEUE &&
		    (tun->numqueues + tun->numdisabled > 1)) {
			/* One or more queue has already been attached, no need
			 * to initialize the device again.
			 */
			netdev_state_change(dev);
			return 0;
		}

		tun->flags = (tun->flags & ~TUN_FEATURES) |
			      (ifr->ifr_flags & TUN_FEATURES);

		netdev_state_change(dev);
	} else {
		char *name;
		unsigned long flags = 0;
		int queues = ifr->ifr_flags & IFF_MULTI_QUEUE ?
			     MAX_TAP_QUEUES : 1;

		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			return -EPERM;
		err = security_tun_dev_create();
		if (err < 0)
			return err;

		/* Set dev type */
		if (ifr->ifr_flags & IFF_TUN) {
			/* TUN device */
			flags |= IFF_TUN;
			name = "tun%d";
		} else if (ifr->ifr_flags & IFF_TAP) {
			/* TAP device */
			flags |= IFF_TAP;
			name = "tap%d";
		} else
			return -EINVAL;

		if (*ifr->ifr_name)
			name = ifr->ifr_name;

		dev = alloc_netdev_mqs(sizeof(struct tun_struct), name,
				       NET_NAME_UNKNOWN, tun_setup, queues,
				       queues);

		if (!dev)
			return -ENOMEM;

		dev_net_set(dev, net);
		dev->rtnl_link_ops = &tun_link_ops;
		dev->ifindex = tfile->ifindex;
		dev->sysfs_groups[0] = &tun_attr_group;

		tun = netdev_priv(dev);
		tun->dev = dev;
		tun->flags = flags;
		tun->txflt.count = 0;
		tun->vnet_hdr_sz = sizeof(struct virtio_net_hdr);

		tun->align = NET_SKB_PAD;
		tun->filter_attached = false;
		tun->sndbuf = tfile->socket.sk->sk_sndbuf;
		tun->rx_batched = 0;
		RCU_INIT_POINTER(tun->steering_prog, NULL);

		tun->ifr = ifr;
		tun->file = file;

		tun_net_initialize(dev);

		err = register_netdevice(tun->dev);
		if (err < 0) {
			free_netdev(dev);
			return err;
		}
		/* free_netdev() won't check refcnt, to avoid race
		 * with dev_put() we need publish tun after registration.
		 */
		rcu_assign_pointer(tfile->tun, tun);
	}

	if (ifr->ifr_flags & IFF_NO_CARRIER)
		netif_carrier_off(tun->dev);
	else
		netif_carrier_on(tun->dev);

	/* Make sure persistent devices do not get stuck in
	 * xoff state.
	 */
	if (netif_running(tun->dev))
		netif_tx_wake_all_queues(tun->dev);

	strcpy(ifr->ifr_name, tun->dev->name);
	return 0;
}

static void tun_get_iff(struct tun_struct *tun, struct ifreq *ifr)
{
	strcpy(ifr->ifr_name, tun->dev->name);

	ifr->ifr_flags = tun_flags(tun);

}

/* This is like a cut-down ethtool ops, except done via tun fd so no
 * privs required. */
static int set_offload(struct tun_struct *tun, unsigned long arg)
{
	netdev_features_t features = 0;

	if (arg & TUN_F_CSUM) {
		features |= NETIF_F_HW_CSUM;
		arg &= ~TUN_F_CSUM;

		if (arg & (TUN_F_TSO4|TUN_F_TSO6)) {
			if (arg & TUN_F_TSO_ECN) {
				features |= NETIF_F_TSO_ECN;
				arg &= ~TUN_F_TSO_ECN;
			}
			if (arg & TUN_F_TSO4)
				features |= NETIF_F_TSO;
			if (arg & TUN_F_TSO6)
				features |= NETIF_F_TSO6;
			arg &= ~(TUN_F_TSO4|TUN_F_TSO6);
		}

		arg &= ~TUN_F_UFO;

		/* TODO: for now USO4 and USO6 should work simultaneously */
		if (arg & TUN_F_USO4 && arg & TUN_F_USO6) {
			features |= NETIF_F_GSO_UDP_L4;
			arg &= ~(TUN_F_USO4 | TUN_F_USO6);
		}
	}

	/* This gives the user a way to test for new features in future by
	 * trying to set them. */
	if (arg)
		return -EINVAL;

	tun->set_features = features;
	tun->dev->wanted_features &= ~TUN_USER_FEATURES;
	tun->dev->wanted_features |= features;
	netdev_update_features(tun->dev);

	return 0;
}

static void tun_detach_filter(struct tun_struct *tun, int n)
{
	int i;
	struct tun_file *tfile;

	for (i = 0; i < n; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		lock_sock(tfile->socket.sk);
		sk_detach_filter(tfile->socket.sk);
		release_sock(tfile->socket.sk);
	}

	tun->filter_attached = false;
}

static int tun_attach_filter(struct tun_struct *tun)
{
	int i, ret = 0;
	struct tun_file *tfile;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		lock_sock(tfile->socket.sk);
		ret = sk_attach_filter(&tun->fprog, tfile->socket.sk);
		release_sock(tfile->socket.sk);
		if (ret) {
			tun_detach_filter(tun, i);
			return ret;
		}
	}

	tun->filter_attached = true;
	return ret;
}

static void tun_set_sndbuf(struct tun_struct *tun)
{
	struct tun_file *tfile;
	int i;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		tfile->socket.sk->sk_sndbuf = tun->sndbuf;
	}
}

static int tun_set_queue(struct file *file, struct ifreq *ifr)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;
	int ret = 0;

	rtnl_lock();

	if (ifr->ifr_flags & IFF_ATTACH_QUEUE) {
		tun = tfile->detached;
		if (!tun) {
			ret = -EINVAL;
			goto unlock;
		}
		ret = security_tun_dev_attach_queue(tun->security);
		if (ret < 0)
			goto unlock;
		ret = tun_attach(tun, file, false, tun->flags & IFF_NAPI,
				 tun->flags & IFF_NAPI_FRAGS, true);
	} else if (ifr->ifr_flags & IFF_DETACH_QUEUE) {
		tun = rtnl_dereference(tfile->tun);
		if (!tun || !(tun->flags & IFF_MULTI_QUEUE) || tfile->detached)
			ret = -EINVAL;
		else
			__tun_detach(tfile, false);
	} else
		ret = -EINVAL;

	if (ret >= 0)
		netdev_state_change(tun->dev);

unlock:
	rtnl_unlock();
	return ret;
}

static int tun_set_ebpf(struct tun_struct *tun, struct tun_prog __rcu **prog_p,
			void __user *data)
{
	struct bpf_prog *prog;
	int fd;

	if (copy_from_user(&fd, data, sizeof(fd)))
		return -EFAULT;

	if (fd == -1) {
		prog = NULL;
	} else {
		prog = bpf_prog_get_type(fd, BPF_PROG_TYPE_SOCKET_FILTER);
		if (IS_ERR(prog))
			return PTR_ERR(prog);
	}

	return __tun_set_ebpf(tun, prog_p, prog);
}

/* Return correct value for tun->dev->addr_len based on tun->dev->type. */
static unsigned char tun_get_addr_len(unsigned short type)
{
	switch (type) {
	case ARPHRD_IP6GRE:
	case ARPHRD_TUNNEL6:
		return sizeof(struct in6_addr);
	case ARPHRD_IPGRE:
	case ARPHRD_TUNNEL:
	case ARPHRD_SIT:
		return 4;
	case ARPHRD_ETHER:
		return ETH_ALEN;
	case ARPHRD_IEEE802154:
	case ARPHRD_IEEE802154_MONITOR:
		return IEEE802154_EXTENDED_ADDR_LEN;
	case ARPHRD_PHONET_PIPE:
	case ARPHRD_PPP:
	case ARPHRD_NONE:
		return 0;
	case ARPHRD_6LOWPAN:
		return EUI64_ADDR_LEN;
	case ARPHRD_FDDI:
		return FDDI_K_ALEN;
	case ARPHRD_HIPPI:
		return HIPPI_ALEN;
	case ARPHRD_IEEE802:
		return FC_ALEN;
	case ARPHRD_ROSE:
		return ROSE_ADDR_LEN;
	case ARPHRD_NETROM:
		return AX25_ADDR_LEN;
	case ARPHRD_LOCALTLK:
		return LTALK_ALEN;
	default:
		return 0;
	}
}

static long __tun_chr_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg, int ifreq_len)
{
	struct tun_file *tfile = file->private_data;
	struct net *net = sock_net(&tfile->sk);
	struct tun_struct *tun;
	void __user* argp = (void __user*)arg;
	unsigned int carrier;
	struct ifreq ifr;
	kuid_t owner;
	kgid_t group;
	int ifindex;
	int sndbuf;
	int vnet_hdr_sz;
	int le;
	int ret;
	bool do_notify = false;

	if (cmd == TUNSETIFF || cmd == TUNSETQUEUE ||
	    (_IOC_TYPE(cmd) == SOCK_IOC_TYPE && cmd != SIOCGSKNS)) {
		if (copy_from_user(&ifr, argp, ifreq_len))
			return -EFAULT;
	} else {
		memset(&ifr, 0, sizeof(ifr));
	}
	if (cmd == TUNGETFEATURES) {
		/* Currently this just means: "what IFF flags are valid?".
		 * This is needed because we never checked for invalid flags on
		 * TUNSETIFF.
		 */
		return put_user(IFF_TUN | IFF_TAP | IFF_NO_CARRIER |
				TUN_FEATURES, (unsigned int __user*)argp);
	} else if (cmd == TUNSETQUEUE) {
		return tun_set_queue(file, &ifr);
	} else if (cmd == SIOCGSKNS) {
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			return -EPERM;
		return open_related_ns(&net->ns, get_net_ns);
	}

	rtnl_lock();

	tun = tun_get(tfile);
	if (cmd == TUNSETIFF) {
		ret = -EEXIST;
		if (tun)
			goto unlock;

		ifr.ifr_name[IFNAMSIZ-1] = '\0';

		ret = tun_set_iff(net, file, &ifr);

		if (ret)
			goto unlock;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		goto unlock;
	}
	if (cmd == TUNSETIFINDEX) {
		ret = -EPERM;
		if (tun)
			goto unlock;

		ret = -EFAULT;
		if (copy_from_user(&ifindex, argp, sizeof(ifindex)))
			goto unlock;
		ret = -EINVAL;
		if (ifindex < 0)
			goto unlock;
		ret = 0;
		tfile->ifindex = ifindex;
		goto unlock;
	}

	ret = -EBADFD;
	if (!tun)
		goto unlock;

	netif_info(tun, drv, tun->dev, "tun_chr_ioctl cmd %u\n", cmd);

	net = dev_net(tun->dev);
	ret = 0;
	switch (cmd) {
	case TUNGETIFF:
		tun_get_iff(tun, &ifr);

		if (tfile->detached)
			ifr.ifr_flags |= IFF_DETACH_QUEUE;
		if (!tfile->socket.sk->sk_filter)
			ifr.ifr_flags |= IFF_NOFILTER;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case TUNSETNOCSUM:
		/* Disable/Enable checksum */

		/* [unimplemented] */
		netif_info(tun, drv, tun->dev, "ignored: set checksum %s\n",
			   arg ? "disabled" : "enabled");
		break;

	case TUNSETPERSIST:
		/* Disable/Enable persist mode. Keep an extra reference to the
		 * module to prevent the module being unprobed.
		 */
		if (arg && !(tun->flags & IFF_PERSIST)) {
			tun->flags |= IFF_PERSIST;
			__module_get(THIS_MODULE);
			do_notify = true;
		}
		if (!arg && (tun->flags & IFF_PERSIST)) {
			tun->flags &= ~IFF_PERSIST;
			module_put(THIS_MODULE);
			do_notify = true;
		}

		netif_info(tun, drv, tun->dev, "persist %s\n",
			   arg ? "enabled" : "disabled");
		break;

	case TUNSETOWNER:
		/* Set owner of the device */
		owner = make_kuid(current_user_ns(), arg);
		if (!uid_valid(owner)) {
			ret = -EINVAL;
			break;
		}
		tun->owner = owner;
		do_notify = true;
		netif_info(tun, drv, tun->dev, "owner set to %u\n",
			   from_kuid(&init_user_ns, tun->owner));
		break;

	case TUNSETGROUP:
		/* Set group of the device */
		group = make_kgid(current_user_ns(), arg);
		if (!gid_valid(group)) {
			ret = -EINVAL;
			break;
		}
		tun->group = group;
		do_notify = true;
		netif_info(tun, drv, tun->dev, "group set to %u\n",
			   from_kgid(&init_user_ns, tun->group));
		break;

	case TUNSETLINK:
		/* Only allow setting the type when the interface is down */
		if (tun->dev->flags & IFF_UP) {
			netif_info(tun, drv, tun->dev,
				   "Linktype set failed because interface is up\n");
			ret = -EBUSY;
		} else {
			ret = call_netdevice_notifiers(NETDEV_PRE_TYPE_CHANGE,
						       tun->dev);
			ret = notifier_to_errno(ret);
			if (ret) {
				netif_info(tun, drv, tun->dev,
					   "Refused to change device type\n");
				break;
			}
			tun->dev->type = (int) arg;
			tun->dev->addr_len = tun_get_addr_len(tun->dev->type);
			netif_info(tun, drv, tun->dev, "linktype set to %d\n",
				   tun->dev->type);
			call_netdevice_notifiers(NETDEV_POST_TYPE_CHANGE,
						 tun->dev);
		}
		break;

	case TUNSETDEBUG:
		tun->msg_enable = (u32)arg;
		break;

	case TUNSETOFFLOAD:
		ret = set_offload(tun, arg);
		break;

	case TUNSETTXFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = update_filter(&tun->txflt, (void __user *)arg);
		break;

	case SIOCGIFHWADDR:
		/* Get hw address */
		dev_get_mac_address(&ifr.ifr_hwaddr, net, tun->dev->name);
		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case SIOCSIFHWADDR:
		/* Set hw address */
		ret = dev_set_mac_address_user(tun->dev, &ifr.ifr_hwaddr, NULL);
		break;

	case TUNGETSNDBUF:
		sndbuf = tfile->socket.sk->sk_sndbuf;
		if (copy_to_user(argp, &sndbuf, sizeof(sndbuf)))
			ret = -EFAULT;
		break;

	case TUNSETSNDBUF:
		if (copy_from_user(&sndbuf, argp, sizeof(sndbuf))) {
			ret = -EFAULT;
			break;
		}
		if (sndbuf <= 0) {
			ret = -EINVAL;
			break;
		}

		tun->sndbuf = sndbuf;
		tun_set_sndbuf(tun);
		break;

	case TUNGETVNETHDRSZ:
		vnet_hdr_sz = tun->vnet_hdr_sz;
		if (copy_to_user(argp, &vnet_hdr_sz, sizeof(vnet_hdr_sz)))
			ret = -EFAULT;
		break;

	case TUNSETVNETHDRSZ:
		if (copy_from_user(&vnet_hdr_sz, argp, sizeof(vnet_hdr_sz))) {
			ret = -EFAULT;
			break;
		}
		if (vnet_hdr_sz < (int)sizeof(struct virtio_net_hdr)) {
			ret = -EINVAL;
			break;
		}

		tun->vnet_hdr_sz = vnet_hdr_sz;
		break;

	case TUNGETVNETLE:
		le = !!(tun->flags & TUN_VNET_LE);
		if (put_user(le, (int __user *)argp))
			ret = -EFAULT;
		break;

	case TUNSETVNETLE:
		if (get_user(le, (int __user *)argp)) {
			ret = -EFAULT;
			break;
		}
		if (le)
			tun->flags |= TUN_VNET_LE;
		else
			tun->flags &= ~TUN_VNET_LE;
		break;

	case TUNGETVNETBE:
		ret = tun_get_vnet_be(tun, argp);
		break;

	case TUNSETVNETBE:
		ret = tun_set_vnet_be(tun, argp);
		break;

	case TUNATTACHFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = -EFAULT;
		if (copy_from_user(&tun->fprog, argp, sizeof(tun->fprog)))
			break;

		ret = tun_attach_filter(tun);
		break;

	case TUNDETACHFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = 0;
		tun_detach_filter(tun, tun->numqueues);
		break;

	case TUNGETFILTER:
		ret = -EINVAL;
		if ((tun->flags & TUN_TYPE_MASK) != IFF_TAP)
			break;
		ret = -EFAULT;
		if (copy_to_user(argp, &tun->fprog, sizeof(tun->fprog)))
			break;
		ret = 0;
		break;

	case TUNSETSTEERINGEBPF:
		ret = tun_set_ebpf(tun, &tun->steering_prog, argp);
		break;

	case TUNSETFILTEREBPF:
		ret = tun_set_ebpf(tun, &tun->filter_prog, argp);
		break;

	case TUNSETCARRIER:
		ret = -EFAULT;
		if (copy_from_user(&carrier, argp, sizeof(carrier)))
			goto unlock;

		ret = tun_net_change_carrier(tun->dev, (bool)carrier);
		break;

	case TUNGETDEVNETNS:
		ret = -EPERM;
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			goto unlock;
		ret = open_related_ns(&net->ns, get_net_ns);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (do_notify)
		netdev_state_change(tun->dev);

unlock:
	rtnl_unlock();
	if (tun)
		tun_put(tun);
	return ret;
}

static long tun_chr_ioctl(struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	return __tun_chr_ioctl(file, cmd, arg, sizeof (struct ifreq));
}

#ifdef CONFIG_COMPAT
static long tun_chr_compat_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case TUNSETIFF:
	case TUNGETIFF:
	case TUNSETTXFILTER:
	case TUNGETSNDBUF:
	case TUNSETSNDBUF:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
		arg = (unsigned long)compat_ptr(arg);
		break;
	default:
		arg = (compat_ulong_t)arg;
		break;
	}

	/*
	 * compat_ifreq is shorter than ifreq, so we must not access beyond
	 * the end of that structure. All fields that are used in this
	 * driver are compatible though, we don't need to convert the
	 * contents.
	 */
	return __tun_chr_ioctl(file, cmd, arg, sizeof(struct compat_ifreq));
}
#endif /* CONFIG_COMPAT */

static int tun_chr_fasync(int fd, struct file *file, int on)
{
	struct tun_file *tfile = file->private_data;
	int ret;

	if ((ret = fasync_helper(fd, file, on, &tfile->fasync)) < 0)
		goto out;

	if (on) {
		__f_setown(file, task_pid(current), PIDTYPE_TGID, 0);
		tfile->flags |= TUN_FASYNC;
	} else
		tfile->flags &= ~TUN_FASYNC;
	ret = 0;
out:
	return ret;
}

static int tun_chr_open(struct inode *inode, struct file * file)
{
	struct net *net = current->nsproxy->net_ns;
	struct tun_file *tfile;

	tfile = (struct tun_file *)sk_alloc(net, AF_UNSPEC, GFP_KERNEL,
					    &tun_proto, 0);
	if (!tfile)
		return -ENOMEM;
	if (ptr_ring_init(&tfile->tx_ring, 0, GFP_KERNEL)) {
		sk_free(&tfile->sk);
		return -ENOMEM;
	}

	mutex_init(&tfile->napi_mutex);
	RCU_INIT_POINTER(tfile->tun, NULL);
	tfile->flags = 0;
	tfile->ifindex = 0;

	init_waitqueue_head(&tfile->socket.wq.wait);

	tfile->socket.file = file;
	tfile->socket.ops = &tun_socket_ops;

	sock_init_data_uid(&tfile->socket, &tfile->sk, current_fsuid());

	tfile->sk.sk_write_space = tun_sock_write_space;
	tfile->sk.sk_sndbuf = INT_MAX;

	file->private_data = tfile;
	INIT_LIST_HEAD(&tfile->next);

	sock_set_flag(&tfile->sk, SOCK_ZEROCOPY);

	/* tun groks IOCB_NOWAIT just fine, mark it as such */
	file->f_mode |= FMODE_NOWAIT;
	return 0;
}

static int tun_chr_close(struct inode *inode, struct file *file)
{
	struct tun_file *tfile = file->private_data;

	tun_detach(tfile, true);

	return 0;
}

#ifdef CONFIG_PROC_FS
static void tun_chr_show_fdinfo(struct seq_file *m, struct file *file)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));

	rtnl_lock();
	tun = tun_get(tfile);
	if (tun)
		tun_get_iff(tun, &ifr);
	rtnl_unlock();

	if (tun)
		tun_put(tun);

	seq_printf(m, "iff:\t%s\n", ifr.ifr_name);
}
#endif

static const struct file_operations tun_fops = {
	.owner	= THIS_MODULE,
	.llseek = no_llseek,
	.read_iter  = tun_chr_read_iter,
	.write_iter = tun_chr_write_iter,
	.poll	= tun_chr_poll,
	.unlocked_ioctl	= tun_chr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tun_chr_compat_ioctl,
#endif
	.open	= tun_chr_open,
	.release = tun_chr_close,
	.fasync = tun_chr_fasync,
#ifdef CONFIG_PROC_FS
	.show_fdinfo = tun_chr_show_fdinfo,
#endif
};

static struct miscdevice tun_miscdev = {
	.minor = TUN_MINOR,
	.name = "tun",
	.nodename = "net/tun",
	.fops = &tun_fops,
};

/* ethtool interface */

static void tun_default_link_ksettings(struct net_device *dev,
				       struct ethtool_link_ksettings *cmd)
{
	ethtool_link_ksettings_zero_link_mode(cmd, supported);
	ethtool_link_ksettings_zero_link_mode(cmd, advertising);
	cmd->base.speed		= SPEED_10000;
	cmd->base.duplex	= DUPLEX_FULL;
	cmd->base.port		= PORT_TP;
	cmd->base.phy_address	= 0;
	cmd->base.autoneg	= AUTONEG_DISABLE;
}

static int tun_get_link_ksettings(struct net_device *dev,
				  struct ethtool_link_ksettings *cmd)
{
	struct tun_struct *tun = netdev_priv(dev);

	memcpy(cmd, &tun->link_ksettings, sizeof(*cmd));
	return 0;
}

static int tun_set_link_ksettings(struct net_device *dev,
				  const struct ethtool_link_ksettings *cmd)
{
	struct tun_struct *tun = netdev_priv(dev);

	memcpy(&tun->link_ksettings, cmd, sizeof(*cmd));
	return 0;
}

static void tun_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct tun_struct *tun = netdev_priv(dev);

	strscpy(info->driver, DRV_NAME, sizeof(info->driver));
	strscpy(info->version, DRV_VERSION, sizeof(info->version));

	switch (tun->flags & TUN_TYPE_MASK) {
	case IFF_TUN:
		strscpy(info->bus_info, "tun", sizeof(info->bus_info));
		break;
	case IFF_TAP:
		strscpy(info->bus_info, "tap", sizeof(info->bus_info));
		break;
	}
}

static u32 tun_get_msglevel(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);

	return tun->msg_enable;
}

static void tun_set_msglevel(struct net_device *dev, u32 value)
{
	struct tun_struct *tun = netdev_priv(dev);

	tun->msg_enable = value;
}

static int tun_get_coalesce(struct net_device *dev,
			    struct ethtool_coalesce *ec,
			    struct kernel_ethtool_coalesce *kernel_coal,
			    struct netlink_ext_ack *extack)
{
	struct tun_struct *tun = netdev_priv(dev);

	ec->rx_max_coalesced_frames = tun->rx_batched;

	return 0;
}

static int tun_set_coalesce(struct net_device *dev,
			    struct ethtool_coalesce *ec,
			    struct kernel_ethtool_coalesce *kernel_coal,
			    struct netlink_ext_ack *extack)
{
	struct tun_struct *tun = netdev_priv(dev);

	if (ec->rx_max_coalesced_frames > NAPI_POLL_WEIGHT)
		tun->rx_batched = NAPI_POLL_WEIGHT;
	else
		tun->rx_batched = ec->rx_max_coalesced_frames;

	return 0;
}

static void tun_get_channels(struct net_device *dev,
			     struct ethtool_channels *channels)
{
	struct tun_struct *tun = netdev_priv(dev);

	channels->combined_count = tun->numqueues;
	channels->max_combined = tun->flags & IFF_MULTI_QUEUE ? MAX_TAP_QUEUES : 1;
}

static const struct ethtool_ops tun_ethtool_ops = {
	.supported_coalesce_params = ETHTOOL_COALESCE_RX_MAX_FRAMES,
	.get_drvinfo	= tun_get_drvinfo,
	.get_msglevel	= tun_get_msglevel,
	.set_msglevel	= tun_set_msglevel,
	.get_link	= ethtool_op_get_link,
	.get_channels   = tun_get_channels,
	.get_ts_info	= ethtool_op_get_ts_info,
	.get_coalesce   = tun_get_coalesce,
	.set_coalesce   = tun_set_coalesce,
	.get_link_ksettings = tun_get_link_ksettings,
	.set_link_ksettings = tun_set_link_ksettings,
};

static int tun_queue_resize(struct tun_struct *tun)
{
	struct net_device *dev = tun->dev;
	struct tun_file *tfile;
	struct ptr_ring **rings;
	int n = tun->numqueues + tun->numdisabled;
	int ret, i;

	rings = kmalloc_array(n, sizeof(*rings), GFP_KERNEL);
	if (!rings)
		return -ENOMEM;

	for (i = 0; i < tun->numqueues; i++) {
		tfile = rtnl_dereference(tun->tfiles[i]);
		rings[i] = &tfile->tx_ring;
	}
	list_for_each_entry(tfile, &tun->disabled, next)
		rings[i++] = &tfile->tx_ring;

	ret = ptr_ring_resize_multiple(rings, n,
				       dev->tx_queue_len, GFP_KERNEL,
				       tun_ptr_free);

	kfree(rings);
	return ret;
}

static int tun_device_event(struct notifier_block *unused,
			    unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct tun_struct *tun = netdev_priv(dev);
	int i;

	if (dev->rtnl_link_ops != &tun_link_ops)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_CHANGE_TX_QUEUE_LEN:
		if (tun_queue_resize(tun))
			return NOTIFY_BAD;
		break;
	case NETDEV_UP:
		for (i = 0; i < tun->numqueues; i++) {
			struct tun_file *tfile;

			tfile = rtnl_dereference(tun->tfiles[i]);
			tfile->socket.sk->sk_write_space(tfile->socket.sk);
		}
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block tun_notifier_block __read_mostly = {
	.notifier_call	= tun_device_event,
};

static int __init tun_init(void)
{
	int ret = 0;

	pr_info("%s, %s\n", DRV_DESCRIPTION, DRV_VERSION);

	ret = rtnl_link_register(&tun_link_ops);
	if (ret) {
		pr_err("Can't register link_ops\n");
		goto err_linkops;
	}

	ret = misc_register(&tun_miscdev);
	if (ret) {
		pr_err("Can't register misc device %d\n", TUN_MINOR);
		goto err_misc;
	}

	ret = register_netdevice_notifier(&tun_notifier_block);
	if (ret) {
		pr_err("Can't register netdevice notifier\n");
		goto err_notifier;
	}

	return  0;

err_notifier:
	misc_deregister(&tun_miscdev);
err_misc:
	rtnl_link_unregister(&tun_link_ops);
err_linkops:
	return ret;
}

static void __exit tun_cleanup(void)
{
	misc_deregister(&tun_miscdev);
	rtnl_link_unregister(&tun_link_ops);
	unregister_netdevice_notifier(&tun_notifier_block);
}

/* Get an underlying socket object from tun file.  Returns error unless file is
 * attached to a device.  The returned object works like a packet socket, it
 * can be used for sock_sendmsg/sock_recvmsg.  The caller is responsible for
 * holding a reference to the file for as long as the socket is in use. */
struct socket *tun_get_socket(struct file *file)
{
	struct tun_file *tfile;
	if (file->f_op != &tun_fops)
		return ERR_PTR(-EINVAL);
	tfile = file->private_data;
	if (!tfile)
		return ERR_PTR(-EBADFD);
	return &tfile->socket;
}
EXPORT_SYMBOL_GPL(tun_get_socket);

struct ptr_ring *tun_get_tx_ring(struct file *file)
{
	struct tun_file *tfile;

	if (file->f_op != &tun_fops)
		return ERR_PTR(-EINVAL);
	tfile = file->private_data;
	if (!tfile)
		return ERR_PTR(-EBADFD);
	return &tfile->tx_ring;
}
EXPORT_SYMBOL_GPL(tun_get_tx_ring);

module_init(tun_init);
module_exit(tun_cleanup);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_COPYRIGHT);
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(TUN_MINOR);
MODULE_ALIAS("devname:net/tun");