{
	struct compat_ifconf ifc32;
	struct ifconf ifc;
	struct ifconf __user *uifc;
	struct compat_ifreq __user *ifr32;
	struct ifreq __user *ifr;
	unsigned int i, j;
	int err;

	if (copy_from_user(&ifc32, uifc32, sizeof(struct compat_ifconf)))
		return -EFAULT;

	memset(&ifc, 0, sizeof(ifc));
	if (ifc32.ifcbuf == 0) {
		ifc32.ifc_len = 0;
		ifc.ifc_len = 0;
		ifc.ifc_req = NULL;
		uifc = compat_alloc_user_space(sizeof(struct ifconf));
	} else {
		size_t len = ((ifc32.ifc_len / sizeof(struct compat_ifreq)) + 1) *
			sizeof(struct ifreq);
		uifc = compat_alloc_user_space(sizeof(struct ifconf) + len);
		ifc.ifc_len = len;
		ifr = ifc.ifc_req = (void __user *)(uifc + 1);
		ifr32 = compat_ptr(ifc32.ifcbuf);
		for (i = 0; i < ifc32.ifc_len; i += sizeof(struct compat_ifreq)) {
			if (copy_in_user(ifr, ifr32, sizeof(struct compat_ifreq)))
				return -EFAULT;
			ifr++;
			ifr32++;
		}
	}
	if (copy_to_user(uifc, &ifc, sizeof(struct ifconf)))
		return -EFAULT;

	err = dev_ioctl(net, SIOCGIFCONF, uifc);
	if (err)
		return err;

	if (copy_from_user(&ifc, uifc, sizeof(struct ifconf)))
		return -EFAULT;

	ifr = ifc.ifc_req;
	ifr32 = compat_ptr(ifc32.ifcbuf);
	for (i = 0, j = 0;
	     i + sizeof(struct compat_ifreq) <= ifc32.ifc_len && j < ifc.ifc_len;
	     i += sizeof(struct compat_ifreq), j += sizeof(struct ifreq)) {
		if (copy_in_user(ifr32, ifr, sizeof(struct compat_ifreq)))
			return -EFAULT;
		ifr32++;
		ifr++;
	}

	if (ifc32.ifcbuf == 0) {
		/* Translate from 64-bit structure multiple to
		 * a 32-bit one.
		 */
		i = ifc.ifc_len;
		i = ((i / sizeof(struct ifreq)) * sizeof(struct compat_ifreq));
		ifc32.ifc_len = i;
	} else {
		ifc32.ifc_len = i;
	}
	if (copy_to_user(uifc32, &ifc32, sizeof(struct compat_ifconf)))
		return -EFAULT;

	return 0;
}

