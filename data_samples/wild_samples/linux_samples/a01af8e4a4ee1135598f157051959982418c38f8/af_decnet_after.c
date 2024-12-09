	switch(optname) {
		case DSO_CONDATA:
			if (r_len > sizeof(struct optdata_dn))
				r_len = sizeof(struct optdata_dn);
			r_data = &scp->conndata_in;
			break;

		case DSO_DISDATA:
			if (r_len > sizeof(struct optdata_dn))
				r_len = sizeof(struct optdata_dn);
			r_data = &scp->discdata_in;
			break;

		case DSO_CONACCESS:
			if (r_len > sizeof(struct accessdata_dn))
				r_len = sizeof(struct accessdata_dn);
			r_data = &scp->accessdata;
			break;

		case DSO_ACCEPTMODE:
			if (r_len > sizeof(unsigned char))
				r_len = sizeof(unsigned char);
			r_data = &scp->accept_mode;
			break;

		case DSO_LINKINFO:
			if (r_len > sizeof(struct linkinfo_dn))
				r_len = sizeof(struct linkinfo_dn);

			memset(&link, 0, sizeof(link));

			switch(sock->state) {
				case SS_CONNECTING:
					link.idn_linkstate = LL_CONNECTING;
					break;
				case SS_DISCONNECTING:
					link.idn_linkstate = LL_DISCONNECTING;
					break;
				case SS_CONNECTED:
					link.idn_linkstate = LL_RUNNING;
					break;
				default:
					link.idn_linkstate = LL_INACTIVE;
			}