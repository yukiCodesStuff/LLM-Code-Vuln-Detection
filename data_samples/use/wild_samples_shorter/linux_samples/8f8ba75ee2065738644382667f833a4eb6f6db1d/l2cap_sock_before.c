
	BT_DBG("sock %p, sk %p", sock, sk);

	addr->sa_family = AF_BLUETOOTH;
	*len = sizeof(struct sockaddr_l2);

	if (peer) {

	chan = l2cap_chan_create();
	if (!chan) {
		l2cap_sock_kill(sk);
		return NULL;
	}

	l2cap_chan_hold(chan);