static int ethtool_ioctl(struct net *net, struct compat_ifreq __user *ifr32)
{
	struct compat_ethtool_rxnfc __user *compat_rxnfc;
	bool convert_in = false, convert_out = false;
	size_t buf_size = ALIGN(sizeof(struct ifreq), 8);
	struct ethtool_rxnfc __user *rxnfc;
	struct ifreq __user *ifr;
	u32 rule_cnt = 0, actual_rule_cnt;
	u32 ethcmd;
	u32 data;
	int ret;

	if (get_user(data, &ifr32->ifr_ifru.ifru_data))
		return -EFAULT;

	compat_rxnfc = compat_ptr(data);

	if (get_user(ethcmd, &compat_rxnfc->cmd))
		return -EFAULT;

	/* Most ethtool structures are defined without padding.
	 * Unfortunately struct ethtool_rxnfc is an exception.
	 */
	switch (ethcmd) {
	default:
		break;
	case ETHTOOL_GRXCLSRLALL:
		/* Buffer size is variable */
		if (get_user(rule_cnt, &compat_rxnfc->rule_cnt))
			return -EFAULT;
		if (rule_cnt > KMALLOC_MAX_SIZE / sizeof(u32))
			return -ENOMEM;
		buf_size += rule_cnt * sizeof(u32);
		/* fall through */
	case ETHTOOL_GRXRINGS:
	case ETHTOOL_GRXCLSRLCNT:
	case ETHTOOL_GRXCLSRULE:
	case ETHTOOL_SRXCLSRLINS:
		convert_out = true;
		/* fall through */
	case ETHTOOL_SRXCLSRLDEL:
		buf_size += sizeof(struct ethtool_rxnfc);
		convert_in = true;
		break;
	}

	ifr = compat_alloc_user_space(buf_size);
	rxnfc = (void *)ifr + ALIGN(sizeof(struct ifreq), 8);

	if (copy_in_user(&ifr->ifr_name, &ifr32->ifr_name, IFNAMSIZ))
		return -EFAULT;

	if (put_user(convert_in ? rxnfc : compat_ptr(data),
		     &ifr->ifr_ifru.ifru_data))
		return -EFAULT;

	if (convert_in) {
		/* We expect there to be holes between fs.m_ext and
		 * fs.ring_cookie and at the end of fs, but nowhere else.
		 */
		BUILD_BUG_ON(offsetof(struct compat_ethtool_rxnfc, fs.m_ext) +
			     sizeof(compat_rxnfc->fs.m_ext) !=
			     offsetof(struct ethtool_rxnfc, fs.m_ext) +
			     sizeof(rxnfc->fs.m_ext));
		BUILD_BUG_ON(
			offsetof(struct compat_ethtool_rxnfc, fs.location) -
			offsetof(struct compat_ethtool_rxnfc, fs.ring_cookie) !=
			offsetof(struct ethtool_rxnfc, fs.location) -
			offsetof(struct ethtool_rxnfc, fs.ring_cookie));

		if (copy_in_user(rxnfc, compat_rxnfc,
				 (void *)(&rxnfc->fs.m_ext + 1) -
				 (void *)rxnfc) ||
		    copy_in_user(&rxnfc->fs.ring_cookie,
				 &compat_rxnfc->fs.ring_cookie,
				 (void *)(&rxnfc->fs.location + 1) -
				 (void *)&rxnfc->fs.ring_cookie) ||
		    copy_in_user(&rxnfc->rule_cnt, &compat_rxnfc->rule_cnt,
				 sizeof(rxnfc->rule_cnt)))
			return -EFAULT;
	}

	ret = dev_ioctl(net, SIOCETHTOOL, ifr);
	if (ret)
		return ret;

	if (convert_out) {
		if (copy_in_user(compat_rxnfc, rxnfc,
				 (const void *)(&rxnfc->fs.m_ext + 1) -
				 (const void *)rxnfc) ||
		    copy_in_user(&compat_rxnfc->fs.ring_cookie,
				 &rxnfc->fs.ring_cookie,
				 (const void *)(&rxnfc->fs.location + 1) -
				 (const void *)&rxnfc->fs.ring_cookie) ||
		    copy_in_user(&compat_rxnfc->rule_cnt, &rxnfc->rule_cnt,
				 sizeof(rxnfc->rule_cnt)))
			return -EFAULT;

		if (ethcmd == ETHTOOL_GRXCLSRLALL) {
			/* As an optimisation, we only copy the actual
			 * number of rules that the underlying
			 * function returned.  Since Mallory might
			 * change the rule count in user memory, we
			 * check that it is less than the rule count
			 * originally given (as the user buffer size),
			 * which has been range-checked.
			 */
			if (get_user(actual_rule_cnt, &rxnfc->rule_cnt))
				return -EFAULT;
			if (actual_rule_cnt < rule_cnt)
				rule_cnt = actual_rule_cnt;
			if (copy_in_user(&compat_rxnfc->rule_locs[0],
					 &rxnfc->rule_locs[0],
					 rule_cnt * sizeof(u32)))
				return -EFAULT;
		}
	}

	return 0;
}

static int compat_siocwandev(struct net *net, struct compat_ifreq __user *uifr32)
{
	void __user *uptr;
	compat_uptr_t uptr32;
	struct ifreq __user *uifr;

	uifr = compat_alloc_user_space(sizeof(*uifr));
	if (copy_in_user(uifr, uifr32, sizeof(struct compat_ifreq)))
		return -EFAULT;

	if (get_user(uptr32, &uifr32->ifr_settings.ifs_ifsu))
		return -EFAULT;

	uptr = compat_ptr(uptr32);

	if (put_user(uptr, &uifr->ifr_settings.ifs_ifsu.raw_hdlc))
		return -EFAULT;

	return dev_ioctl(net, SIOCWANDEV, uifr);
}

static int bond_ioctl(struct net *net, unsigned int cmd,
			 struct compat_ifreq __user *ifr32)
{
	struct ifreq kifr;
	struct ifreq __user *uifr;
	mm_segment_t old_fs;
	int err;
	u32 data;
	void __user *datap;

