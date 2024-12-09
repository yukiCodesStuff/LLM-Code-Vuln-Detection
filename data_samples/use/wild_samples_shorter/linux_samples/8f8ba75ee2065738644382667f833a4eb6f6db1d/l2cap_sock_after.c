
	BT_DBG("sock %p, sk %p", sock, sk);

	memset(la, 0, sizeof(struct sockaddr_l2));
	addr->sa_family = AF_BLUETOOTH;
	*len = sizeof(struct sockaddr_l2);

	if (peer) {

	chan = l2cap_chan_create();
	if (!chan) {
		sk_free(sk);
		return NULL;
	}

	l2cap_chan_hold(chan);