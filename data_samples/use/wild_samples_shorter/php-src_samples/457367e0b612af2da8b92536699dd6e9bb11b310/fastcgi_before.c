			if (sa.sa_inet.sin_addr.s_addr == INADDR_NONE) {
				struct hostent *hep;

				hep = gethostbyname(host);
				if (!hep || hep->h_addrtype != AF_INET || !hep->h_addr_list[0]) {
					fprintf(stderr, "Cannot resolve host name '%s'!\n", host);
					return -1;
				} else if (hep->h_addr_list[1]) {

		if (!req->tcp) {
			unsigned int in_len = tmp > UINT_MAX ? UINT_MAX : (unsigned int)tmp;
	
			ret = read(req->fd, ((char*)buf)+n, in_len);
		} else {
			int in_len = tmp > INT_MAX ? INT_MAX : (int)tmp;