	switch (cmd) {
	case SIOCBONDENSLAVE:
	case SIOCBONDRELEASE:
	case SIOCBONDSETHWADDR:
	case SIOCBONDCHANGEACTIVE:
		if (copy_from_user(&kifr, ifr32, sizeof(struct compat_ifreq)))
			return -EFAULT;

		old_fs = get_fs();
		set_fs(KERNEL_DS);
		err = dev_ioctl(net, cmd,
				(struct ifreq __user __force *) &kifr);
		set_fs(old_fs);

		return err;
	case SIOCBONDSLAVEINFOQUERY:
	case SIOCBONDINFOQUERY:
		uifr = compat_alloc_user_space(sizeof(*uifr));
		if (copy_in_user(&uifr->ifr_name, &ifr32->ifr_name, IFNAMSIZ))
			return -EFAULT;

		if (get_user(data, &ifr32->ifr_ifru.ifru_data))
			return -EFAULT;

		datap = compat_ptr(data);
		if (put_user(datap, &uifr->ifr_ifru.ifru_data))
			return -EFAULT;

		return dev_ioctl(net, cmd, uifr);
	default:
		return -ENOIOCTLCMD;
	}
}

static int siocdevprivate_ioctl(struct net *net, unsigned int cmd,
				 struct compat_ifreq __user *u_ifreq32)
{
	struct ifreq __user *u_ifreq64;
	char tmp_buf[IFNAMSIZ];
	void __user *data64;
	u32 data32;

	if (copy_from_user(&tmp_buf[0], &(u_ifreq32->ifr_ifrn.ifrn_name[0]),
			   IFNAMSIZ))
		return -EFAULT;
	if (__get_user(data32, &u_ifreq32->ifr_ifru.ifru_data))
		return -EFAULT;
	data64 = compat_ptr(data32);

	u_ifreq64 = compat_alloc_user_space(sizeof(*u_ifreq64));

	/* Don't check these user accesses, just let that get trapped
	 * in the ioctl handler instead.
	 */
	if (copy_to_user(&u_ifreq64->ifr_ifrn.ifrn_name[0], &tmp_buf[0],
			 IFNAMSIZ))
		return -EFAULT;
	if (__put_user(data64, &u_ifreq64->ifr_ifru.ifru_data))
		return -EFAULT;

	return dev_ioctl(net, cmd, u_ifreq64);
}

static int dev_ifsioc(struct net *net, struct socket *sock,
			 unsigned int cmd, struct compat_ifreq __user *uifr32)
{
	struct ifreq __user *uifr;
	int err;

	uifr = compat_alloc_user_space(sizeof(*uifr));
	if (copy_in_user(uifr, uifr32, sizeof(*uifr32)))
		return -EFAULT;

	err = sock_do_ioctl(net, sock, cmd, (unsigned long)uifr);

	if (!err) {
		switch (cmd) {
		case SIOCGIFFLAGS:
		case SIOCGIFMETRIC:
		case SIOCGIFMTU:
		case SIOCGIFMEM:
		case SIOCGIFHWADDR:
		case SIOCGIFINDEX:
		case SIOCGIFADDR:
		case SIOCGIFBRDADDR:
		case SIOCGIFDSTADDR:
		case SIOCGIFNETMASK:
		case SIOCGIFPFLAGS:
		case SIOCGIFTXQLEN:
		case SIOCGMIIPHY:
		case SIOCGMIIREG:
			if (copy_in_user(uifr32, uifr, sizeof(*uifr32)))
				err = -EFAULT;
			break;
		}
	}
	return err;
}

static int compat_sioc_ifmap(struct net *net, unsigned int cmd,
			struct compat_ifreq __user *uifr32)
{
	struct ifreq ifr;
	struct compat_ifmap __user *uifmap32;
	mm_segment_t old_fs;
	int err;

	uifmap32 = &uifr32->ifr_ifru.ifru_map;
	err = copy_from_user(&ifr, uifr32, sizeof(ifr.ifr_name));
	err |= __get_user(ifr.ifr_map.mem_start, &uifmap32->mem_start);
	err |= __get_user(ifr.ifr_map.mem_end, &uifmap32->mem_end);
	err |= __get_user(ifr.ifr_map.base_addr, &uifmap32->base_addr);
	err |= __get_user(ifr.ifr_map.irq, &uifmap32->irq);
	err |= __get_user(ifr.ifr_map.dma, &uifmap32->dma);
	err |= __get_user(ifr.ifr_map.port, &uifmap32->port);
	if (err)
		return -EFAULT;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	err = dev_ioctl(net, cmd, (void  __user __force *)&ifr);
	set_fs(old_fs);

