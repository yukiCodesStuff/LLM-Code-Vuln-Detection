
	info = nla_data(attr);
	list_for_each_entry_rcu(laddr, address_list, list) {
		memcpy(info, &laddr->a, sizeof(laddr->a));
		memset(info + sizeof(laddr->a), 0, addrlen - sizeof(laddr->a));
		info += addrlen;
	}

	return 0;
	info = nla_data(attr);
	list_for_each_entry(from, &asoc->peer.transport_addr_list,
			    transports) {
		memcpy(info, &from->ipaddr, sizeof(from->ipaddr));
		memset(info + sizeof(from->ipaddr), 0,
		       addrlen - sizeof(from->ipaddr));
		info += addrlen;
	}

	return 0;