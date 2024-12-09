
	info = nla_data(attr);
	list_for_each_entry_rcu(laddr, address_list, list) {
		memcpy(info, &laddr->a, addrlen);
		info += addrlen;
	}

	return 0;
	info = nla_data(attr);
	list_for_each_entry(from, &asoc->peer.transport_addr_list,
			    transports) {
		memcpy(info, &from->ipaddr, addrlen);
		info += addrlen;
	}

	return 0;