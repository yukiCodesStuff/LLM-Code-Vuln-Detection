		switch (type) {
		case L2CAP_CONF_MTU:
			mtu = val;
			break;

		case L2CAP_CONF_FLUSH_TO:
			chan->flush_to = val;
			break;

		case L2CAP_CONF_QOS:
			break;

		case L2CAP_CONF_RFC:
			if (olen == sizeof(rfc))
				memcpy(&rfc, (void *) val, olen);
			break;

		case L2CAP_CONF_FCS:
			if (val == L2CAP_FCS_NONE)
				set_bit(CONF_RECV_NO_FCS, &chan->conf_state);
			break;

		case L2CAP_CONF_EFS:
			if (olen == sizeof(efs)) {
				remote_efs = 1;
				memcpy(&efs, (void *) val, olen);
			}
			if (val < L2CAP_DEFAULT_MIN_MTU) {
				*result = L2CAP_CONF_UNACCEPT;
				chan->imtu = L2CAP_DEFAULT_MIN_MTU;
			} else
				chan->imtu = val;
			l2cap_add_conf_opt(&ptr, L2CAP_CONF_MTU, 2, chan->imtu, endptr - ptr);
			break;

		case L2CAP_CONF_FLUSH_TO:
			chan->flush_to = val;
			l2cap_add_conf_opt(&ptr, L2CAP_CONF_FLUSH_TO,
					   2, chan->flush_to, endptr - ptr);
			break;

		case L2CAP_CONF_RFC:
			if (olen == sizeof(rfc))
				memcpy(&rfc, (void *)val, olen);

			if (test_bit(CONF_STATE2_DEVICE, &chan->conf_state) &&
			    rfc.mode != chan->mode)
				return -ECONNREFUSED;

			chan->fcs = 0;

			l2cap_add_conf_opt(&ptr, L2CAP_CONF_RFC,
					   sizeof(rfc), (unsigned long) &rfc, endptr - ptr);
			break;

		case L2CAP_CONF_EWS:
			chan->ack_win = min_t(u16, val, chan->ack_win);
			l2cap_add_conf_opt(&ptr, L2CAP_CONF_EWS, 2,
					   chan->tx_win, endptr - ptr);
			break;

		case L2CAP_CONF_EFS:
			if (olen == sizeof(efs)) {
				memcpy(&efs, (void *)val, olen);

				if (chan->local_stype != L2CAP_SERV_NOTRAFIC &&
				    efs.stype != L2CAP_SERV_NOTRAFIC &&
				    efs.stype != chan->local_stype)
					return -ECONNREFUSED;

				l2cap_add_conf_opt(&ptr, L2CAP_CONF_EFS, sizeof(efs),
						   (unsigned long) &efs, endptr - ptr);
			}