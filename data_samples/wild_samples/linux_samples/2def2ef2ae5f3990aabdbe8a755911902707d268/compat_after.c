{
	int datagrams;
	struct timespec ktspec;

	if (flags & MSG_CMSG_COMPAT)
		return -EINVAL;

	if (timeout == NULL)
		return __sys_recvmmsg(fd, (struct mmsghdr __user *)mmsg, vlen,
				      flags | MSG_CMSG_COMPAT, NULL);

	if (compat_get_timespec(&ktspec, timeout))
		return -EFAULT;

	datagrams = __sys_recvmmsg(fd, (struct mmsghdr __user *)mmsg, vlen,
				   flags | MSG_CMSG_COMPAT, &ktspec);
	if (datagrams > 0 && compat_put_timespec(&ktspec, timeout))
		datagrams = -EFAULT;

	return datagrams;
}

asmlinkage long compat_sys_socketcall(int call, u32 __user *args)
{
	int ret;
	u32 a[6];
	u32 a0, a1;

	if (call < SYS_SOCKET || call > SYS_SENDMMSG)
		return -EINVAL;
	if (copy_from_user(a, args, nas[call]))
		return -EFAULT;
	a0 = a[0];
	a1 = a[1];

	switch (call) {
	case SYS_SOCKET:
		ret = sys_socket(a0, a1, a[2]);
		break;
	case SYS_BIND:
		ret = sys_bind(a0, compat_ptr(a1), a[2]);
		break;
	case SYS_CONNECT:
		ret = sys_connect(a0, compat_ptr(a1), a[2]);
		break;
	case SYS_LISTEN:
		ret = sys_listen(a0, a1);
		break;
	case SYS_ACCEPT:
		ret = sys_accept4(a0, compat_ptr(a1), compat_ptr(a[2]), 0);
		break;
	case SYS_GETSOCKNAME:
		ret = sys_getsockname(a0, compat_ptr(a1), compat_ptr(a[2]));
		break;
	case SYS_GETPEERNAME:
		ret = sys_getpeername(a0, compat_ptr(a1), compat_ptr(a[2]));
		break;
	case SYS_SOCKETPAIR:
		ret = sys_socketpair(a0, a1, a[2], compat_ptr(a[3]));
		break;
	case SYS_SEND:
		ret = sys_send(a0, compat_ptr(a1), a[2], a[3]);
		break;
	case SYS_SENDTO:
		ret = sys_sendto(a0, compat_ptr(a1), a[2], a[3], compat_ptr(a[4]), a[5]);
		break;
	case SYS_RECV:
		ret = compat_sys_recv(a0, compat_ptr(a1), a[2], a[3]);
		break;
	case SYS_RECVFROM:
		ret = compat_sys_recvfrom(a0, compat_ptr(a1), a[2], a[3],
					  compat_ptr(a[4]), compat_ptr(a[5]));
		break;
	case SYS_SHUTDOWN:
		ret = sys_shutdown(a0, a1);
		break;
	case SYS_SETSOCKOPT:
		ret = compat_sys_setsockopt(a0, a1, a[2],
				compat_ptr(a[3]), a[4]);
		break;
	case SYS_GETSOCKOPT:
		ret = compat_sys_getsockopt(a0, a1, a[2],
				compat_ptr(a[3]), compat_ptr(a[4]));
		break;
	case SYS_SENDMSG:
		ret = compat_sys_sendmsg(a0, compat_ptr(a1), a[2]);
		break;
	case SYS_SENDMMSG:
		ret = compat_sys_sendmmsg(a0, compat_ptr(a1), a[2], a[3]);
		break;
	case SYS_RECVMSG:
		ret = compat_sys_recvmsg(a0, compat_ptr(a1), a[2]);
		break;
	case SYS_RECVMMSG:
		ret = compat_sys_recvmmsg(a0, compat_ptr(a1), a[2], a[3],
					  compat_ptr(a[4]));
		break;
	case SYS_ACCEPT4:
		ret = sys_accept4(a0, compat_ptr(a1), compat_ptr(a[2]), a[3]);
		break;
	default:
		ret = -EINVAL;
		break;
	}