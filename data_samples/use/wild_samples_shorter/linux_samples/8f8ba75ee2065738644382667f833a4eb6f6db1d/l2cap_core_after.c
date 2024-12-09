	sk = chan->sk;

	hci_conn_hold(conn->hcon);
	conn->hcon->disc_timeout = HCI_DISCONN_TIMEOUT;

	bacpy(&bt_sk(sk)->src, conn->src);
	bacpy(&bt_sk(sk)->dst, conn->dst);
