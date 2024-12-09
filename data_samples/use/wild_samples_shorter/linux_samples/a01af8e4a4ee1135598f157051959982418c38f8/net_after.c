	size_t hdr_size;
	struct socket *sock;

	/* TODO: check that we are running from vhost_worker?
	 * Not sure it's worth it, it's straight-forward enough. */
	sock = rcu_dereference_check(vq->private_data, 1);
	if (!sock)
		return;

	wmem = atomic_read(&sock->sk->sk_wmem_alloc);