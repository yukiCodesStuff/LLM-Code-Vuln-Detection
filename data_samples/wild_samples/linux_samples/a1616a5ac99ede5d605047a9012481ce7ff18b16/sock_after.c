		if (!isock) {
			sockfd_put(csock);
			return err;
		}
		ca.name[sizeof(ca.name)-1] = 0;

		err = hidp_connection_add(&ca, csock, isock);
		if (!err && copy_to_user(argp, &ca, sizeof(ca)))
			err = -EFAULT;

		sockfd_put(csock);
		sockfd_put(isock);

		return err;

	case HIDPCONNDEL:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(&cd, argp, sizeof(cd)))
			return -EFAULT;

		return hidp_connection_del(&cd);

	case HIDPGETCONNLIST:
		if (copy_from_user(&cl, argp, sizeof(cl)))
			return -EFAULT;

		if (cl.cnum <= 0)
			return -EINVAL;

		err = hidp_get_connlist(&cl);
		if (!err && copy_to_user(argp, &cl, sizeof(cl)))
			return -EFAULT;

		return err;

	case HIDPGETCONNINFO:
		if (copy_from_user(&ci, argp, sizeof(ci)))
			return -EFAULT;

		err = hidp_get_conninfo(&ci);
		if (!err && copy_to_user(argp, &ci, sizeof(ci)))
			return -EFAULT;

		return err;
	}

	return -EINVAL;
}

static int hidp_sock_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	return do_hidp_sock_ioctl(sock, cmd, (void __user *)arg);
}

#ifdef CONFIG_COMPAT
struct compat_hidp_connadd_req {
	int   ctrl_sock;	/* Connected control socket */
	int   intr_sock;	/* Connected interrupt socket */
	__u16 parser;
	__u16 rd_size;
	compat_uptr_t rd_data;
	__u8  country;
	__u8  subclass;
	__u16 vendor;
	__u16 product;
	__u16 version;
	__u32 flags;
	__u32 idle_to;
	char  name[128];
};

static int hidp_sock_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	void __user *argp = compat_ptr(arg);
	int err;

	if (cmd == HIDPGETCONNLIST) {
		struct hidp_connlist_req cl;
		u32 __user *p = argp;
		u32 uci;

		if (get_user(cl.cnum, p) || get_user(uci, p + 1))
			return -EFAULT;

		cl.ci = compat_ptr(uci);

		if (cl.cnum <= 0)
			return -EINVAL;

		err = hidp_get_connlist(&cl);

		if (!err && put_user(cl.cnum, p))
			err = -EFAULT;

		return err;
	} else if (cmd == HIDPCONNADD) {
		struct compat_hidp_connadd_req ca32;
		struct hidp_connadd_req ca;
		struct socket *csock;
		struct socket *isock;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(&ca32, (void __user *) arg, sizeof(ca32)))
			return -EFAULT;

		ca.ctrl_sock = ca32.ctrl_sock;
		ca.intr_sock = ca32.intr_sock;
		ca.parser = ca32.parser;
		ca.rd_size = ca32.rd_size;
		ca.rd_data = compat_ptr(ca32.rd_data);
		ca.country = ca32.country;
		ca.subclass = ca32.subclass;
		ca.vendor = ca32.vendor;
		ca.product = ca32.product;
		ca.version = ca32.version;
		ca.flags = ca32.flags;
		ca.idle_to = ca32.idle_to;
		memcpy(ca.name, ca32.name, 128);

		csock = sockfd_lookup(ca.ctrl_sock, &err);
		if (!csock)
			return err;

		isock = sockfd_lookup(ca.intr_sock, &err);
		if (!isock) {
			sockfd_put(csock);
			return err;
		}

		err = hidp_connection_add(&ca, csock, isock);
		if (!err && copy_to_user(argp, &ca32, sizeof(ca32)))
			err = -EFAULT;

		sockfd_put(csock);
		sockfd_put(isock);

		return err;
	}