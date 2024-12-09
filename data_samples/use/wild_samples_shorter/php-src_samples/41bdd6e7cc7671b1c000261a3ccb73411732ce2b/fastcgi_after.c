			if (sa.sa_inet.sin_addr.s_addr == INADDR_NONE) {
				struct hostent *hep;

				if(strlen(host) > MAXHOSTNAMELEN) {
					hep = NULL;
				} else {
					hep = gethostbyname(host);
				}
				if (!hep || hep->h_addrtype != AF_INET || !hep->h_addr_list[0]) {
					fprintf(stderr, "Cannot resolve host name '%s'!\n", host);
					return -1;
				} else if (hep->h_addr_list[1]) {