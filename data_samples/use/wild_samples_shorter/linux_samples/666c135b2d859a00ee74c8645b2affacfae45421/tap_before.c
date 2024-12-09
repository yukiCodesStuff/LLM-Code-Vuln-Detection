	q->sock.state = SS_CONNECTED;
	q->sock.file = file;
	q->sock.ops = &tap_socket_ops;
	sock_init_data_uid(&q->sock, &q->sk, inode->i_uid);
	q->sk.sk_write_space = tap_sock_write_space;
	q->sk.sk_destruct = tap_sock_destruct;
	q->flags = IFF_VNET_HDR | IFF_NO_PI | IFF_TAP;
	q->vnet_hdr_sz = sizeof(struct virtio_net_hdr);