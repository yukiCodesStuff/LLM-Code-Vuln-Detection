{
	struct net_device *dev;
	struct inet6_dev *idev;

	for_each_netdev(net, dev) {
		idev = __in6_dev_get(dev);
		if (idev) {
			int changed = (!idev->cnf.forwarding) ^ (!newf);
			idev->cnf.forwarding = newf;
			if (changed)
				dev_forward_change(idev);
		}
	}
		if (idev) {
			int changed = (!idev->cnf.forwarding) ^ (!newf);
			idev->cnf.forwarding = newf;
			if (changed)
				dev_forward_change(idev);
		}
	}
}

static int addrconf_fixup_forwarding(struct ctl_table *table, int *p, int newf)
{
	struct net *net;
	int old;

	if (!rtnl_trylock())
		return restart_syscall();

	net = (struct net *)table->extra2;
	old = *p;
	*p = newf;

	if (p == &net->ipv6.devconf_dflt->forwarding) {
		rtnl_unlock();
		return 0;
	}

	if (p == &net->ipv6.devconf_all->forwarding) {
		net->ipv6.devconf_dflt->forwarding = newf;
		addrconf_forward_change(net, newf);
	} else if ((!newf) ^ (!old))
		dev_forward_change((struct inet6_dev *)table->extra1);
	rtnl_unlock();

	if (newf)
		rt6_purge_dflt_routers(net);
	return 1;
}
#endif

/* Nobody refers to this ifaddr, destroy it */
void inet6_ifa_finish_destroy(struct inet6_ifaddr *ifp)
{
	WARN_ON(!hlist_unhashed(&ifp->addr_lst));

#ifdef NET_REFCNT_DEBUG
	pr_debug("%s\n", __func__);
#endif

	in6_dev_put(ifp->idev);

	if (del_timer(&ifp->timer))
		pr_notice("Timer is still running, when freeing ifa=%p\n", ifp);

	if (ifp->state != INET6_IFADDR_STATE_DEAD) {
		pr_warn("Freeing alive inet6 address %p\n", ifp);
		return;
	}
	dst_release(&ifp->rt->dst);

	kfree_rcu(ifp, rcu);
}

static void
ipv6_link_dev_addr(struct inet6_dev *idev, struct inet6_ifaddr *ifp)
{
	struct list_head *p;
	int ifp_scope = ipv6_addr_src_scope(&ifp->addr);

	/*
	 * Each device address list is sorted in order of scope -
	 * global before linklocal.
	 */
	list_for_each(p, &idev->addr_list) {
		struct inet6_ifaddr *ifa
			= list_entry(p, struct inet6_ifaddr, if_list);
		if (ifp_scope >= ipv6_addr_src_scope(&ifa->addr))
			break;
	}

	list_add_tail(&ifp->if_list, p);
}

static u32 inet6_addr_hash(const struct in6_addr *addr)
{
	return hash_32(ipv6_addr_hash(addr), IN6_ADDR_HSIZE_SHIFT);
}

/* On success it returns ifp with increased reference count */

static struct inet6_ifaddr *
ipv6_add_addr(struct inet6_dev *idev, const struct in6_addr *addr, int pfxlen,
	      int scope, u32 flags)
{
	struct inet6_ifaddr *ifa = NULL;
	struct rt6_info *rt;
	unsigned int hash;
	int err = 0;
	int addr_type = ipv6_addr_type(addr);

	if (addr_type == IPV6_ADDR_ANY ||
	    addr_type & IPV6_ADDR_MULTICAST ||
	    (!(idev->dev->flags & IFF_LOOPBACK) &&
	     addr_type & IPV6_ADDR_LOOPBACK))
		return ERR_PTR(-EADDRNOTAVAIL);

	rcu_read_lock_bh();
	if (idev->dead) {
		err = -ENODEV;			/*XXX*/
		goto out2;
	}

	if (idev->cnf.disable_ipv6) {
		err = -EACCES;
		goto out2;
	}

	spin_lock(&addrconf_hash_lock);

	/* Ignore adding duplicate addresses on an interface */
	if (ipv6_chk_same_addr(dev_net(idev->dev), addr, idev->dev)) {
		ADBG(("ipv6_add_addr: already assigned\n"));
		err = -EEXIST;
		goto out;
	}

	ifa = kzalloc(sizeof(struct inet6_ifaddr), GFP_ATOMIC);

	if (ifa == NULL) {
		ADBG(("ipv6_add_addr: malloc failed\n"));
		err = -ENOBUFS;
		goto out;
	}

	rt = addrconf_dst_alloc(idev, addr, false);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		goto out;
	}

	ifa->addr = *addr;

	spin_lock_init(&ifa->lock);
	spin_lock_init(&ifa->state_lock);
	init_timer(&ifa->timer);
	INIT_HLIST_NODE(&ifa->addr_lst);
	ifa->timer.data = (unsigned long) ifa;
	ifa->scope = scope;
	ifa->prefix_len = pfxlen;
	ifa->flags = flags | IFA_F_TENTATIVE;
	ifa->cstamp = ifa->tstamp = jiffies;

	ifa->rt = rt;

	ifa->idev = idev;
	in6_dev_hold(idev);
	/* For caller */
	in6_ifa_hold(ifa);

	/* Add to big hash table */
	hash = inet6_addr_hash(addr);

	hlist_add_head_rcu(&ifa->addr_lst, &inet6_addr_lst[hash]);
	spin_unlock(&addrconf_hash_lock);

	write_lock(&idev->lock);
	/* Add to inet6_dev unicast addr list. */
	ipv6_link_dev_addr(idev, ifa);

#ifdef CONFIG_IPV6_PRIVACY
	if (ifa->flags&IFA_F_TEMPORARY) {
		list_add(&ifa->tmp_list, &idev->tempaddr_list);
		in6_ifa_hold(ifa);
	}
#endif

	in6_ifa_hold(ifa);
	write_unlock(&idev->lock);
out2:
	rcu_read_unlock_bh();

	if (likely(err == 0))
		atomic_notifier_call_chain(&inet6addr_chain, NETDEV_UP, ifa);
	else {
		kfree(ifa);
		ifa = ERR_PTR(err);
	}

	return ifa;
out:
	spin_unlock(&addrconf_hash_lock);
	goto out2;
}

/* This function wants to get referenced ifp and releases it before return */

static void ipv6_del_addr(struct inet6_ifaddr *ifp)
{
	struct inet6_ifaddr *ifa, *ifn;
	struct inet6_dev *idev = ifp->idev;
	int state;
	int deleted = 0, onlink = 0;
	unsigned long expires = jiffies;

	spin_lock_bh(&ifp->state_lock);
	state = ifp->state;
	ifp->state = INET6_IFADDR_STATE_DEAD;
	spin_unlock_bh(&ifp->state_lock);

	if (state == INET6_IFADDR_STATE_DEAD)
		goto out;

	spin_lock_bh(&addrconf_hash_lock);
	hlist_del_init_rcu(&ifp->addr_lst);
	spin_unlock_bh(&addrconf_hash_lock);

	write_lock_bh(&idev->lock);
#ifdef CONFIG_IPV6_PRIVACY
	if (ifp->flags&IFA_F_TEMPORARY) {
		list_del(&ifp->tmp_list);
		if (ifp->ifpub) {
			in6_ifa_put(ifp->ifpub);
			ifp->ifpub = NULL;
		}