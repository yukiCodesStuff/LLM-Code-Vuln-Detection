	lsa->l2tp_family = AF_INET6;
	lsa->l2tp_flowinfo = 0;
	lsa->l2tp_scope_id = 0;
	if (peer) {
		if (!lsk->peer_conn_id)
			return -ENOTCONN;
		lsa->l2tp_conn_id = lsk->peer_conn_id;