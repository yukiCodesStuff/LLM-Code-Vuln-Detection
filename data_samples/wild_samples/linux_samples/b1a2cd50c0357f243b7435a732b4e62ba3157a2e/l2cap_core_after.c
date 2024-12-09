		switch (rfc.mode) {
		case L2CAP_MODE_BASIC:
			chan->fcs = L2CAP_FCS_NONE;
			set_bit(CONF_MODE_DONE, &chan->conf_state);
			break;

		case L2CAP_MODE_ERTM:
			if (!test_bit(CONF_EWS_RECV, &chan->conf_state))
				chan->remote_tx_win = rfc.txwin_size;
			else
				rfc.txwin_size = L2CAP_DEFAULT_TX_WINDOW;

			chan->remote_max_tx = rfc.max_transmit;

			size = min_t(u16, le16_to_cpu(rfc.max_pdu_size),
				     chan->conn->mtu - L2CAP_EXT_HDR_SIZE -
				     L2CAP_SDULEN_SIZE - L2CAP_FCS_SIZE);
			rfc.max_pdu_size = cpu_to_le16(size);
			chan->remote_mps = size;

			__l2cap_set_ertm_timeouts(chan, &rfc);

			set_bit(CONF_MODE_DONE, &chan->conf_state);

			l2cap_add_conf_opt(&ptr, L2CAP_CONF_RFC,
					   sizeof(rfc), (unsigned long) &rfc, endptr - ptr);

			if (remote_efs &&
			    test_bit(FLAG_EFS_ENABLE, &chan->flags)) {
				chan->remote_id = efs.id;
				chan->remote_stype = efs.stype;
				chan->remote_msdu = le16_to_cpu(efs.msdu);
				chan->remote_flush_to =
					le32_to_cpu(efs.flush_to);
				chan->remote_acc_lat =
					le32_to_cpu(efs.acc_lat);
				chan->remote_sdu_itime =
					le32_to_cpu(efs.sdu_itime);
				l2cap_add_conf_opt(&ptr, L2CAP_CONF_EFS,
						   sizeof(efs),
						   (unsigned long) &efs, endptr - ptr);
			}