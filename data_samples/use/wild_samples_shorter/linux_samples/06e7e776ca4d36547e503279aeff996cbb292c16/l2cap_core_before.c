			break;

		case L2CAP_CONF_EFS:
			remote_efs = 1;
			if (olen == sizeof(efs))
				memcpy(&efs, (void *) val, olen);
			break;

		case L2CAP_CONF_EWS:
			if (!(chan->conn->local_fixed_chan & L2CAP_FC_A2MP))
			break;

		case L2CAP_CONF_EFS:
			if (olen == sizeof(efs))
				memcpy(&efs, (void *)val, olen);

			if (chan->local_stype != L2CAP_SERV_NOTRAFIC &&
			    efs.stype != L2CAP_SERV_NOTRAFIC &&
			    efs.stype != chan->local_stype)
				return -ECONNREFUSED;

			l2cap_add_conf_opt(&ptr, L2CAP_CONF_EFS, sizeof(efs),
					   (unsigned long) &efs, endptr - ptr);
			break;

		case L2CAP_CONF_FCS:
			if (*result == L2CAP_CONF_PENDING)