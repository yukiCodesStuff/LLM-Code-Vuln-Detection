	sk = chan->sk;

	hci_conn_hold(conn->hcon);

	bacpy(&bt_sk(sk)->src, conn->src);
	bacpy(&bt_sk(sk)->dst, conn->dst);
