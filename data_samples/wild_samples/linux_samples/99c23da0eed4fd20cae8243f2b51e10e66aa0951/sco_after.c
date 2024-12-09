	} else {
		sk->sk_state = BT_CONNECT;
		sco_sock_set_timer(sk, sk->sk_sndtimeo);
	}

	return err;
}

static int sco_send_frame(struct sock *sk, void *buf, int len,
			  unsigned int msg_flags)
{
	struct sco_conn *conn = sco_pi(sk)->conn;
	struct sk_buff *skb;
	int err;

	/* Check outgoing MTU */
	if (len > conn->mtu)
		return -EINVAL;

	BT_DBG("sk %p len %d", sk, len);

	skb = bt_skb_send_alloc(sk, len, msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		return err;

	memcpy(skb_put(skb, len), buf, len);
	hci_send_sco(conn->hcon, skb);

	return len;
}

static void sco_recv_frame(struct sco_conn *conn, struct sk_buff *skb)
{
	struct sock *sk;

	sco_conn_lock(conn);
	sk = conn->sk;
	sco_conn_unlock(conn);

	if (!sk)
		goto drop;

	BT_DBG("sk %p len %u", sk, skb->len);

	if (sk->sk_state != BT_CONNECTED)
		goto drop;

	if (!sock_queue_rcv_skb(sk, skb))
		return;

drop:
	kfree_skb(skb);
}

/* -------- Socket interface ---------- */
static struct sock *__sco_get_sock_listen_by_addr(bdaddr_t *ba)
{
	struct sock *sk;

	sk_for_each(sk, &sco_sk_list.head) {
		if (sk->sk_state != BT_LISTEN)
			continue;

		if (!bacmp(&sco_pi(sk)->src, ba))
			return sk;
	}
{
	struct sco_conn *conn = sco_pi(sk)->conn;
	struct sk_buff *skb;
	int err;

	/* Check outgoing MTU */
	if (len > conn->mtu)
		return -EINVAL;

	BT_DBG("sk %p len %d", sk, len);

	skb = bt_skb_send_alloc(sk, len, msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		return err;

	memcpy(skb_put(skb, len), buf, len);
	hci_send_sco(conn->hcon, skb);

	return len;
}

static void sco_recv_frame(struct sco_conn *conn, struct sk_buff *skb)
{
	struct sock *sk;

	sco_conn_lock(conn);
	sk = conn->sk;
	sco_conn_unlock(conn);

	if (!sk)
		goto drop;

	BT_DBG("sk %p len %u", sk, skb->len);

	if (sk->sk_state != BT_CONNECTED)
		goto drop;

	if (!sock_queue_rcv_skb(sk, skb))
		return;

drop:
	kfree_skb(skb);
}

/* -------- Socket interface ---------- */
static struct sock *__sco_get_sock_listen_by_addr(bdaddr_t *ba)
{
	struct sock *sk;

	sk_for_each(sk, &sco_sk_list.head) {
		if (sk->sk_state != BT_LISTEN)
			continue;

		if (!bacmp(&sco_pi(sk)->src, ba))
			return sk;
	}
{
	struct sock *sk = sock->sk;
	void *buf;
	int err;

	BT_DBG("sock %p, sk %p", sock, sk);

	err = sock_error(sk);
	if (err)
		return err;

	if (msg->msg_flags & MSG_OOB)
		return -EOPNOTSUPP;

	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (memcpy_from_msg(buf, msg, len)) {
		kfree(buf);
		return -EFAULT;
	}

	lock_sock(sk);

	if (sk->sk_state == BT_CONNECTED)
		err = sco_send_frame(sk, buf, len, msg->msg_flags);
	else
		err = -ENOTCONN;

	release_sock(sk);
	kfree(buf);
	return err;
}
{
	struct sock *sk = sock->sk;
	void *buf;
	int err;

	BT_DBG("sock %p, sk %p", sock, sk);

	err = sock_error(sk);
	if (err)
		return err;

	if (msg->msg_flags & MSG_OOB)
		return -EOPNOTSUPP;

	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (memcpy_from_msg(buf, msg, len)) {
		kfree(buf);
		return -EFAULT;
	}