	if (cmd == SIOCGIFMAP && !err) {
		err = copy_to_user(uifr32, &ifr, sizeof(ifr.ifr_name));
		err |= __put_user(ifr.ifr_map.mem_start, &uifmap32->mem_start);
		err |= __put_user(ifr.ifr_map.mem_end, &uifmap32->mem_end);
		err |= __put_user(ifr.ifr_map.base_addr, &uifmap32->base_addr);
		err |= __put_user(ifr.ifr_map.irq, &uifmap32->irq);
		err |= __put_user(ifr.ifr_map.dma, &uifmap32->dma);
		err |= __put_user(ifr.ifr_map.port, &uifmap32->port);
		if (err)
			err = -EFAULT;
	}
	return err;
}

static int compat_siocshwtstamp(struct net *net, struct compat_ifreq __user *uifr32)
{
	void __user *uptr;
	compat_uptr_t uptr32;
	struct ifreq __user *uifr;

	uifr = compat_alloc_user_space(sizeof(*uifr));
	if (copy_in_user(uifr, uifr32, sizeof(struct compat_ifreq)))
		return -EFAULT;

	if (get_user(uptr32, &uifr32->ifr_data))
		return -EFAULT;

	uptr = compat_ptr(uptr32);

	if (put_user(uptr, &uifr->ifr_data))
		return -EFAULT;

	return dev_ioctl(net, SIOCSHWTSTAMP, uifr);
}

struct rtentry32 {
	u32		rt_pad1;
	struct sockaddr rt_dst;         /* target address               */
	struct sockaddr rt_gateway;     /* gateway addr (RTF_GATEWAY)   */
	struct sockaddr rt_genmask;     /* target network mask (IP)     */
	unsigned short	rt_flags;
	short		rt_pad2;
	u32		rt_pad3;
	unsigned char	rt_tos;
	unsigned char	rt_class;
	short		rt_pad4;
	short		rt_metric;      /* +1 for binary compatibility! */
	/* char * */ u32 rt_dev;        /* forcing the device at add    */
	u32		rt_mtu;         /* per route MTU/Window         */
	u32		rt_window;      /* Window clamping              */
	unsigned short  rt_irtt;        /* Initial RTT                  */
};

struct in6_rtmsg32 {
	struct in6_addr		rtmsg_dst;
	struct in6_addr		rtmsg_src;
	struct in6_addr		rtmsg_gateway;
	u32			rtmsg_type;
	u16			rtmsg_dst_len;
	u16			rtmsg_src_len;
	u32			rtmsg_metric;
	u32			rtmsg_info;
	u32			rtmsg_flags;
	s32			rtmsg_ifindex;
};

static int routing_ioctl(struct net *net, struct socket *sock,
			 unsigned int cmd, void __user *argp)
{
	int ret;
	void *r = NULL;
	struct in6_rtmsg r6;
	struct rtentry r4;
	char devname[16];
	u32 rtdev;
	mm_segment_t old_fs = get_fs();

	if (sock && sock->sk && sock->sk->sk_family == AF_INET6) { /* ipv6 */
		struct in6_rtmsg32 __user *ur6 = argp;
		ret = copy_from_user(&r6.rtmsg_dst, &(ur6->rtmsg_dst),
			3 * sizeof(struct in6_addr));
		ret |= __get_user(r6.rtmsg_type, &(ur6->rtmsg_type));
		ret |= __get_user(r6.rtmsg_dst_len, &(ur6->rtmsg_dst_len));
		ret |= __get_user(r6.rtmsg_src_len, &(ur6->rtmsg_src_len));
		ret |= __get_user(r6.rtmsg_metric, &(ur6->rtmsg_metric));
		ret |= __get_user(r6.rtmsg_info, &(ur6->rtmsg_info));
		ret |= __get_user(r6.rtmsg_flags, &(ur6->rtmsg_flags));
		ret |= __get_user(r6.rtmsg_ifindex, &(ur6->rtmsg_ifindex));

		r = (void *) &r6;
	} else { /* ipv4 */
		struct rtentry32 __user *ur4 = argp;
		ret = copy_from_user(&r4.rt_dst, &(ur4->rt_dst),
					3 * sizeof(struct sockaddr));
		ret |= __get_user(r4.rt_flags, &(ur4->rt_flags));
		ret |= __get_user(r4.rt_metric, &(ur4->rt_metric));
		ret |= __get_user(r4.rt_mtu, &(ur4->rt_mtu));
		ret |= __get_user(r4.rt_window, &(ur4->rt_window));
		ret |= __get_user(r4.rt_irtt, &(ur4->rt_irtt));
		ret |= __get_user(rtdev, &(ur4->rt_dev));
		if (rtdev) {
			ret |= copy_from_user(devname, compat_ptr(rtdev), 15);
			r4.rt_dev = (char __user __force *)devname;
			devname[15] = 0;
		} else
			r4.rt_dev = NULL;

		r = (void *) &r4;
	}

	if (ret) {
		ret = -EFAULT;
		goto out;
	}

	set_fs(KERNEL_DS);
	ret = sock_do_ioctl(net, sock, cmd, (unsigned long) r);
	set_fs(old_fs);

out:
	return ret;
}

