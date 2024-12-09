	tfile->socket.file = file;
	tfile->socket.ops = &tun_socket_ops;

	sock_init_data_uid(&tfile->socket, &tfile->sk, inode->i_uid);

	tfile->sk.sk_write_space = tun_sock_write_space;
	tfile->sk.sk_sndbuf = INT_MAX;
