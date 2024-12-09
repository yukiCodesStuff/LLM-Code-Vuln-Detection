	if (ptr_ring_init(&q->ring, tap->dev->tx_queue_len, GFP_KERNEL)) {
		sk_free(&q->sk);
		goto err;
	}

	init_waitqueue_head(&q->sock.wq.wait);
	q->sock.type = SOCK_RAW;
	q->sock.state = SS_CONNECTED;
	q->sock.file = file;
	q->sock.ops = &tap_socket_ops;
	sock_init_data_uid(&q->sock, &q->sk, current_fsuid());
	q->sk.sk_write_space = tap_sock_write_space;
	q->sk.sk_destruct = tap_sock_destruct;
	q->flags = IFF_VNET_HDR | IFF_NO_PI | IFF_TAP;
	q->vnet_hdr_sz = sizeof(struct virtio_net_hdr);

	/*
	 * so far only KVM virtio_net uses tap, enable zero copy between
	 * guest kernel and host kernel when lower device supports zerocopy
	 *
	 * The macvlan supports zerocopy iff the lower device supports zero
	 * copy so we don't have to look at the lower device directly.
	 */
	if ((tap->dev->features & NETIF_F_HIGHDMA) && (tap->dev->features & NETIF_F_SG))
		sock_set_flag(&q->sk, SOCK_ZEROCOPY);

	err = tap_set_queue(tap, file, q);
	if (err) {
		/* tap_sock_destruct() will take care of freeing ptr_ring */
		goto err_put;
	}