/* Since old style bridge ioctl's endup using SIOCDEVPRIVATE
 * for some operations; this forces use of the newer bridge-utils that
 * use compatible ioctls
 */
static int old_bridge_ioctl(compat_ulong_t __user *argp)
{
	compat_ulong_t tmp;

	if (get_user(tmp, argp))
		return -EFAULT;
	if (tmp == BRCTL_GET_VERSION)
		return BRCTL_VERSION + 1;
	return -EINVAL;
}

static int compat_sock_ioctl_trans(struct file *file, struct socket *sock,
			 unsigned int cmd, unsigned long arg)
{
	void __user *argp = compat_ptr(arg);
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);

	if (cmd >= SIOCDEVPRIVATE && cmd <= (SIOCDEVPRIVATE + 15))
		return siocdevprivate_ioctl(net, cmd, argp);

	switch (cmd) {
	case SIOCSIFBR:
	case SIOCGIFBR:
		return old_bridge_ioctl(argp);
	case SIOCGIFNAME:
		return dev_ifname32(net, argp);
	case SIOCGIFCONF:
		return dev_ifconf(net, argp);
	case SIOCETHTOOL:
		return ethtool_ioctl(net, argp);
	case SIOCWANDEV:
		return compat_siocwandev(net, argp);
	case SIOCGIFMAP:
	case SIOCSIFMAP:
		return compat_sioc_ifmap(net, cmd, argp);
	case SIOCBONDENSLAVE:
	case SIOCBONDRELEASE:
	case SIOCBONDSETHWADDR:
	case SIOCBONDSLAVEINFOQUERY:
	case SIOCBONDINFOQUERY:
	case SIOCBONDCHANGEACTIVE:
		return bond_ioctl(net, cmd, argp);
	case SIOCADDRT:
	case SIOCDELRT:
		return routing_ioctl(net, sock, cmd, argp);
	case SIOCGSTAMP:
		return do_siocgstamp(net, sock, cmd, argp);
	case SIOCGSTAMPNS:
		return do_siocgstampns(net, sock, cmd, argp);
	case SIOCSHWTSTAMP:
		return compat_siocshwtstamp(net, argp);

	case FIOSETOWN:
	case SIOCSPGRP:
	case FIOGETOWN:
	case SIOCGPGRP:
	case SIOCBRADDBR:
	case SIOCBRDELBR:
	case SIOCGIFVLAN:
	case SIOCSIFVLAN:
	case SIOCADDDLCI:
	case SIOCDELDLCI:
		return sock_ioctl(file, cmd, arg);

	case SIOCGIFFLAGS:
	case SIOCSIFFLAGS:
	case SIOCGIFMETRIC:
	case SIOCSIFMETRIC:
	case SIOCGIFMTU:
	case SIOCSIFMTU:
	case SIOCGIFMEM:
	case SIOCSIFMEM:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
	case SIOCADDMULTI:
	case SIOCDELMULTI:
	case SIOCGIFINDEX:
	case SIOCGIFADDR:
	case SIOCSIFADDR:
	case SIOCSIFHWBROADCAST:
	case SIOCDIFADDR:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCSIFPFLAGS:
	case SIOCGIFPFLAGS:
	case SIOCGIFTXQLEN:
	case SIOCSIFTXQLEN:
	case SIOCBRADDIF:
	case SIOCBRDELIF:
	case SIOCSIFNAME:
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		return dev_ifsioc(net, sock, cmd, argp);

	case SIOCSARP:
	case SIOCGARP:
	case SIOCDARP:
	case SIOCATMARK:
		return sock_do_ioctl(net, sock, cmd, arg);
	}

	return -ENOIOCTLCMD;
}

