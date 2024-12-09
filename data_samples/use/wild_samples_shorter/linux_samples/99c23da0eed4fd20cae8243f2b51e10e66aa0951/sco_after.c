	return err;
}

static int sco_send_frame(struct sock *sk, void *buf, int len,
			  unsigned int msg_flags)
{
	struct sco_conn *conn = sco_pi(sk)->conn;
	struct sk_buff *skb;
	int err;

	BT_DBG("sk %p len %d", sk, len);

	skb = bt_skb_send_alloc(sk, len, msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		return err;

	memcpy(skb_put(skb, len), buf, len);
	hci_send_sco(conn->hcon, skb);

	return len;
}
			    size_t len)
{
	struct sock *sk = sock->sk;
	void *buf;
	int err;

	BT_DBG("sock %p, sk %p", sock, sk);

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

static void sco_conn_defer_accept(struct hci_conn *conn, u16 setting)