	if (sk) {
		bh_lock_sock(sk);
		sco_sock_clear_timer(sk);
		sco_chan_del(sk, err);
		bh_unlock_sock(sk);

		sco_conn_lock(conn);
		conn->sk = NULL;
		sco_pi(sk)->conn = NULL;
		sco_conn_unlock(conn);

		if (conn->hcon)
			hci_conn_put(conn->hcon);

		sco_sock_kill(sk);
	}

	hcon->sco_data = NULL;
	kfree(conn);
	return 0;
}

static int sco_chan_add(struct sco_conn *conn, struct sock *sk,
			struct sock *parent)
{
	int err = 0;

	sco_conn_lock(conn);
	if (conn->sk)
		err = -EBUSY;
	else
		__sco_chan_add(conn, sk, parent);

	sco_conn_unlock(conn);
	return err;
}

static int sco_connect(struct sock *sk)
{
	bdaddr_t *src = &bt_sk(sk)->src;
	bdaddr_t *dst = &bt_sk(sk)->dst;
	struct sco_conn *conn;
	struct hci_conn *hcon;
	struct hci_dev  *hdev;
	int err, type;

	BT_DBG("%s -> %s", batostr(src), batostr(dst));

	hdev = hci_get_route(dst, src);
	if (!hdev)
		return -EHOSTUNREACH;

	hci_dev_lock(hdev);

	if (lmp_esco_capable(hdev) && !disable_esco)
		type = ESCO_LINK;
	else
		type = SCO_LINK;

	hcon = hci_connect(hdev, type, dst, BDADDR_BREDR, BT_SECURITY_LOW,
			   HCI_AT_NO_BONDING);
	if (IS_ERR(hcon)) {
		err = PTR_ERR(hcon);
		goto done;
	}

	conn = sco_conn_add(hcon);
	if (!conn) {
		hci_conn_put(hcon);
		err = -ENOMEM;
		goto done;
	}

	/* Update source addr of the socket */
	bacpy(src, conn->src);

	err = sco_chan_add(conn, sk, NULL);
	if (err)
		goto done;

	if (hcon->state == BT_CONNECTED) {
		sco_sock_clear_timer(sk);
		sk->sk_state = BT_CONNECTED;
	} else {
		sk->sk_state = BT_CONNECT;
		sco_sock_set_timer(sk, sk->sk_sndtimeo);
	}
{
	struct sco_conn *conn;

	conn = sco_pi(sk)->conn;

	BT_DBG("sk %p, conn %p, err %d", sk, conn, err);

	sk->sk_state = BT_CLOSED;
	sk->sk_err   = err;
	sk->sk_state_change(sk);

	sock_set_flag(sk, SOCK_ZAPPED);
}

static void sco_conn_ready(struct sco_conn *conn)
{
	struct sock *parent;
	struct sock *sk = conn->sk;

	BT_DBG("conn %p", conn);

	sco_conn_lock(conn);

	if (sk) {
		sco_sock_clear_timer(sk);
		bh_lock_sock(sk);
		sk->sk_state = BT_CONNECTED;
		sk->sk_state_change(sk);
		bh_unlock_sock(sk);
	} else {
		parent = sco_get_sock_listen(conn->src);
		if (!parent)
			goto done;

		bh_lock_sock(parent);

		sk = sco_sock_alloc(sock_net(parent), NULL,
				    BTPROTO_SCO, GFP_ATOMIC);
		if (!sk) {
			bh_unlock_sock(parent);
			goto done;
		}

		sco_sock_init(sk, parent);

		bacpy(&bt_sk(sk)->src, conn->src);
		bacpy(&bt_sk(sk)->dst, conn->dst);

		hci_conn_hold(conn->hcon);
		__sco_chan_add(conn, sk, parent);

		sk->sk_state = BT_CONNECTED;

		/* Wake up parent */
		parent->sk_data_ready(parent, 1);

		bh_unlock_sock(parent);
	}

done:
	sco_conn_unlock(conn);
}