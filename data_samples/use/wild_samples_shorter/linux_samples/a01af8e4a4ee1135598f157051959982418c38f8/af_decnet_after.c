			if (r_len > sizeof(struct linkinfo_dn))
				r_len = sizeof(struct linkinfo_dn);

			memset(&link, 0, sizeof(link));

			switch(sock->state) {
				case SS_CONNECTING:
					link.idn_linkstate = LL_CONNECTING;
					break;