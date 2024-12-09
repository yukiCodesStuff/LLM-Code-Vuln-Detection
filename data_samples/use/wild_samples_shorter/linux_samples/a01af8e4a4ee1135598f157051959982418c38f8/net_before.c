	size_t hdr_size;
	struct socket *sock;

	sock = rcu_dereference_check(vq->private_data,
				     lockdep_is_held(&vq->mutex));
	if (!sock)
		return;

	wmem = atomic_read(&sock->sk->sk_wmem_alloc);