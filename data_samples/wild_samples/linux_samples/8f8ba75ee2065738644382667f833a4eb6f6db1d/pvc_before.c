{
	struct sockaddr_atmpvc *addr;
	struct atm_vcc *vcc = ATM_SD(sock);

	if (!vcc->dev || !test_bit(ATM_VF_ADDR, &vcc->flags))
		return -ENOTCONN;
	*sockaddr_len = sizeof(struct sockaddr_atmpvc);
	addr = (struct sockaddr_atmpvc *)sockaddr;
	addr->sap_family = AF_ATMPVC;
	addr->sap_addr.itf = vcc->dev->number;
	addr->sap_addr.vpi = vcc->vpi;
	addr->sap_addr.vci = vcc->vci;
	return 0;
}

static const struct proto_ops pvc_proto_ops = {
	.family =	PF_ATMPVC,
	.owner =	THIS_MODULE,

	.release =	vcc_release,
	.bind =		pvc_bind,
	.connect =	pvc_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	pvc_getname,
	.poll =		vcc_poll,
	.ioctl =	vcc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = vcc_compat_ioctl,
#endif
	.listen =	sock_no_listen,
	.shutdown =	pvc_shutdown,
	.setsockopt =	pvc_setsockopt,
	.getsockopt =	pvc_getsockopt,
	.sendmsg =	vcc_sendmsg,
	.recvmsg =	vcc_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};


static int pvc_create(struct net *net, struct socket *sock, int protocol,
		      int kern)
{
	if (net != &init_net)
		return -EAFNOSUPPORT;

	sock->ops = &pvc_proto_ops;
	return vcc_create(net, sock, protocol, PF_ATMPVC);
}