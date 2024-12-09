	struct msghdr msg = {
		.msg_name = NULL,
		.msg_namelen = 0,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_iov = vq->iov,
		.msg_flags = MSG_DONTWAIT,
	};
	size_t len, total_len = 0;
	int err, wmem;
	size_t hdr_size;
	struct socket *sock;

	/* TODO: check that we are running from vhost_worker?
	 * Not sure it's worth it, it's straight-forward enough. */
	sock = rcu_dereference_check(vq->private_data, 1);
	if (!sock)
		return;

	wmem = atomic_read(&sock->sk->sk_wmem_alloc);
	if (wmem >= sock->sk->sk_sndbuf) {
		mutex_lock(&vq->mutex);
		tx_poll_start(net, sock);
		mutex_unlock(&vq->mutex);
		return;
	}