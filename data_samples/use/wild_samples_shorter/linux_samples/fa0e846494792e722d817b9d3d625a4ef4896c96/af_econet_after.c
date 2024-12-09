
	mutex_lock(&econet_mutex);

        if (saddr == NULL || msg->msg_namelen < sizeof(struct sockaddr_ec)) {
                mutex_unlock(&econet_mutex);
                return -EINVAL;
        }
        addr.station = saddr->addr.station;
        addr.net = saddr->addr.net;
        port = saddr->port;
        cb = saddr->cb;

	/* Look for a device with the right network number. */
	dev = net2dev_map[addr.net];


		eb = (struct ec_cb *)&skb->cb;

		eb->cookie = saddr->cookie;
		eb->sec = *saddr;
		eb->sent = ec_tx_done;
