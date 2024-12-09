		udp_conf.local_ip.s_addr = htonl(INADDR_ANY);
		udp_conf.use_udp_checksums = false;
		ub->ifindex = dev->ifindex;
		if (tipc_mtu_bad(dev, sizeof(struct iphdr) +
				      sizeof(struct udphdr))) {
			err = -EINVAL;
			goto err;
		}
		b->mtu = dev->mtu - sizeof(struct iphdr)
			- sizeof(struct udphdr);
#if IS_ENABLED(CONFIG_IPV6)
	} else if (local.proto == htons(ETH_P_IPV6)) {