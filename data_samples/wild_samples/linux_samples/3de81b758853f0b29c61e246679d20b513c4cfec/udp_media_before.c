		if (!dev) {
			err = -ENODEV;
			goto err;
		}
		udp_conf.family = AF_INET;
		udp_conf.local_ip.s_addr = htonl(INADDR_ANY);
		udp_conf.use_udp_checksums = false;
		ub->ifindex = dev->ifindex;
		b->mtu = dev->mtu - sizeof(struct iphdr)
			- sizeof(struct udphdr);
#if IS_ENABLED(CONFIG_IPV6)
	} else if (local.proto == htons(ETH_P_IPV6)) {
		udp_conf.family = AF_INET6;
		udp_conf.use_udp6_tx_checksums = true;
		udp_conf.use_udp6_rx_checksums = true;
		udp_conf.local_ip6 = in6addr_any;
		b->mtu = 1280;
#endif
	} else {
		err = -EAFNOSUPPORT;
		goto err;
	}
	udp_conf.local_udp_port = local.port;
	err = udp_sock_create(net, &udp_conf, &ub->ubsock);
	if (err)
		goto err;
	tuncfg.sk_user_data = ub;
	tuncfg.encap_type = 1;
	tuncfg.encap_rcv = tipc_udp_recv;
	tuncfg.encap_destroy = NULL;
	setup_udp_tunnel_sock(net, ub->ubsock, &tuncfg);

	/**
	 * The bcast media address port is used for all peers and the ip
	 * is used if it's a multicast address.
	 */
	memcpy(&b->bcast_addr.value, &remote, sizeof(remote));
	if (tipc_udp_is_mcast_addr(&remote))
		err = enable_mcast(ub, &remote);
	else
		err = tipc_udp_rcast_add(b, &remote);
	if (err)
		goto err;

	return 0;
err:
	if (ub->ubsock)
		udp_tunnel_sock_release(ub->ubsock);
	kfree(ub);
	return err;
}