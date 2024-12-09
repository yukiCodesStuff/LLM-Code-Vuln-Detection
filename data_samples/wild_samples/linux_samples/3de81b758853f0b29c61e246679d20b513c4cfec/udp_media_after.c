		if (!dev) {
			err = -ENODEV;
			goto err;
		}
		udp_conf.family = AF_INET;
		udp_conf.local_ip.s_addr = htonl(INADDR_ANY);
		udp_conf.use_udp_checksums = false;
		ub->ifindex = dev->ifindex;
		if (tipc_mtu_bad(dev, sizeof(struct iphdr) +
				      sizeof(struct udphdr))) {
			err = -EINVAL;
			goto err;
		}