			l2cap_add_conf_opt(&ptr, L2CAP_CONF_RFC,
					   sizeof(rfc), (unsigned long) &rfc, endptr - ptr);

			if (test_bit(FLAG_EFS_ENABLE, &chan->flags)) {
				chan->remote_id = efs.id;
				chan->remote_stype = efs.stype;
				chan->remote_msdu = le16_to_cpu(efs.msdu);
				chan->remote_flush_to =