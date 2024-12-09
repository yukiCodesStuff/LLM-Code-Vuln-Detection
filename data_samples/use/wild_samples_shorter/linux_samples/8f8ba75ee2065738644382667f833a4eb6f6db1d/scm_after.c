	for (i=0, cmfptr=(__force int __user *)CMSG_DATA(cm); i<fdmax;
	     i++, cmfptr++)
	{
		struct socket *sock;
		int new_fd;
		err = security_file_receive(fp[i]);
		if (err)
			break;
		}
		/* Bump the usage count and install the file. */
		get_file(fp[i]);
		sock = sock_from_file(fp[i], &err);
		if (sock)
			sock_update_netprioidx(sock->sk, current);
		fd_install(new_fd, fp[i]);
	}

	if (i > 0)