static long compat_sock_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	struct socket *sock = file->private_data;
	int ret = -ENOIOCTLCMD;
	struct sock *sk;
	struct net *net;

	sk = sock->sk;
	net = sock_net(sk);

	if (sock->ops->compat_ioctl)
		ret = sock->ops->compat_ioctl(sock, cmd, arg);

	if (ret == -ENOIOCTLCMD &&
	    (cmd >= SIOCIWFIRST && cmd <= SIOCIWLAST))
		ret = compat_wext_handle_ioctl(net, cmd, arg);

	if (ret == -ENOIOCTLCMD)
		ret = compat_sock_ioctl_trans(file, sock, cmd, arg);

	return ret;
}
#endif

int kernel_bind(struct socket *sock, struct sockaddr *addr, int addrlen)
{
	return sock->ops->bind(sock, addr, addrlen);
}
EXPORT_SYMBOL(kernel_bind);

int kernel_listen(struct socket *sock, int backlog)
{
	return sock->ops->listen(sock, backlog);
}
EXPORT_SYMBOL(kernel_listen);

int kernel_accept(struct socket *sock, struct socket **newsock, int flags)
{
	struct sock *sk = sock->sk;
	int err;

	err = sock_create_lite(sk->sk_family, sk->sk_type, sk->sk_protocol,
			       newsock);
	if (err < 0)
		goto done;

	err = sock->ops->accept(sock, *newsock, flags);
	if (err < 0) {
		sock_release(*newsock);
		*newsock = NULL;
		goto done;
	}

	(*newsock)->ops = sock->ops;
	__module_get((*newsock)->ops->owner);

done:
	return err;
}
EXPORT_SYMBOL(kernel_accept);

int kernel_connect(struct socket *sock, struct sockaddr *addr, int addrlen,
		   int flags)
{
	return sock->ops->connect(sock, addr, addrlen, flags);
}
EXPORT_SYMBOL(kernel_connect);

int kernel_getsockname(struct socket *sock, struct sockaddr *addr,
			 int *addrlen)
{
	return sock->ops->getname(sock, addr, addrlen, 0);
}
EXPORT_SYMBOL(kernel_getsockname);

int kernel_getpeername(struct socket *sock, struct sockaddr *addr,
			 int *addrlen)
{
	return sock->ops->getname(sock, addr, addrlen, 1);
}
EXPORT_SYMBOL(kernel_getpeername);

int kernel_getsockopt(struct socket *sock, int level, int optname,
			char *optval, int *optlen)
{
	mm_segment_t oldfs = get_fs();
	char __user *uoptval;
	int __user *uoptlen;
	int err;

	uoptval = (char __user __force *) optval;
	uoptlen = (int __user __force *) optlen;

	set_fs(KERNEL_DS);
	if (level == SOL_SOCKET)
		err = sock_getsockopt(sock, level, optname, uoptval, uoptlen);
	else
		err = sock->ops->getsockopt(sock, level, optname, uoptval,
					    uoptlen);
	set_fs(oldfs);
	return err;
}
EXPORT_SYMBOL(kernel_getsockopt);

int kernel_setsockopt(struct socket *sock, int level, int optname,
			char *optval, unsigned int optlen)
{
	mm_segment_t oldfs = get_fs();
	char __user *uoptval;
	int err;

	uoptval = (char __user __force *) optval;

	set_fs(KERNEL_DS);
	if (level == SOL_SOCKET)
		err = sock_setsockopt(sock, level, optname, uoptval, optlen);
	else
		err = sock->ops->setsockopt(sock, level, optname, uoptval,
					    optlen);
	set_fs(oldfs);
	return err;
}
EXPORT_SYMBOL(kernel_setsockopt);

int kernel_sendpage(struct socket *sock, struct page *page, int offset,
		    size_t size, int flags)
{
	sock_update_classid(sock->sk);

	if (sock->ops->sendpage)
		return sock->ops->sendpage(sock, page, offset, size, flags);

	return sock_no_sendpage(sock, page, offset, size, flags);
}
EXPORT_SYMBOL(kernel_sendpage);

int kernel_sock_ioctl(struct socket *sock, int cmd, unsigned long arg)
{
	mm_segment_t oldfs = get_fs();
	int err;

	set_fs(KERNEL_DS);
	err = sock->ops->ioctl(sock, cmd, arg);
	set_fs(oldfs);

	return err;
}
EXPORT_SYMBOL(kernel_sock_ioctl);

int kernel_sock_shutdown(struct socket *sock, enum sock_shutdown_cmd how)
{
	return sock->ops->shutdown(sock, how);
}
EXPORT_SYMBOL(kernel_sock_shutdown);