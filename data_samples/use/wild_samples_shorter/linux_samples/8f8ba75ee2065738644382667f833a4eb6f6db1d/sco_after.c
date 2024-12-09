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

	BT_DBG("sk %p, conn %p, err %d", sk, conn, err);

	sk->sk_state = BT_CLOSED;
	sk->sk_err   = err;
	sk->sk_state_change(sk);
