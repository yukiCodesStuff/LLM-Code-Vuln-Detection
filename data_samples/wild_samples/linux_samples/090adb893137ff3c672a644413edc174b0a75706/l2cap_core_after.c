	switch (rfc.mode) {
	case L2CAP_MODE_ERTM:
		chan->retrans_timeout = le16_to_cpu(rfc.retrans_timeout);
		chan->monitor_timeout = le16_to_cpu(rfc.monitor_timeout);
		chan->mps = le16_to_cpu(rfc.max_pdu_size);
		if (test_bit(FLAG_EXT_CTRL, &chan->flags))
			chan->ack_win = min_t(u16, chan->ack_win, txwin_ext);
		else
			chan->ack_win = min_t(u16, chan->ack_win,
					      rfc.txwin_size);
		break;
	case L2CAP_MODE_STREAMING:
		chan->mps    = le16_to_cpu(rfc.max_pdu_size);
	}
}

static inline int l2cap_command_rej(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				    u8 *data)
{
	struct l2cap_cmd_rej_unk *rej = (struct l2cap_cmd_rej_unk *) data;

	if (cmd_len < sizeof(*rej))
		return -EPROTO;

	if (rej->reason != L2CAP_REJ_NOT_UNDERSTOOD)
		return 0;

	if ((conn->info_state & L2CAP_INFO_FEAT_MASK_REQ_SENT) &&
	    cmd->ident == conn->info_ident) {
		cancel_delayed_work(&conn->info_timer);

		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);
	}

	return 0;
}
	    result == L2CAP_CR_SUCCESS) {
		u8 buf[128];
		set_bit(CONF_REQ_SENT, &chan->conf_state);
		l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_CONF_REQ,
			       l2cap_build_conf_req(chan, buf), buf);
		chan->num_conf_req++;
	}

	return chan;
}

static int l2cap_connect_req(struct l2cap_conn *conn,
			     struct l2cap_cmd_hdr *cmd, u16 cmd_len, u8 *data)
{
	struct hci_dev *hdev = conn->hcon->hdev;
	struct hci_conn *hcon = conn->hcon;

	if (cmd_len < sizeof(struct l2cap_conn_req))
		return -EPROTO;

	hci_dev_lock(hdev);
	if (test_bit(HCI_MGMT, &hdev->dev_flags) &&
	    !test_and_set_bit(HCI_CONN_MGMT_CONNECTED, &hcon->flags))
		mgmt_device_connected(hdev, &hcon->dst, hcon->type,
				      hcon->dst_type, 0, NULL, 0,
				      hcon->dev_class);
	hci_dev_unlock(hdev);

	l2cap_connect(conn, cmd, data, L2CAP_CONN_RSP, 0);
	return 0;
}
{
	struct hci_dev *hdev = conn->hcon->hdev;
	struct hci_conn *hcon = conn->hcon;

	if (cmd_len < sizeof(struct l2cap_conn_req))
		return -EPROTO;

	hci_dev_lock(hdev);
	if (test_bit(HCI_MGMT, &hdev->dev_flags) &&
	    !test_and_set_bit(HCI_CONN_MGMT_CONNECTED, &hcon->flags))
		mgmt_device_connected(hdev, &hcon->dst, hcon->type,
				      hcon->dst_type, 0, NULL, 0,
				      hcon->dev_class);
	hci_dev_unlock(hdev);

	l2cap_connect(conn, cmd, data, L2CAP_CONN_RSP, 0);
	return 0;
}

static int l2cap_connect_create_rsp(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				    u8 *data)
{
	struct l2cap_conn_rsp *rsp = (struct l2cap_conn_rsp *) data;
	u16 scid, dcid, result, status;
	struct l2cap_chan *chan;
	u8 req[128];
	int err;

	if (cmd_len < sizeof(*rsp))
		return -EPROTO;

	scid   = __le16_to_cpu(rsp->scid);
	dcid   = __le16_to_cpu(rsp->dcid);
	result = __le16_to_cpu(rsp->result);
	status = __le16_to_cpu(rsp->status);

	BT_DBG("dcid 0x%4.4x scid 0x%4.4x result 0x%2.2x status 0x%2.2x",
	       dcid, scid, result, status);

	mutex_lock(&conn->chan_lock);

	if (scid) {
		chan = __l2cap_get_chan_by_scid(conn, scid);
		if (!chan) {
			err = -EFAULT;
			goto unlock;
		}
	} else {
		chan = __l2cap_get_chan_by_ident(conn, cmd->ident);
		if (!chan) {
			err = -EFAULT;
			goto unlock;
		}
	}

	err = 0;

	l2cap_chan_lock(chan);

	switch (result) {
	case L2CAP_CR_SUCCESS:
		l2cap_state_change(chan, BT_CONFIG);
		chan->ident = 0;
		chan->dcid = dcid;
		clear_bit(CONF_CONNECT_PEND, &chan->conf_state);

		if (test_and_set_bit(CONF_REQ_SENT, &chan->conf_state))
			break;

		l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_CONF_REQ,
			       l2cap_build_conf_req(chan, req), req);
		chan->num_conf_req++;
		break;

	case L2CAP_CR_PEND:
		set_bit(CONF_CONNECT_PEND, &chan->conf_state);
		break;

	default:
		l2cap_chan_del(chan, ECONNREFUSED);
		break;
	}

	l2cap_chan_unlock(chan);

unlock:
	mutex_unlock(&conn->chan_lock);

	return err;
}

static inline void set_default_fcs(struct l2cap_chan *chan)
{
	/* FCS is enabled only in ERTM or streaming mode, if one or both
	 * sides request it.
	 */
	if (chan->mode != L2CAP_MODE_ERTM && chan->mode != L2CAP_MODE_STREAMING)
		chan->fcs = L2CAP_FCS_NONE;
	else if (!test_bit(CONF_RECV_NO_FCS, &chan->conf_state))
		chan->fcs = L2CAP_FCS_CRC16;
}

static void l2cap_send_efs_conf_rsp(struct l2cap_chan *chan, void *data,
				    u8 ident, u16 flags)
{
	struct l2cap_conn *conn = chan->conn;

	BT_DBG("conn %p chan %p ident %d flags 0x%4.4x", conn, chan, ident,
	       flags);

	clear_bit(CONF_LOC_CONF_PEND, &chan->conf_state);
	set_bit(CONF_OUTPUT_DONE, &chan->conf_state);

	l2cap_send_cmd(conn, ident, L2CAP_CONF_RSP,
		       l2cap_build_conf_rsp(chan, data,
					    L2CAP_CONF_SUCCESS, flags), data);
}

static inline int l2cap_config_req(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				   u8 *data)
{
	struct l2cap_conf_req *req = (struct l2cap_conf_req *) data;
	u16 dcid, flags;
	u8 rsp[64];
	struct l2cap_chan *chan;
	int len, err = 0;

	if (cmd_len < sizeof(*req))
		return -EPROTO;

	dcid  = __le16_to_cpu(req->dcid);
	flags = __le16_to_cpu(req->flags);

	BT_DBG("dcid 0x%4.4x flags 0x%2.2x", dcid, flags);

	chan = l2cap_get_chan_by_scid(conn, dcid);
	if (!chan)
		return -ENOENT;

	if (chan->state != BT_CONFIG && chan->state != BT_CONNECT2) {
		struct l2cap_cmd_rej_cid rej;

		rej.reason = __constant_cpu_to_le16(L2CAP_REJ_INVALID_CID);
		rej.scid = cpu_to_le16(chan->scid);
		rej.dcid = cpu_to_le16(chan->dcid);

		l2cap_send_cmd(conn, cmd->ident, L2CAP_COMMAND_REJ,
			       sizeof(rej), &rej);
		goto unlock;
	}

	/* Reject if config buffer is too small. */
	len = cmd_len - sizeof(*req);
	if (chan->conf_len + len > sizeof(chan->conf_req)) {
		l2cap_send_cmd(conn, cmd->ident, L2CAP_CONF_RSP,
			       l2cap_build_conf_rsp(chan, rsp,
			       L2CAP_CONF_REJECT, flags), rsp);
		goto unlock;
	}

	/* Store config. */
	memcpy(chan->conf_req + chan->conf_len, req->data, len);
	chan->conf_len += len;

	if (flags & L2CAP_CONF_FLAG_CONTINUATION) {
		/* Incomplete config. Send empty response. */
		l2cap_send_cmd(conn, cmd->ident, L2CAP_CONF_RSP,
			       l2cap_build_conf_rsp(chan, rsp,
			       L2CAP_CONF_SUCCESS, flags), rsp);
		goto unlock;
	}

	/* Complete config. */
	len = l2cap_parse_conf_req(chan, rsp);
	if (len < 0) {
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto unlock;
	}

	chan->ident = cmd->ident;
	l2cap_send_cmd(conn, cmd->ident, L2CAP_CONF_RSP, len, rsp);
	chan->num_conf_rsp++;

	/* Reset config buffer. */
	chan->conf_len = 0;

	if (!test_bit(CONF_OUTPUT_DONE, &chan->conf_state))
		goto unlock;

	if (test_bit(CONF_INPUT_DONE, &chan->conf_state)) {
		set_default_fcs(chan);

		if (chan->mode == L2CAP_MODE_ERTM ||
		    chan->mode == L2CAP_MODE_STREAMING)
			err = l2cap_ertm_init(chan);

		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);

		goto unlock;
	}

	if (!test_and_set_bit(CONF_REQ_SENT, &chan->conf_state)) {
		u8 buf[64];
		l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_CONF_REQ,
			       l2cap_build_conf_req(chan, buf), buf);
		chan->num_conf_req++;
	}

	/* Got Conf Rsp PENDING from remote side and asume we sent
	   Conf Rsp PENDING in the code above */
	if (test_bit(CONF_REM_CONF_PEND, &chan->conf_state) &&
	    test_bit(CONF_LOC_CONF_PEND, &chan->conf_state)) {

		/* check compatibility */

		/* Send rsp for BR/EDR channel */
		if (!chan->hs_hcon)
			l2cap_send_efs_conf_rsp(chan, rsp, cmd->ident, flags);
		else
			chan->ident = cmd->ident;
	}

unlock:
	l2cap_chan_unlock(chan);
	return err;
}

static inline int l2cap_config_rsp(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				   u8 *data)
{
	struct l2cap_conf_rsp *rsp = (struct l2cap_conf_rsp *)data;
	u16 scid, flags, result;
	struct l2cap_chan *chan;
	int len = cmd_len - sizeof(*rsp);
	int err = 0;

	if (cmd_len < sizeof(*rsp))
		return -EPROTO;

	scid   = __le16_to_cpu(rsp->scid);
	flags  = __le16_to_cpu(rsp->flags);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("scid 0x%4.4x flags 0x%2.2x result 0x%2.2x len %d", scid, flags,
	       result, len);

	chan = l2cap_get_chan_by_scid(conn, scid);
	if (!chan)
		return 0;

	switch (result) {
	case L2CAP_CONF_SUCCESS:
		l2cap_conf_rfc_get(chan, rsp->data, len);
		clear_bit(CONF_REM_CONF_PEND, &chan->conf_state);
		break;

	case L2CAP_CONF_PENDING:
		set_bit(CONF_REM_CONF_PEND, &chan->conf_state);

		if (test_bit(CONF_LOC_CONF_PEND, &chan->conf_state)) {
			char buf[64];

			len = l2cap_parse_conf_rsp(chan, rsp->data, len,
						   buf, &result);
			if (len < 0) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			if (!chan->hs_hcon) {
				l2cap_send_efs_conf_rsp(chan, buf, cmd->ident,
							0);
			} else {
				if (l2cap_check_efs(chan)) {
					amp_create_logical_link(chan);
					chan->ident = cmd->ident;
				}
			}
		}
		goto done;

	case L2CAP_CONF_UNACCEPT:
		if (chan->num_conf_rsp <= L2CAP_CONF_MAX_CONF_RSP) {
			char req[64];

			if (len > sizeof(req) - sizeof(struct l2cap_conf_req)) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			/* throw out any old stored conf requests */
			result = L2CAP_CONF_SUCCESS;
			len = l2cap_parse_conf_rsp(chan, rsp->data, len,
						   req, &result);
			if (len < 0) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			l2cap_send_cmd(conn, l2cap_get_ident(conn),
				       L2CAP_CONF_REQ, len, req);
			chan->num_conf_req++;
			if (result != L2CAP_CONF_SUCCESS)
				goto done;
			break;
		}

	default:
		l2cap_chan_set_err(chan, ECONNRESET);

		__set_chan_timer(chan, L2CAP_DISC_REJ_TIMEOUT);
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto done;
	}

	if (flags & L2CAP_CONF_FLAG_CONTINUATION)
		goto done;

	set_bit(CONF_INPUT_DONE, &chan->conf_state);

	if (test_bit(CONF_OUTPUT_DONE, &chan->conf_state)) {
		set_default_fcs(chan);

		if (chan->mode == L2CAP_MODE_ERTM ||
		    chan->mode == L2CAP_MODE_STREAMING)
			err = l2cap_ertm_init(chan);

		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}

done:
	l2cap_chan_unlock(chan);
	return err;
}

static inline int l2cap_disconnect_req(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_req *req = (struct l2cap_disconn_req *) data;
	struct l2cap_disconn_rsp rsp;
	u16 dcid, scid;
	struct l2cap_chan *chan;
	struct sock *sk;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	scid = __le16_to_cpu(req->scid);
	dcid = __le16_to_cpu(req->dcid);

	BT_DBG("scid 0x%4.4x dcid 0x%4.4x", scid, dcid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, dcid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	sk = chan->sk;

	rsp.dcid = cpu_to_le16(chan->scid);
	rsp.scid = cpu_to_le16(chan->dcid);
	l2cap_send_cmd(conn, cmd->ident, L2CAP_DISCONN_RSP, sizeof(rsp), &rsp);

	lock_sock(sk);
	sk->sk_shutdown = SHUTDOWN_MASK;
	release_sock(sk);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, ECONNRESET);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_disconnect_rsp(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_rsp *rsp = (struct l2cap_disconn_rsp *) data;
	u16 dcid, scid;
	struct l2cap_chan *chan;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	scid = __le16_to_cpu(rsp->scid);
	dcid = __le16_to_cpu(rsp->dcid);

	BT_DBG("dcid 0x%4.4x scid 0x%4.4x", dcid, scid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, scid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, 0);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_information_req(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_req *req = (struct l2cap_info_req *) data;
	u16 type;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	type = __le16_to_cpu(req->type);

	BT_DBG("type 0x%4.4x", type);

	if (type == L2CAP_IT_FEAT_MASK) {
		u8 buf[8];
		u32 feat_mask = l2cap_feat_mask;
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;
		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FEAT_MASK);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		if (!disable_ertm)
			feat_mask |= L2CAP_FEAT_ERTM | L2CAP_FEAT_STREAMING
				| L2CAP_FEAT_FCS;
		if (enable_hs)
			feat_mask |= L2CAP_FEAT_EXT_FLOW
				| L2CAP_FEAT_EXT_WINDOW;

		put_unaligned_le32(feat_mask, rsp->data);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else if (type == L2CAP_IT_FIXED_CHAN) {
		u8 buf[12];
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;

		if (enable_hs)
			l2cap_fixed_chan[0] |= L2CAP_FC_A2MP;
		else
			l2cap_fixed_chan[0] &= ~L2CAP_FC_A2MP;

		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		memcpy(rsp->data, l2cap_fixed_chan, sizeof(l2cap_fixed_chan));
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else {
		struct l2cap_info_rsp rsp;
		rsp.type   = cpu_to_le16(type);
		rsp.result = __constant_cpu_to_le16(L2CAP_IR_NOTSUPP);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(rsp),
			       &rsp);
	}

	return 0;
}

static inline int l2cap_information_rsp(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) data;
	u16 type, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	type   = __le16_to_cpu(rsp->type);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("type 0x%4.4x result 0x%2.2x", type, result);

	/* L2CAP Info req/rsp are unbound to channels, add extra checks */
	if (cmd->ident != conn->info_ident ||
	    conn->info_state & L2CAP_INFO_FEAT_MASK_REQ_DONE)
		return 0;

	cancel_delayed_work(&conn->info_timer);

	if (result != L2CAP_IR_SUCCESS) {
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);

		return 0;
	}

	switch (type) {
	case L2CAP_IT_FEAT_MASK:
		conn->feat_mask = get_unaligned_le32(rsp->data);

		if (conn->feat_mask & L2CAP_FEAT_FIXED_CHAN) {
			struct l2cap_info_req req;
			req.type = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);

			conn->info_ident = l2cap_get_ident(conn);

			l2cap_send_cmd(conn, conn->info_ident,
				       L2CAP_INFO_REQ, sizeof(req), &req);
		} else {
			conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
			conn->info_ident = 0;

			l2cap_conn_start(conn);
		}
		break;

	case L2CAP_IT_FIXED_CHAN:
		conn->fixed_chan_mask = rsp->data[0];
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);
		break;
	}

	return 0;
}

static int l2cap_create_channel_req(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd,
				    u16 cmd_len, void *data)
{
	struct l2cap_create_chan_req *req = data;
	struct l2cap_create_chan_rsp rsp;
	struct l2cap_chan *chan;
	struct hci_dev *hdev;
	u16 psm, scid;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	if (!enable_hs)
		return -EINVAL;

	psm = le16_to_cpu(req->psm);
	scid = le16_to_cpu(req->scid);

	BT_DBG("psm 0x%2.2x, scid 0x%4.4x, amp_id %d", psm, scid, req->amp_id);

	/* For controller id 0 make BR/EDR connection */
	if (req->amp_id == HCI_BREDR_ID) {
		l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			      req->amp_id);
		return 0;
	}

	/* Validate AMP controller id */
	hdev = hci_dev_get(req->amp_id);
	if (!hdev)
		goto error;

	if (hdev->dev_type != HCI_AMP || !test_bit(HCI_UP, &hdev->flags)) {
		hci_dev_put(hdev);
		goto error;
	}

	chan = l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			     req->amp_id);
	if (chan) {
		struct amp_mgr *mgr = conn->hcon->amp_mgr;
		struct hci_conn *hs_hcon;

		hs_hcon = hci_conn_hash_lookup_ba(hdev, AMP_LINK, conn->dst);
		if (!hs_hcon) {
			hci_dev_put(hdev);
			return -EFAULT;
		}

		BT_DBG("mgr %p bredr_chan %p hs_hcon %p", mgr, chan, hs_hcon);

		mgr->bredr_chan = chan;
		chan->hs_hcon = hs_hcon;
		chan->fcs = L2CAP_FCS_NONE;
		conn->mtu = hdev->block_mtu;
	}

	hci_dev_put(hdev);

	return 0;

error:
	rsp.dcid = 0;
	rsp.scid = cpu_to_le16(scid);
	rsp.result = __constant_cpu_to_le16(L2CAP_CR_BAD_AMP);
	rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CREATE_CHAN_RSP,
		       sizeof(rsp), &rsp);

	return -EFAULT;
}

static void l2cap_send_move_chan_req(struct l2cap_chan *chan, u8 dest_amp_id)
{
	struct l2cap_move_chan_req req;
	u8 ident;

	BT_DBG("chan %p, dest_amp_id %d", chan, dest_amp_id);

	ident = l2cap_get_ident(chan->conn);
	chan->ident = ident;

	req.icid = cpu_to_le16(chan->scid);
	req.dest_amp_id = dest_amp_id;

	l2cap_send_cmd(chan->conn, ident, L2CAP_MOVE_CHAN_REQ, sizeof(req),
		       &req);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_rsp(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_rsp rsp;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	rsp.icid = cpu_to_le16(chan->dcid);
	rsp.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_RSP,
		       sizeof(rsp), &rsp);
}

static void l2cap_send_move_chan_cfm(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	chan->ident = l2cap_get_ident(chan->conn);

	cfm.icid = cpu_to_le16(chan->scid);
	cfm.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_cfm_icid(struct l2cap_conn *conn, u16 icid)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("conn %p, icid 0x%4.4x", conn, icid);

	cfm.icid = cpu_to_le16(icid);
	cfm.result = __constant_cpu_to_le16(L2CAP_MC_UNCONFIRMED);

	l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);
}

static void l2cap_send_move_chan_cfm_rsp(struct l2cap_conn *conn, u8 ident,
					 u16 icid)
{
	struct l2cap_move_chan_cfm_rsp rsp;

	BT_DBG("icid 0x%4.4x", icid);

	rsp.icid = cpu_to_le16(icid);
	l2cap_send_cmd(conn, ident, L2CAP_MOVE_CHAN_CFM_RSP, sizeof(rsp), &rsp);
}

static void __release_logical_link(struct l2cap_chan *chan)
{
	chan->hs_hchan = NULL;
	chan->hs_hcon = NULL;

	/* Placeholder - release the logical link */
}

static void l2cap_logical_fail(struct l2cap_chan *chan)
{
	/* Logical link setup failed */
	if (chan->state != BT_CONNECTED) {
		/* Create channel failure, disconnect */
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	switch (chan->move_role) {
	case L2CAP_MOVE_ROLE_RESPONDER:
		l2cap_move_done(chan);
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_SUPP);
		break;
	case L2CAP_MOVE_ROLE_INITIATOR:
		if (chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_COMP ||
		    chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_CFM) {
			/* Remote has only sent pending or
			 * success responses, clean up
			 */
			l2cap_move_done(chan);
		}

		/* Other amp move states imply that the move
		 * has already aborted
		 */
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
		break;
	}
}

static void l2cap_logical_finish_create(struct l2cap_chan *chan,
					struct hci_chan *hchan)
{
	struct l2cap_conf_rsp rsp;

	chan->hs_hchan = hchan;
	chan->hs_hcon->l2cap_data = chan->conn;

	l2cap_send_efs_conf_rsp(chan, &rsp, chan->ident, 0);

	if (test_bit(CONF_INPUT_DONE, &chan->conf_state)) {
		int err;

		set_default_fcs(chan);

		err = l2cap_ertm_init(chan);
		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}
}

static void l2cap_logical_finish_move(struct l2cap_chan *chan,
				      struct hci_chan *hchan)
{
	chan->hs_hcon = hchan->conn;
	chan->hs_hcon->l2cap_data = chan->conn;

	BT_DBG("move_state %d", chan->move_state);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent after a success
		 * response is received
		 */
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		break;
	case L2CAP_MOVE_WAIT_LOGICAL_CFM:
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);
		}
		break;
	default:
		/* Move was not in expected state, free the channel */
		__release_logical_link(chan);

		chan->move_state = L2CAP_MOVE_STABLE;
	}
}

/* Call with chan locked */
void l2cap_logical_cfm(struct l2cap_chan *chan, struct hci_chan *hchan,
		       u8 status)
{
	BT_DBG("chan %p, hchan %p, status %d", chan, hchan, status);

	if (status) {
		l2cap_logical_fail(chan);
		__release_logical_link(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		/* Ignore logical link if channel is on BR/EDR */
		if (chan->local_amp_id)
			l2cap_logical_finish_create(chan, hchan);
	} else {
		l2cap_logical_finish_move(chan, hchan);
	}
}

void l2cap_move_start(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	if (chan->local_amp_id == HCI_BREDR_ID) {
		if (chan->chan_policy != BT_CHANNEL_POLICY_AMP_PREFERRED)
			return;
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - start physical link setup */
	} else {
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		chan->move_id = 0;
		l2cap_move_setup(chan);
		l2cap_send_move_chan_req(chan, 0);
	}
}

static void l2cap_do_create(struct l2cap_chan *chan, int result,
			    u8 local_amp_id, u8 remote_amp_id)
{
	BT_DBG("chan %p state %s %u -> %u", chan, state_to_string(chan->state),
	       local_amp_id, remote_amp_id);

	chan->fcs = L2CAP_FCS_NONE;

	/* Outgoing channel on AMP */
	if (chan->state == BT_CONNECT) {
		if (result == L2CAP_CR_SUCCESS) {
			chan->local_amp_id = local_amp_id;
			l2cap_send_create_chan_req(chan, remote_amp_id);
		} else {
			/* Revert to BR/EDR connect */
			l2cap_send_conn_req(chan);
		}

		return;
	}

	/* Incoming channel on AMP */
	if (__l2cap_no_conn_pending(chan)) {
		struct l2cap_conn_rsp rsp;
		char buf[128];
		rsp.scid = cpu_to_le16(chan->dcid);
		rsp.dcid = cpu_to_le16(chan->scid);

		if (result == L2CAP_CR_SUCCESS) {
			/* Send successful response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_SUCCESS);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		} else {
			/* Send negative response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_NO_MEM);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		}

		l2cap_send_cmd(chan->conn, chan->ident, L2CAP_CREATE_CHAN_RSP,
			       sizeof(rsp), &rsp);

		if (result == L2CAP_CR_SUCCESS) {
			__l2cap_state_change(chan, BT_CONFIG);
			set_bit(CONF_REQ_SENT, &chan->conf_state);
			l2cap_send_cmd(chan->conn, l2cap_get_ident(chan->conn),
				       L2CAP_CONF_REQ,
				       l2cap_build_conf_req(chan, buf), buf);
			chan->num_conf_req++;
		}
	}
}

static void l2cap_do_move_initiate(struct l2cap_chan *chan, u8 local_amp_id,
				   u8 remote_amp_id)
{
	l2cap_move_setup(chan);
	chan->move_id = local_amp_id;
	chan->move_state = L2CAP_MOVE_WAIT_RSP;

	l2cap_send_move_chan_req(chan, remote_amp_id);
}

static void l2cap_do_move_respond(struct l2cap_chan *chan, int result)
{
	struct hci_chan *hchan = NULL;

	/* Placeholder - get hci_chan for logical link */

	if (hchan) {
		if (hchan->state == BT_CONNECTED) {
			/* Logical link is ready to go */
			chan->hs_hcon = hchan->conn;
			chan->hs_hcon->l2cap_data = chan->conn;
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);

			l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		} else {
			/* Wait for logical link to be ready */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		}
	} else {
		/* Logical link not available */
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_ALLOWED);
	}
}

static void l2cap_do_move_cancel(struct l2cap_chan *chan, int result)
{
	if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
		u8 rsp_result;
		if (result == -EINVAL)
			rsp_result = L2CAP_MR_BAD_ID;
		else
			rsp_result = L2CAP_MR_NOT_ALLOWED;

		l2cap_send_move_chan_rsp(chan, rsp_result);
	}

	chan->move_role = L2CAP_MOVE_ROLE_NONE;
	chan->move_state = L2CAP_MOVE_STABLE;

	/* Restart data transmission */
	l2cap_ertm_send(chan);
}

/* Invoke with locked chan */
void __l2cap_physical_cfm(struct l2cap_chan *chan, int result)
{
	u8 local_amp_id = chan->local_amp_id;
	u8 remote_amp_id = chan->remote_amp_id;

	BT_DBG("chan %p, result %d, local_amp_id %d, remote_amp_id %d",
	       chan, result, local_amp_id, remote_amp_id);

	if (chan->state == BT_DISCONN || chan->state == BT_CLOSED) {
		l2cap_chan_unlock(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		l2cap_do_create(chan, result, local_amp_id, remote_amp_id);
	} else if (result != L2CAP_MR_SUCCESS) {
		l2cap_do_move_cancel(chan, result);
	} else {
		switch (chan->move_role) {
		case L2CAP_MOVE_ROLE_INITIATOR:
			l2cap_do_move_initiate(chan, local_amp_id,
					       remote_amp_id);
			break;
		case L2CAP_MOVE_ROLE_RESPONDER:
			l2cap_do_move_respond(chan, result);
			break;
		default:
			l2cap_do_move_cancel(chan, result);
			break;
		}
	}
}

static inline int l2cap_move_channel_req(struct l2cap_conn *conn,
					 struct l2cap_cmd_hdr *cmd,
					 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_req *req = data;
	struct l2cap_move_chan_rsp rsp;
	struct l2cap_chan *chan;
	u16 icid = 0;
	u16 result = L2CAP_MR_NOT_ALLOWED;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	icid = le16_to_cpu(req->icid);

	BT_DBG("icid 0x%4.4x, dest_amp_id %d", icid, req->dest_amp_id);

	if (!enable_hs)
		return -EINVAL;

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		rsp.icid = cpu_to_le16(icid);
		rsp.result = __constant_cpu_to_le16(L2CAP_MR_NOT_ALLOWED);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_MOVE_CHAN_RSP,
			       sizeof(rsp), &rsp);
		return 0;
	}

	chan->ident = cmd->ident;

	if (chan->scid < L2CAP_CID_DYN_START ||
	    chan->chan_policy == BT_CHANNEL_POLICY_BREDR_ONLY ||
	    (chan->mode != L2CAP_MODE_ERTM &&
	     chan->mode != L2CAP_MODE_STREAMING)) {
		result = L2CAP_MR_NOT_ALLOWED;
		goto send_move_response;
	}

	if (chan->local_amp_id == req->dest_amp_id) {
		result = L2CAP_MR_SAME_ID;
		goto send_move_response;
	}

	if (req->dest_amp_id) {
		struct hci_dev *hdev;
		hdev = hci_dev_get(req->dest_amp_id);
		if (!hdev || hdev->dev_type != HCI_AMP ||
		    !test_bit(HCI_UP, &hdev->flags)) {
			if (hdev)
				hci_dev_put(hdev);

			result = L2CAP_MR_BAD_ID;
			goto send_move_response;
		}
		hci_dev_put(hdev);
	}

	/* Detect a move collision.  Only send a collision response
	 * if this side has "lost", otherwise proceed with the move.
	 * The winner has the larger bd_addr.
	 */
	if ((__chan_is_moving(chan) ||
	     chan->move_role != L2CAP_MOVE_ROLE_NONE) &&
	    bacmp(conn->src, conn->dst) > 0) {
		result = L2CAP_MR_COLLISION;
		goto send_move_response;
	}

	chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
	l2cap_move_setup(chan);
	chan->move_id = req->dest_amp_id;
	icid = chan->dcid;

	if (!req->dest_amp_id) {
		/* Moving to BR/EDR */
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
			result = L2CAP_MR_PEND;
		} else {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			result = L2CAP_MR_SUCCESS;
		}
	} else {
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - uncomment when amp functions are available */
		/*amp_accept_physical(chan, req->dest_amp_id);*/
		result = L2CAP_MR_PEND;
	}

send_move_response:
	l2cap_send_move_chan_rsp(chan, result);

	l2cap_chan_unlock(chan);

	return 0;
}

static void l2cap_move_continue(struct l2cap_conn *conn, u16 icid, u16 result)
{
	struct l2cap_chan *chan;
	struct hci_chan *hchan = NULL;

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan) {
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);
	if (result == L2CAP_MR_PEND)
		__set_chan_timer(chan, L2CAP_MOVE_ERTX_TIMEOUT);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent when logical link
		 * is complete.
		 */
		chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		break;
	case L2CAP_MOVE_WAIT_RSP_SUCCESS:
		if (result == L2CAP_MR_PEND) {
			break;
		} else if (test_bit(CONN_LOCAL_BUSY,
				    &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else {
			/* Logical link is up or moving to BR/EDR,
			 * proceed with move
			 */
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		}
		break;
	case L2CAP_MOVE_WAIT_RSP:
		/* Moving to AMP */
		if (result == L2CAP_MR_SUCCESS) {
			/* Remote is ready, send confirm immediately
			 * after logical link is ready
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		} else {
			/* Both logical link and move success
			 * are required to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_COMP;
		}

		/* Placeholder - get hci_chan for logical link */
		if (!hchan) {
			/* Logical link not available */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
			break;
		}

		/* If the logical link is not yet connected, do not
		 * send confirmation.
		 */
		if (hchan->state != BT_CONNECTED)
			break;

		/* Logical link is already ready to go */

		chan->hs_hcon = hchan->conn;
		chan->hs_hcon->l2cap_data = chan->conn;

		if (result == L2CAP_MR_SUCCESS) {
			/* Can confirm now */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else {
			/* Now only need move success
			 * to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		}

		l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		break;
	default:
		/* Any other amp move state means the move failed. */
		chan->move_id = chan->local_amp_id;
		l2cap_move_done(chan);
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
	}

	l2cap_chan_unlock(chan);
}

static void l2cap_move_fail(struct l2cap_conn *conn, u8 ident, u16 icid,
			    u16 result)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_ident(conn, ident);
	if (!chan) {
		/* Could not locate channel, icid is best guess */
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);

	if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
		if (result == L2CAP_MR_COLLISION) {
			chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
		} else {
			/* Cleanup - cancel move */
			chan->move_id = chan->local_amp_id;
			l2cap_move_done(chan);
		}
	}

	l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);

	l2cap_chan_unlock(chan);
}

static int l2cap_move_channel_rsp(struct l2cap_conn *conn,
				  struct l2cap_cmd_hdr *cmd,
				  u16 cmd_len, void *data)
{
	struct l2cap_move_chan_rsp *rsp = data;
	u16 icid, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);
	result = le16_to_cpu(rsp->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	if (result == L2CAP_MR_SUCCESS || result == L2CAP_MR_PEND)
		l2cap_move_continue(conn, icid, result);
	else
		l2cap_move_fail(conn, cmd->ident, icid, result);

	return 0;
}

static int l2cap_move_channel_confirm(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd,
				      u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm *cfm = data;
	struct l2cap_chan *chan;
	u16 icid, result;

	if (cmd_len != sizeof(*cfm))
		return -EPROTO;

	icid = le16_to_cpu(cfm->icid);
	result = le16_to_cpu(cfm->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		/* Spec requires a response even if the icid was not found */
		l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);
		return 0;
	}

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM) {
		if (result == L2CAP_MC_CONFIRMED) {
			chan->local_amp_id = chan->move_id;
			if (!chan->local_amp_id)
				__release_logical_link(chan);
		} else {
			chan->move_id = chan->local_amp_id;
		}

		l2cap_move_done(chan);
	}

	l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_move_channel_confirm_rsp(struct l2cap_conn *conn,
						 struct l2cap_cmd_hdr *cmd,
						 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm_rsp *rsp = data;
	struct l2cap_chan *chan;
	u16 icid;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);

	BT_DBG("icid 0x%4.4x", icid);

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan)
		return 0;

	__clear_chan_timer(chan);

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM_RSP) {
		chan->local_amp_id = chan->move_id;

		if (!chan->local_amp_id && chan->hs_hchan)
			__release_logical_link(chan);

		l2cap_move_done(chan);
	}

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_check_conn_param(u16 min, u16 max, u16 latency,
					 u16 to_multiplier)
{
	u16 max_latency;

	if (min > max || min < 6 || max > 3200)
		return -EINVAL;

	if (to_multiplier < 10 || to_multiplier > 3200)
		return -EINVAL;

	if (max >= to_multiplier * 8)
		return -EINVAL;

	max_latency = (to_multiplier * 8 / max) - 1;
	if (latency > 499 || latency > max_latency)
		return -EINVAL;

	return 0;
}

static inline int l2cap_conn_param_update_req(struct l2cap_conn *conn,
					      struct l2cap_cmd_hdr *cmd,
					      u8 *data)
{
	struct hci_conn *hcon = conn->hcon;
	struct l2cap_conn_param_update_req *req;
	struct l2cap_conn_param_update_rsp rsp;
	u16 min, max, latency, to_multiplier, cmd_len;
	int err;

	if (!(hcon->link_mode & HCI_LM_MASTER))
		return -EINVAL;

	cmd_len = __le16_to_cpu(cmd->len);
	if (cmd_len != sizeof(struct l2cap_conn_param_update_req))
		return -EPROTO;

	req = (struct l2cap_conn_param_update_req *) data;
	min		= __le16_to_cpu(req->min);
	max		= __le16_to_cpu(req->max);
	latency		= __le16_to_cpu(req->latency);
	to_multiplier	= __le16_to_cpu(req->to_multiplier);

	BT_DBG("min 0x%4.4x max 0x%4.4x latency: 0x%4.4x Timeout: 0x%4.4x",
	       min, max, latency, to_multiplier);

	memset(&rsp, 0, sizeof(rsp));

	err = l2cap_check_conn_param(min, max, latency, to_multiplier);
	if (err)
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_REJECTED);
	else
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_ACCEPTED);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CONN_PARAM_UPDATE_RSP,
		       sizeof(rsp), &rsp);

	if (!err)
		hci_le_conn_update(hcon, min, max, latency, to_multiplier);

	return 0;
}

static inline int l2cap_bredr_sig_cmd(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				      u8 *data)
{
	int err = 0;

	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}

	return err;
}

static inline int l2cap_le_sig_cmd(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u8 *data)
{
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		return 0;

	case L2CAP_CONN_PARAM_UPDATE_REQ:
		return l2cap_conn_param_update_req(conn, cmd, data);

	case L2CAP_CONN_PARAM_UPDATE_RSP:
		return 0;

	default:
		BT_ERR("Unknown LE signaling command 0x%2.2x", cmd->code);
		return -EINVAL;
	}
}

static inline void l2cap_sig_channel(struct l2cap_conn *conn,
				     struct sk_buff *skb)
{
	u8 *data = skb->data;
	int len = skb->len;
	struct l2cap_cmd_hdr cmd;
	int err;

	l2cap_raw_recv(conn, skb);

	while (len >= L2CAP_CMD_HDR_SIZE) {
		u16 cmd_len;
		memcpy(&cmd, data, L2CAP_CMD_HDR_SIZE);
		data += L2CAP_CMD_HDR_SIZE;
		len  -= L2CAP_CMD_HDR_SIZE;

		cmd_len = le16_to_cpu(cmd.len);

		BT_DBG("code 0x%2.2x len %d id 0x%2.2x", cmd.code, cmd_len,
		       cmd.ident);

		if (cmd_len > len || !cmd.ident) {
			BT_DBG("corrupted command");
			break;
		}

		if (conn->hcon->type == LE_LINK)
			err = l2cap_le_sig_cmd(conn, &cmd, data);
		else
			err = l2cap_bredr_sig_cmd(conn, &cmd, cmd_len, data);

		if (err) {
			struct l2cap_cmd_rej_unk rej;

			BT_ERR("Wrong link type (%d)", err);

			/* FIXME: Map err to a valid reason */
			rej.reason = __constant_cpu_to_le16(L2CAP_REJ_NOT_UNDERSTOOD);
			l2cap_send_cmd(conn, cmd.ident, L2CAP_COMMAND_REJ,
				       sizeof(rej), &rej);
		}

		data += cmd_len;
		len  -= cmd_len;
	}

	kfree_skb(skb);
}

static int l2cap_check_fcs(struct l2cap_chan *chan,  struct sk_buff *skb)
{
	u16 our_fcs, rcv_fcs;
	int hdr_size;

	if (test_bit(FLAG_EXT_CTRL, &chan->flags))
		hdr_size = L2CAP_EXT_HDR_SIZE;
	else
		hdr_size = L2CAP_ENH_HDR_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16) {
		skb_trim(skb, skb->len - L2CAP_FCS_SIZE);
		rcv_fcs = get_unaligned_le16(skb->data + skb->len);
		our_fcs = crc16(0, skb->data - hdr_size, skb->len + hdr_size);

		if (our_fcs != rcv_fcs)
			return -EBADMSG;
	}
	return 0;
}

static void l2cap_send_i_or_rr_or_rnr(struct l2cap_chan *chan)
{
	struct l2cap_ctrl control;

	BT_DBG("chan %p", chan);

	memset(&control, 0, sizeof(control));
	control.sframe = 1;
	control.final = 1;
	control.reqseq = chan->buffer_seq;
	set_bit(CONN_SEND_FBIT, &chan->conn_state);

	if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		control.super = L2CAP_SUPER_RNR;
		l2cap_send_sframe(chan, &control);
	}

	if (test_and_clear_bit(CONN_REMOTE_BUSY, &chan->conn_state) &&
	    chan->unacked_frames > 0)
		__set_retrans_timer(chan);

	/* Send pending iframes */
	l2cap_ertm_send(chan);

	if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state) &&
	    test_bit(CONN_SEND_FBIT, &chan->conn_state)) {
		/* F-bit wasn't sent in an s-frame or i-frame yet, so
		 * send it now.
		 */
		control.super = L2CAP_SUPER_RR;
		l2cap_send_sframe(chan, &control);
	}
}

static void append_skb_frag(struct sk_buff *skb, struct sk_buff *new_frag,
			    struct sk_buff **last_frag)
{
	/* skb->len reflects data in skb as well as all fragments
	 * skb->data_len reflects only data in fragments
	 */
	if (!skb_has_frag_list(skb))
		skb_shinfo(skb)->frag_list = new_frag;

	new_frag->next = NULL;

	(*last_frag)->next = new_frag;
	*last_frag = new_frag;

	skb->len += new_frag->len;
	skb->data_len += new_frag->len;
	skb->truesize += new_frag->truesize;
}

static int l2cap_reassemble_sdu(struct l2cap_chan *chan, struct sk_buff *skb,
				struct l2cap_ctrl *control)
{
	int err = -EINVAL;

	switch (control->sar) {
	case L2CAP_SAR_UNSEGMENTED:
		if (chan->sdu)
			break;

		err = chan->ops->recv(chan, skb);
		break;

	case L2CAP_SAR_START:
		if (chan->sdu)
			break;

		chan->sdu_len = get_unaligned_le16(skb->data);
		skb_pull(skb, L2CAP_SDULEN_SIZE);

		if (chan->sdu_len > chan->imtu) {
			err = -EMSGSIZE;
			break;
		}

		if (skb->len >= chan->sdu_len)
			break;

		chan->sdu = skb;
		chan->sdu_last_frag = skb;

		skb = NULL;
		err = 0;
		break;

	case L2CAP_SAR_CONTINUE:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len >= chan->sdu_len)
			break;

		err = 0;
		break;

	case L2CAP_SAR_END:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len != chan->sdu_len)
			break;

		err = chan->ops->recv(chan, chan->sdu);

		if (!err) {
			/* Reassembly complete */
			chan->sdu = NULL;
			chan->sdu_last_frag = NULL;
			chan->sdu_len = 0;
		}
		break;
	}

	if (err) {
		kfree_skb(skb);
		kfree_skb(chan->sdu);
		chan->sdu = NULL;
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;
	}

	return err;
}

static int l2cap_resegment(struct l2cap_chan *chan)
{
	/* Placeholder */
	return 0;
}

void l2cap_chan_busy(struct l2cap_chan *chan, int busy)
{
	u8 event;

	if (chan->mode != L2CAP_MODE_ERTM)
		return;

	event = busy ? L2CAP_EV_LOCAL_BUSY_DETECTED : L2CAP_EV_LOCAL_BUSY_CLEAR;
	l2cap_tx(chan, NULL, NULL, event);
}

static int l2cap_rx_queued_iframes(struct l2cap_chan *chan)
{
	int err = 0;
	/* Pass sequential frames to l2cap_reassemble_sdu()
	 * until a gap is encountered.
	 */

	BT_DBG("chan %p", chan);

	while (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		struct sk_buff *skb;
		BT_DBG("Searching for skb with txseq %d (queue len %d)",
		       chan->buffer_seq, skb_queue_len(&chan->srej_q));

		skb = l2cap_ertm_seq_in_queue(&chan->srej_q, chan->buffer_seq);

		if (!skb)
			break;

		skb_unlink(skb, &chan->srej_q);
		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);
		err = l2cap_reassemble_sdu(chan, skb, &bt_cb(skb)->control);
		if (err)
			break;
	}

	if (skb_queue_empty(&chan->srej_q)) {
		chan->rx_state = L2CAP_RX_STATE_RECV;
		l2cap_send_ack(chan);
	}

	return err;
}

static void l2cap_handle_srej(struct l2cap_chan *chan,
			      struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (skb == NULL) {
		BT_DBG("Seq %d not available for retransmission",
		       control->reqseq);
		return;
	}

	if (chan->max_tx != 0 && bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	if (control->poll) {
		l2cap_pass_to_tx(chan, control);

		set_bit(CONN_SEND_FBIT, &chan->conn_state);
		l2cap_retransmit(chan, control);
		l2cap_ertm_send(chan);

		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
			set_bit(CONN_SREJ_ACT, &chan->conn_state);
			chan->srej_save_reqseq = control->reqseq;
		}
	} else {
		l2cap_pass_to_tx_fbit(chan, control);

		if (control->final) {
			if (chan->srej_save_reqseq != control->reqseq ||
			    !test_and_clear_bit(CONN_SREJ_ACT,
						&chan->conn_state))
				l2cap_retransmit(chan, control);
		} else {
			l2cap_retransmit(chan, control);
			if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
				set_bit(CONN_SREJ_ACT, &chan->conn_state);
				chan->srej_save_reqseq = control->reqseq;
			}
		}
	}
}

static void l2cap_handle_rej(struct l2cap_chan *chan,
			     struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (chan->max_tx && skb &&
	    bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	l2cap_pass_to_tx(chan, control);

	if (control->final) {
		if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state))
			l2cap_retransmit_all(chan, control);
	} else {
		l2cap_retransmit_all(chan, control);
		l2cap_ertm_send(chan);
		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F)
			set_bit(CONN_REJ_ACT, &chan->conn_state);
	}
}

static u8 l2cap_classify_txseq(struct l2cap_chan *chan, u16 txseq)
{
	BT_DBG("chan %p, txseq %d", chan, txseq);

	BT_DBG("last_acked_seq %d, expected_tx_seq %d", chan->last_acked_seq,
	       chan->expected_tx_seq);

	if (chan->rx_state == L2CAP_RX_STATE_SREJ_SENT) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			/* See notes below regarding "double poll" and
			 * invalid packets.
			 */
			if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
				BT_DBG("Invalid/Ignore - after SREJ");
				return L2CAP_TXSEQ_INVALID_IGNORE;
			} else {
				BT_DBG("Invalid - in window after SREJ sent");
				return L2CAP_TXSEQ_INVALID;
			}
		}

		if (chan->srej_list.head == txseq) {
			BT_DBG("Expected SREJ");
			return L2CAP_TXSEQ_EXPECTED_SREJ;
		}

		if (l2cap_ertm_seq_in_queue(&chan->srej_q, txseq)) {
			BT_DBG("Duplicate SREJ - txseq already stored");
			return L2CAP_TXSEQ_DUPLICATE_SREJ;
		}

		if (l2cap_seq_list_contains(&chan->srej_list, txseq)) {
			BT_DBG("Unexpected SREJ - not requested");
			return L2CAP_TXSEQ_UNEXPECTED_SREJ;
		}
	}

	if (chan->expected_tx_seq == txseq) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		} else {
			BT_DBG("Expected");
			return L2CAP_TXSEQ_EXPECTED;
		}
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) <
	    __seq_offset(chan, chan->expected_tx_seq, chan->last_acked_seq)) {
		BT_DBG("Duplicate - expected_tx_seq later than txseq");
		return L2CAP_TXSEQ_DUPLICATE;
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) >= chan->tx_win) {
		/* A source of invalid packets is a "double poll" condition,
		 * where delays cause us to send multiple poll packets.  If
		 * the remote stack receives and processes both polls,
		 * sequence numbers can wrap around in such a way that a
		 * resent frame has a sequence number that looks like new data
		 * with a sequence gap.  This would trigger an erroneous SREJ
		 * request.
		 *
		 * Fortunately, this is impossible with a tx window that's
		 * less than half of the maximum sequence number, which allows
		 * invalid frames to be safely ignored.
		 *
		 * With tx window sizes greater than half of the tx window
		 * maximum, the frame is invalid and cannot be ignored.  This
		 * causes a disconnect.
		 */

		if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
			BT_DBG("Invalid/Ignore - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID_IGNORE;
		} else {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		}
	} else {
		BT_DBG("Unexpected - txseq indicates missing frames");
		return L2CAP_TXSEQ_UNEXPECTED;
	}
}

static int l2cap_rx_state_recv(struct l2cap_chan *chan,
			       struct l2cap_ctrl *control,
			       struct sk_buff *skb, u8 event)
{
	int err = 0;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, control->txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			l2cap_pass_to_tx(chan, control);

			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding expected seq %d",
				       control->txseq);
				break;
			}

			chan->expected_tx_seq = __next_seq(chan,
							   control->txseq);

			chan->buffer_seq = chan->expected_tx_seq;
			skb_in_use = 1;

			err = l2cap_reassemble_sdu(chan, skb, control);
			if (err)
				break;

			if (control->final) {
				if (!test_and_clear_bit(CONN_REJ_ACT,
							&chan->conn_state)) {
					control->final = 0;
					l2cap_retransmit_all(chan, control);
					l2cap_ertm_send(chan);
				}
			}

			if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state))
				l2cap_send_ack(chan);
			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			l2cap_pass_to_tx(chan, control);

			/* Can't issue SREJ frames in the local busy state.
			 * Drop this frame, it will be seen as missing
			 * when local busy is exited.
			 */
			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding unexpected seq %d",
				       control->txseq);
				break;
			}

			/* There was a gap in the sequence, so an SREJ
			 * must be sent for each missing frame.  The
			 * current frame is stored for later use.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			clear_bit(CONN_SREJ_ACT, &chan->conn_state);
			l2cap_seq_list_clear(&chan->srej_list);
			l2cap_send_srej(chan, control->txseq);

			chan->rx_state = L2CAP_RX_STATE_SREJ_SENT;
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state) &&
			    !__chan_is_moving(chan)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			l2cap_send_i_or_rr_or_rnr(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_ertm_send(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control && control->poll) {
			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_rr_or_rnr(chan, 0);
		}
		__clear_retrans_timer(chan);
		l2cap_seq_list_clear(&chan->retrans_list);
		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	default:
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_rx_state_srej_sent(struct l2cap_chan *chan,
				    struct l2cap_ctrl *control,
				    struct sk_buff *skb, u8 event)
{
	int err = 0;
	u16 txseq = control->txseq;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			/* Keep frame for reassembly later */
			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			chan->expected_tx_seq = __next_seq(chan, txseq);
			break;
		case L2CAP_TXSEQ_EXPECTED_SREJ:
			l2cap_seq_list_pop(&chan->srej_list);

			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			err = l2cap_rx_queued_iframes(chan);
			if (err)
				break;

			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			/* Got a frame that can't be reassembled yet.
			 * Save it for later, and send SREJs to cover
			 * the missing frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_UNEXPECTED_SREJ:
			/* This frame was requested with an SREJ, but
			 * some expected retransmitted frames are
			 * missing.  Request retransmission of missing
			 * SREJ'd frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej_list(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_DUPLICATE_SREJ:
			/* We've already queued this frame.  Drop this copy. */
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			/* Expecting a later sequence number, so this frame
			 * was already received.  Ignore it completely.
			 */
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT,
						&chan->conn_state)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames) {
				__set_retrans_timer(chan);
			}

			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_srej_tail(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_send_ack(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control->poll) {
			l2cap_send_srej_tail(chan);
		} else {
			struct l2cap_ctrl rr_control;
			memset(&rr_control, 0, sizeof(rr_control));
			rr_control.sframe = 1;
			rr_control.super = L2CAP_SUPER_RR;
			rr_control.reqseq = chan->buffer_seq;
			l2cap_send_sframe(chan, &rr_control);
		}

		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_finish_move(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	chan->rx_state = L2CAP_RX_STATE_RECV;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	return l2cap_resegment(chan);
}

static int l2cap_rx_state_wait_p(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	if (!control->poll)
		return -EPROTO;

	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	err = l2cap_finish_move(chan);
	if (err)
		return err;

	set_bit(CONN_SEND_FBIT, &chan->conn_state);
	l2cap_send_i_or_rr_or_rnr(chan);

	if (event == L2CAP_EV_RECV_IFRAME)
		return -EPROTO;

	return l2cap_rx_state_recv(chan, control, NULL, event);
}

static int l2cap_rx_state_wait_f(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	if (!control->final)
		return -EPROTO;

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	chan->rx_state = L2CAP_RX_STATE_RECV;
	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	err = l2cap_resegment(chan);

	if (!err)
		err = l2cap_rx_state_recv(chan, control, skb, event);

	return err;
}

static bool __valid_reqseq(struct l2cap_chan *chan, u16 reqseq)
{
	/* Make sure reqseq is for a packet that has been sent but not acked */
	u16 unacked;

	unacked = __seq_offset(chan, chan->next_tx_seq, chan->expected_ack_seq);
	return __seq_offset(chan, chan->next_tx_seq, reqseq) <= unacked;
}

static int l2cap_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
		    struct sk_buff *skb, u8 event)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d, state %d", chan,
	       control, skb, event, chan->rx_state);

	if (__valid_reqseq(chan, control->reqseq)) {
		switch (chan->rx_state) {
		case L2CAP_RX_STATE_RECV:
			err = l2cap_rx_state_recv(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_SREJ_SENT:
			err = l2cap_rx_state_srej_sent(chan, control, skb,
						       event);
			break;
		case L2CAP_RX_STATE_WAIT_P:
			err = l2cap_rx_state_wait_p(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_WAIT_F:
			err = l2cap_rx_state_wait_f(chan, control, skb, event);
			break;
		default:
			/* shut it down */
			break;
		}
	} else {
		BT_DBG("Invalid reqseq %d (next_tx_seq %d, expected_ack_seq %d",
		       control->reqseq, chan->next_tx_seq,
		       chan->expected_ack_seq);
		l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return err;
}

static int l2cap_stream_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
			   struct sk_buff *skb)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, state %d", chan, control, skb,
	       chan->rx_state);

	if (l2cap_classify_txseq(chan, control->txseq) ==
	    L2CAP_TXSEQ_EXPECTED) {
		l2cap_pass_to_tx(chan, control);

		BT_DBG("buffer_seq %d->%d", chan->buffer_seq,
		       __next_seq(chan, chan->buffer_seq));

		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);

		l2cap_reassemble_sdu(chan, skb, control);
	} else {
		if (chan->sdu) {
			kfree_skb(chan->sdu);
			chan->sdu = NULL;
		}
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;

		if (skb) {
			BT_DBG("Freeing %p", skb);
			kfree_skb(skb);
		}
	}

	chan->last_acked_seq = control->txseq;
	chan->expected_tx_seq = __next_seq(chan, control->txseq);

	return err;
}

static int l2cap_data_rcv(struct l2cap_chan *chan, struct sk_buff *skb)
{
	struct l2cap_ctrl *control = &bt_cb(skb)->control;
	u16 len;
	u8 event;

	__unpack_control(chan, skb);

	len = skb->len;

	/*
	 * We can just drop the corrupted I-frame here.
	 * Receiver will miss it and start proper recovery
	 * procedures and ask for retransmission.
	 */
	if (l2cap_check_fcs(chan, skb))
		goto drop;

	if (!control->sframe && control->sar == L2CAP_SAR_START)
		len -= L2CAP_SDULEN_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16)
		len -= L2CAP_FCS_SIZE;

	if (len > chan->mps) {
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto drop;
	}

	if (!control->sframe) {
		int err;

		BT_DBG("iframe sar %d, reqseq %d, final %d, txseq %d",
		       control->sar, control->reqseq, control->final,
		       control->txseq);

		/* Validate F-bit - F=0 always valid, F=1 only
		 * valid in TX WAIT_F
		 */
		if (control->final && chan->tx_state != L2CAP_TX_STATE_WAIT_F)
			goto drop;

		if (chan->mode != L2CAP_MODE_STREAMING) {
			event = L2CAP_EV_RECV_IFRAME;
			err = l2cap_rx(chan, control, skb, event);
		} else {
			err = l2cap_stream_rx(chan, control, skb);
		}

		if (err)
			l2cap_send_disconn_req(chan, ECONNRESET);
	} else {
		const u8 rx_func_to_event[4] = {
			L2CAP_EV_RECV_RR, L2CAP_EV_RECV_REJ,
			L2CAP_EV_RECV_RNR, L2CAP_EV_RECV_SREJ
		};

		/* Only I-frames are expected in streaming mode */
		if (chan->mode == L2CAP_MODE_STREAMING)
			goto drop;

		BT_DBG("sframe reqseq %d, final %d, poll %d, super %d",
		       control->reqseq, control->final, control->poll,
		       control->super);

		if (len != 0) {
			BT_ERR("Trailing bytes: %d in sframe", len);
			l2cap_send_disconn_req(chan, ECONNRESET);
			goto drop;
		}

		/* Validate F and P bits */
		if (control->final && (control->poll ||
				       chan->tx_state != L2CAP_TX_STATE_WAIT_F))
			goto drop;

		event = rx_func_to_event[control->super];
		if (l2cap_rx(chan, control, skb, event))
			l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return 0;

drop:
	kfree_skb(skb);
	return 0;
}

static void l2cap_data_channel(struct l2cap_conn *conn, u16 cid,
			       struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_scid(conn, cid);
	if (!chan) {
		if (cid == L2CAP_CID_A2MP) {
			chan = a2mp_channel_create(conn, skb);
			if (!chan) {
				kfree_skb(skb);
				return;
			}

			l2cap_chan_lock(chan);
		} else {
			BT_DBG("unknown cid 0x%4.4x", cid);
			/* Drop packet and return */
			kfree_skb(skb);
			return;
		}
	}

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_CONNECTED)
		goto drop;

	switch (chan->mode) {
	case L2CAP_MODE_BASIC:
		/* If socket recv buffers overflows we drop data here
		 * which is *bad* because L2CAP has to be reliable.
		 * But we don't have any other choice. L2CAP doesn't
		 * provide flow control mechanism. */

		if (chan->imtu < skb->len)
			goto drop;

		if (!chan->ops->recv(chan, skb))
			goto done;
		break;

	case L2CAP_MODE_ERTM:
	case L2CAP_MODE_STREAMING:
		l2cap_data_rcv(chan, skb);
		goto done;

	default:
		BT_DBG("chan %p: bad mode 0x%2.2x", chan, chan->mode);
		break;
	}

drop:
	kfree_skb(skb);

done:
	l2cap_chan_unlock(chan);
}

static void l2cap_conless_channel(struct l2cap_conn *conn, __le16 psm,
				  struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_psm(0, psm, conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_att_channel(struct l2cap_conn *conn,
			      struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_scid(0, L2CAP_CID_LE_DATA,
					 conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_recv_frame(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct l2cap_hdr *lh = (void *) skb->data;
	u16 cid, len;
	__le16 psm;

	skb_pull(skb, L2CAP_HDR_SIZE);
	cid = __le16_to_cpu(lh->cid);
	len = __le16_to_cpu(lh->len);

	if (len != skb->len) {
		kfree_skb(skb);
		return;
	}

	BT_DBG("len %d, cid 0x%4.4x", len, cid);

	switch (cid) {
	case L2CAP_CID_LE_SIGNALING:
	case L2CAP_CID_SIGNALING:
		l2cap_sig_channel(conn, skb);
		break;

	case L2CAP_CID_CONN_LESS:
		psm = get_unaligned((__le16 *) skb->data);
		skb_pull(skb, L2CAP_PSMLEN_SIZE);
		l2cap_conless_channel(conn, psm, skb);
		break;

	case L2CAP_CID_LE_DATA:
		l2cap_att_channel(conn, skb);
		break;

	case L2CAP_CID_SMP:
		if (smp_sig_channel(conn, skb))
			l2cap_conn_del(conn->hcon, EACCES);
		break;

	default:
		l2cap_data_channel(conn, cid, skb);
		break;
	}
}

/* ---- L2CAP interface with lower layer (HCI) ---- */

int l2cap_connect_ind(struct hci_dev *hdev, bdaddr_t *bdaddr)
{
	int exact = 0, lm1 = 0, lm2 = 0;
	struct l2cap_chan *c;

	BT_DBG("hdev %s, bdaddr %pMR", hdev->name, bdaddr);

	/* Find listening sockets and check their link_mode */
	read_lock(&chan_list_lock);
	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		if (c->state != BT_LISTEN)
			continue;

		if (!bacmp(&bt_sk(sk)->src, &hdev->bdaddr)) {
			lm1 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm1 |= HCI_LM_MASTER;
			exact++;
		} else if (!bacmp(&bt_sk(sk)->src, BDADDR_ANY)) {
			lm2 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm2 |= HCI_LM_MASTER;
		}
	}
	read_unlock(&chan_list_lock);

	return exact ? lm1 : lm2;
}

void l2cap_connect_cfm(struct hci_conn *hcon, u8 status)
{
	struct l2cap_conn *conn;

	BT_DBG("hcon %p bdaddr %pMR status %d", hcon, &hcon->dst, status);

	if (!status) {
		conn = l2cap_conn_add(hcon);
		if (conn)
			l2cap_conn_ready(conn);
	} else {
		l2cap_conn_del(hcon, bt_to_errno(status));
	}
}

int l2cap_disconn_ind(struct hci_conn *hcon)
{
	struct l2cap_conn *conn = hcon->l2cap_data;

	BT_DBG("hcon %p", hcon);

	if (!conn)
		return HCI_ERROR_REMOTE_USER_TERM;
	return conn->disc_reason;
}

void l2cap_disconn_cfm(struct hci_conn *hcon, u8 reason)
{
	BT_DBG("hcon %p reason %d", hcon, reason);

	l2cap_conn_del(hcon, bt_to_errno(reason));
}

static inline void l2cap_check_encryption(struct l2cap_chan *chan, u8 encrypt)
{
	if (chan->chan_type != L2CAP_CHAN_CONN_ORIENTED)
		return;

	if (encrypt == 0x00) {
		if (chan->sec_level == BT_SECURITY_MEDIUM) {
			__set_chan_timer(chan, L2CAP_ENC_TIMEOUT);
		} else if (chan->sec_level == BT_SECURITY_HIGH)
			l2cap_chan_close(chan, ECONNREFUSED);
	} else {
		if (chan->sec_level == BT_SECURITY_MEDIUM)
			__clear_chan_timer(chan);
	}
}

int l2cap_security_cfm(struct hci_conn *hcon, u8 status, u8 encrypt)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_chan *chan;

	if (!conn)
		return 0;

	BT_DBG("conn %p status 0x%2.2x encrypt %u", conn, status, encrypt);

	if (hcon->type == LE_LINK) {
		if (!status && encrypt)
			smp_distribute_keys(conn, 0);
		cancel_delayed_work(&conn->security_timer);
	}

	mutex_lock(&conn->chan_lock);

	list_for_each_entry(chan, &conn->chan_l, list) {
		l2cap_chan_lock(chan);

		BT_DBG("chan %p scid 0x%4.4x state %s", chan, chan->scid,
		       state_to_string(chan->state));

		if (chan->chan_type == L2CAP_CHAN_CONN_FIX_A2MP) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->scid == L2CAP_CID_LE_DATA) {
			if (!status && encrypt) {
				chan->sec_level = hcon->sec_level;
				l2cap_chan_ready(chan);
			}

			l2cap_chan_unlock(chan);
			continue;
		}

		if (!__l2cap_no_conn_pending(chan)) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (!status && (chan->state == BT_CONNECTED ||
				chan->state == BT_CONFIG)) {
			struct sock *sk = chan->sk;

			clear_bit(BT_SK_SUSPEND, &bt_sk(sk)->flags);
			sk->sk_state_change(sk);

			l2cap_check_encryption(chan, encrypt);
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->state == BT_CONNECT) {
			if (!status) {
				l2cap_start_connection(chan);
			} else {
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
			}
		} else if (chan->state == BT_CONNECT2) {
			struct sock *sk = chan->sk;
			struct l2cap_conn_rsp rsp;
			__u16 res, stat;

			lock_sock(sk);

			if (!status) {
				if (test_bit(BT_SK_DEFER_SETUP,
					     &bt_sk(sk)->flags)) {
					res = L2CAP_CR_PEND;
					stat = L2CAP_CS_AUTHOR_PEND;
					chan->ops->defer(chan);
				} else {
					__l2cap_state_change(chan, BT_CONFIG);
					res = L2CAP_CR_SUCCESS;
					stat = L2CAP_CS_NO_INFO;
				}
			} else {
				__l2cap_state_change(chan, BT_DISCONN);
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
				res = L2CAP_CR_SEC_BLOCK;
				stat = L2CAP_CS_NO_INFO;
			}

			release_sock(sk);

			rsp.scid   = cpu_to_le16(chan->dcid);
			rsp.dcid   = cpu_to_le16(chan->scid);
			rsp.result = cpu_to_le16(res);
			rsp.status = cpu_to_le16(stat);
			l2cap_send_cmd(conn, chan->ident, L2CAP_CONN_RSP,
				       sizeof(rsp), &rsp);

			if (!test_bit(CONF_REQ_SENT, &chan->conf_state) &&
			    res == L2CAP_CR_SUCCESS) {
				char buf[128];
				set_bit(CONF_REQ_SENT, &chan->conf_state);
				l2cap_send_cmd(conn, l2cap_get_ident(conn),
					       L2CAP_CONF_REQ,
					       l2cap_build_conf_req(chan, buf),
					       buf);
				chan->num_conf_req++;
			}
		}

		l2cap_chan_unlock(chan);
	}

	mutex_unlock(&conn->chan_lock);

	return 0;
}

int l2cap_recv_acldata(struct hci_conn *hcon, struct sk_buff *skb, u16 flags)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_hdr *hdr;
	int len;

	/* For AMP controller do not create l2cap conn */
	if (!conn && hcon->hdev->dev_type != HCI_BREDR)
		goto drop;

	if (!conn)
		conn = l2cap_conn_add(hcon);

	if (!conn)
		goto drop;

	BT_DBG("conn %p len %d flags 0x%x", conn, skb->len, flags);

	switch (flags) {
	case ACL_START:
	case ACL_START_NO_FLUSH:
	case ACL_COMPLETE:
		if (conn->rx_len) {
			BT_ERR("Unexpected start frame (len %d)", skb->len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
		}

		/* Start fragment always begin with Basic L2CAP header */
		if (skb->len < L2CAP_HDR_SIZE) {
			BT_ERR("Frame is too short (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		hdr = (struct l2cap_hdr *) skb->data;
		len = __le16_to_cpu(hdr->len) + L2CAP_HDR_SIZE;

		if (len == skb->len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, skb);
			return 0;
		}

		BT_DBG("Start: total len %d, frag len %d", len, skb->len);

		if (skb->len > len) {
			BT_ERR("Frame is too long (len %d, expected len %d)",
			       skb->len, len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		/* Allocate skb for the complete frame (with header) */
		conn->rx_skb = bt_skb_alloc(len, GFP_KERNEL);
		if (!conn->rx_skb)
			goto drop;

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len = len - skb->len;
		break;

	case ACL_CONT:
		BT_DBG("Cont: frag len %d (expecting %d)", skb->len, conn->rx_len);

		if (!conn->rx_len) {
			BT_ERR("Unexpected continuation frame (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		if (skb->len > conn->rx_len) {
			BT_ERR("Fragment is too long (len %d, expected %d)",
			       skb->len, conn->rx_len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len -= skb->len;

		if (!conn->rx_len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, conn->rx_skb);
			conn->rx_skb = NULL;
		}
		break;
	}

drop:
	kfree_skb(skb);
	return 0;
}

static int l2cap_debugfs_show(struct seq_file *f, void *p)
{
	struct l2cap_chan *c;

	read_lock(&chan_list_lock);

	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		seq_printf(f, "%pMR %pMR %d %d 0x%4.4x 0x%4.4x %d %d %d %d\n",
			   &bt_sk(sk)->src, &bt_sk(sk)->dst,
			   c->state, __le16_to_cpu(c->psm),
			   c->scid, c->dcid, c->imtu, c->omtu,
			   c->sec_level, c->mode);
	}

	read_unlock(&chan_list_lock);

	return 0;
}

static int l2cap_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, l2cap_debugfs_show, inode->i_private);
}

static const struct file_operations l2cap_debugfs_fops = {
	.open		= l2cap_debugfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *l2cap_debugfs;

int __init l2cap_init(void)
{
	int err;

	err = l2cap_init_sockets();
	if (err < 0)
		return err;

	if (bt_debugfs) {
		l2cap_debugfs = debugfs_create_file("l2cap", 0444, bt_debugfs,
						    NULL, &l2cap_debugfs_fops);
		if (!l2cap_debugfs)
			BT_ERR("Failed to create L2CAP debug file");
	}

	return 0;
}

void l2cap_exit(void)
{
	debugfs_remove(l2cap_debugfs);
	l2cap_cleanup_sockets();
}

module_param(disable_ertm, bool, 0644);
MODULE_PARM_DESC(disable_ertm, "Disable enhanced retransmission mode");
{
	struct l2cap_conf_req *req = (struct l2cap_conf_req *) data;
	u16 dcid, flags;
	u8 rsp[64];
	struct l2cap_chan *chan;
	int len, err = 0;

	if (cmd_len < sizeof(*req))
		return -EPROTO;

	dcid  = __le16_to_cpu(req->dcid);
	flags = __le16_to_cpu(req->flags);

	BT_DBG("dcid 0x%4.4x flags 0x%2.2x", dcid, flags);

	chan = l2cap_get_chan_by_scid(conn, dcid);
	if (!chan)
		return -ENOENT;

	if (chan->state != BT_CONFIG && chan->state != BT_CONNECT2) {
		struct l2cap_cmd_rej_cid rej;

		rej.reason = __constant_cpu_to_le16(L2CAP_REJ_INVALID_CID);
		rej.scid = cpu_to_le16(chan->scid);
		rej.dcid = cpu_to_le16(chan->dcid);

		l2cap_send_cmd(conn, cmd->ident, L2CAP_COMMAND_REJ,
			       sizeof(rej), &rej);
		goto unlock;
	}
	if (chan->state != BT_CONFIG && chan->state != BT_CONNECT2) {
		struct l2cap_cmd_rej_cid rej;

		rej.reason = __constant_cpu_to_le16(L2CAP_REJ_INVALID_CID);
		rej.scid = cpu_to_le16(chan->scid);
		rej.dcid = cpu_to_le16(chan->dcid);

		l2cap_send_cmd(conn, cmd->ident, L2CAP_COMMAND_REJ,
			       sizeof(rej), &rej);
		goto unlock;
	}

	/* Reject if config buffer is too small. */
	len = cmd_len - sizeof(*req);
	if (chan->conf_len + len > sizeof(chan->conf_req)) {
		l2cap_send_cmd(conn, cmd->ident, L2CAP_CONF_RSP,
			       l2cap_build_conf_rsp(chan, rsp,
			       L2CAP_CONF_REJECT, flags), rsp);
		goto unlock;
	}
	    test_bit(CONF_LOC_CONF_PEND, &chan->conf_state)) {

		/* check compatibility */

		/* Send rsp for BR/EDR channel */
		if (!chan->hs_hcon)
			l2cap_send_efs_conf_rsp(chan, rsp, cmd->ident, flags);
		else
			chan->ident = cmd->ident;
	}

unlock:
	l2cap_chan_unlock(chan);
	return err;
}

static inline int l2cap_config_rsp(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				   u8 *data)
{
	struct l2cap_conf_rsp *rsp = (struct l2cap_conf_rsp *)data;
	u16 scid, flags, result;
	struct l2cap_chan *chan;
	int len = cmd_len - sizeof(*rsp);
	int err = 0;

	if (cmd_len < sizeof(*rsp))
		return -EPROTO;

	scid   = __le16_to_cpu(rsp->scid);
	flags  = __le16_to_cpu(rsp->flags);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("scid 0x%4.4x flags 0x%2.2x result 0x%2.2x len %d", scid, flags,
	       result, len);

	chan = l2cap_get_chan_by_scid(conn, scid);
	if (!chan)
		return 0;

	switch (result) {
	case L2CAP_CONF_SUCCESS:
		l2cap_conf_rfc_get(chan, rsp->data, len);
		clear_bit(CONF_REM_CONF_PEND, &chan->conf_state);
		break;

	case L2CAP_CONF_PENDING:
		set_bit(CONF_REM_CONF_PEND, &chan->conf_state);

		if (test_bit(CONF_LOC_CONF_PEND, &chan->conf_state)) {
			char buf[64];

			len = l2cap_parse_conf_rsp(chan, rsp->data, len,
						   buf, &result);
			if (len < 0) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			if (!chan->hs_hcon) {
				l2cap_send_efs_conf_rsp(chan, buf, cmd->ident,
							0);
			} else {
				if (l2cap_check_efs(chan)) {
					amp_create_logical_link(chan);
					chan->ident = cmd->ident;
				}
			}
		}
		goto done;

	case L2CAP_CONF_UNACCEPT:
		if (chan->num_conf_rsp <= L2CAP_CONF_MAX_CONF_RSP) {
			char req[64];

			if (len > sizeof(req) - sizeof(struct l2cap_conf_req)) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			/* throw out any old stored conf requests */
			result = L2CAP_CONF_SUCCESS;
			len = l2cap_parse_conf_rsp(chan, rsp->data, len,
						   req, &result);
			if (len < 0) {
				l2cap_send_disconn_req(chan, ECONNRESET);
				goto done;
			}

			l2cap_send_cmd(conn, l2cap_get_ident(conn),
				       L2CAP_CONF_REQ, len, req);
			chan->num_conf_req++;
			if (result != L2CAP_CONF_SUCCESS)
				goto done;
			break;
		}

	default:
		l2cap_chan_set_err(chan, ECONNRESET);

		__set_chan_timer(chan, L2CAP_DISC_REJ_TIMEOUT);
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto done;
	}

	if (flags & L2CAP_CONF_FLAG_CONTINUATION)
		goto done;

	set_bit(CONF_INPUT_DONE, &chan->conf_state);

	if (test_bit(CONF_OUTPUT_DONE, &chan->conf_state)) {
		set_default_fcs(chan);

		if (chan->mode == L2CAP_MODE_ERTM ||
		    chan->mode == L2CAP_MODE_STREAMING)
			err = l2cap_ertm_init(chan);

		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}

done:
	l2cap_chan_unlock(chan);
	return err;
}

static inline int l2cap_disconnect_req(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_req *req = (struct l2cap_disconn_req *) data;
	struct l2cap_disconn_rsp rsp;
	u16 dcid, scid;
	struct l2cap_chan *chan;
	struct sock *sk;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	scid = __le16_to_cpu(req->scid);
	dcid = __le16_to_cpu(req->dcid);

	BT_DBG("scid 0x%4.4x dcid 0x%4.4x", scid, dcid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, dcid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	sk = chan->sk;

	rsp.dcid = cpu_to_le16(chan->scid);
	rsp.scid = cpu_to_le16(chan->dcid);
	l2cap_send_cmd(conn, cmd->ident, L2CAP_DISCONN_RSP, sizeof(rsp), &rsp);

	lock_sock(sk);
	sk->sk_shutdown = SHUTDOWN_MASK;
	release_sock(sk);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, ECONNRESET);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_disconnect_rsp(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_rsp *rsp = (struct l2cap_disconn_rsp *) data;
	u16 dcid, scid;
	struct l2cap_chan *chan;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	scid = __le16_to_cpu(rsp->scid);
	dcid = __le16_to_cpu(rsp->dcid);

	BT_DBG("dcid 0x%4.4x scid 0x%4.4x", dcid, scid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, scid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, 0);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_information_req(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_req *req = (struct l2cap_info_req *) data;
	u16 type;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	type = __le16_to_cpu(req->type);

	BT_DBG("type 0x%4.4x", type);

	if (type == L2CAP_IT_FEAT_MASK) {
		u8 buf[8];
		u32 feat_mask = l2cap_feat_mask;
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;
		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FEAT_MASK);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		if (!disable_ertm)
			feat_mask |= L2CAP_FEAT_ERTM | L2CAP_FEAT_STREAMING
				| L2CAP_FEAT_FCS;
		if (enable_hs)
			feat_mask |= L2CAP_FEAT_EXT_FLOW
				| L2CAP_FEAT_EXT_WINDOW;

		put_unaligned_le32(feat_mask, rsp->data);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else if (type == L2CAP_IT_FIXED_CHAN) {
		u8 buf[12];
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;

		if (enable_hs)
			l2cap_fixed_chan[0] |= L2CAP_FC_A2MP;
		else
			l2cap_fixed_chan[0] &= ~L2CAP_FC_A2MP;

		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		memcpy(rsp->data, l2cap_fixed_chan, sizeof(l2cap_fixed_chan));
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else {
		struct l2cap_info_rsp rsp;
		rsp.type   = cpu_to_le16(type);
		rsp.result = __constant_cpu_to_le16(L2CAP_IR_NOTSUPP);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(rsp),
			       &rsp);
	}

	return 0;
}

static inline int l2cap_information_rsp(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) data;
	u16 type, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	type   = __le16_to_cpu(rsp->type);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("type 0x%4.4x result 0x%2.2x", type, result);

	/* L2CAP Info req/rsp are unbound to channels, add extra checks */
	if (cmd->ident != conn->info_ident ||
	    conn->info_state & L2CAP_INFO_FEAT_MASK_REQ_DONE)
		return 0;

	cancel_delayed_work(&conn->info_timer);

	if (result != L2CAP_IR_SUCCESS) {
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);

		return 0;
	}

	switch (type) {
	case L2CAP_IT_FEAT_MASK:
		conn->feat_mask = get_unaligned_le32(rsp->data);

		if (conn->feat_mask & L2CAP_FEAT_FIXED_CHAN) {
			struct l2cap_info_req req;
			req.type = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);

			conn->info_ident = l2cap_get_ident(conn);

			l2cap_send_cmd(conn, conn->info_ident,
				       L2CAP_INFO_REQ, sizeof(req), &req);
		} else {
			conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
			conn->info_ident = 0;

			l2cap_conn_start(conn);
		}
		break;

	case L2CAP_IT_FIXED_CHAN:
		conn->fixed_chan_mask = rsp->data[0];
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);
		break;
	}

	return 0;
}

static int l2cap_create_channel_req(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd,
				    u16 cmd_len, void *data)
{
	struct l2cap_create_chan_req *req = data;
	struct l2cap_create_chan_rsp rsp;
	struct l2cap_chan *chan;
	struct hci_dev *hdev;
	u16 psm, scid;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	if (!enable_hs)
		return -EINVAL;

	psm = le16_to_cpu(req->psm);
	scid = le16_to_cpu(req->scid);

	BT_DBG("psm 0x%2.2x, scid 0x%4.4x, amp_id %d", psm, scid, req->amp_id);

	/* For controller id 0 make BR/EDR connection */
	if (req->amp_id == HCI_BREDR_ID) {
		l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			      req->amp_id);
		return 0;
	}

	/* Validate AMP controller id */
	hdev = hci_dev_get(req->amp_id);
	if (!hdev)
		goto error;

	if (hdev->dev_type != HCI_AMP || !test_bit(HCI_UP, &hdev->flags)) {
		hci_dev_put(hdev);
		goto error;
	}

	chan = l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			     req->amp_id);
	if (chan) {
		struct amp_mgr *mgr = conn->hcon->amp_mgr;
		struct hci_conn *hs_hcon;

		hs_hcon = hci_conn_hash_lookup_ba(hdev, AMP_LINK, conn->dst);
		if (!hs_hcon) {
			hci_dev_put(hdev);
			return -EFAULT;
		}

		BT_DBG("mgr %p bredr_chan %p hs_hcon %p", mgr, chan, hs_hcon);

		mgr->bredr_chan = chan;
		chan->hs_hcon = hs_hcon;
		chan->fcs = L2CAP_FCS_NONE;
		conn->mtu = hdev->block_mtu;
	}

	hci_dev_put(hdev);

	return 0;

error:
	rsp.dcid = 0;
	rsp.scid = cpu_to_le16(scid);
	rsp.result = __constant_cpu_to_le16(L2CAP_CR_BAD_AMP);
	rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CREATE_CHAN_RSP,
		       sizeof(rsp), &rsp);

	return -EFAULT;
}

static void l2cap_send_move_chan_req(struct l2cap_chan *chan, u8 dest_amp_id)
{
	struct l2cap_move_chan_req req;
	u8 ident;

	BT_DBG("chan %p, dest_amp_id %d", chan, dest_amp_id);

	ident = l2cap_get_ident(chan->conn);
	chan->ident = ident;

	req.icid = cpu_to_le16(chan->scid);
	req.dest_amp_id = dest_amp_id;

	l2cap_send_cmd(chan->conn, ident, L2CAP_MOVE_CHAN_REQ, sizeof(req),
		       &req);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_rsp(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_rsp rsp;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	rsp.icid = cpu_to_le16(chan->dcid);
	rsp.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_RSP,
		       sizeof(rsp), &rsp);
}

static void l2cap_send_move_chan_cfm(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	chan->ident = l2cap_get_ident(chan->conn);

	cfm.icid = cpu_to_le16(chan->scid);
	cfm.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_cfm_icid(struct l2cap_conn *conn, u16 icid)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("conn %p, icid 0x%4.4x", conn, icid);

	cfm.icid = cpu_to_le16(icid);
	cfm.result = __constant_cpu_to_le16(L2CAP_MC_UNCONFIRMED);

	l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);
}

static void l2cap_send_move_chan_cfm_rsp(struct l2cap_conn *conn, u8 ident,
					 u16 icid)
{
	struct l2cap_move_chan_cfm_rsp rsp;

	BT_DBG("icid 0x%4.4x", icid);

	rsp.icid = cpu_to_le16(icid);
	l2cap_send_cmd(conn, ident, L2CAP_MOVE_CHAN_CFM_RSP, sizeof(rsp), &rsp);
}

static void __release_logical_link(struct l2cap_chan *chan)
{
	chan->hs_hchan = NULL;
	chan->hs_hcon = NULL;

	/* Placeholder - release the logical link */
}

static void l2cap_logical_fail(struct l2cap_chan *chan)
{
	/* Logical link setup failed */
	if (chan->state != BT_CONNECTED) {
		/* Create channel failure, disconnect */
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	switch (chan->move_role) {
	case L2CAP_MOVE_ROLE_RESPONDER:
		l2cap_move_done(chan);
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_SUPP);
		break;
	case L2CAP_MOVE_ROLE_INITIATOR:
		if (chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_COMP ||
		    chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_CFM) {
			/* Remote has only sent pending or
			 * success responses, clean up
			 */
			l2cap_move_done(chan);
		}

		/* Other amp move states imply that the move
		 * has already aborted
		 */
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
		break;
	}
}

static void l2cap_logical_finish_create(struct l2cap_chan *chan,
					struct hci_chan *hchan)
{
	struct l2cap_conf_rsp rsp;

	chan->hs_hchan = hchan;
	chan->hs_hcon->l2cap_data = chan->conn;

	l2cap_send_efs_conf_rsp(chan, &rsp, chan->ident, 0);

	if (test_bit(CONF_INPUT_DONE, &chan->conf_state)) {
		int err;

		set_default_fcs(chan);

		err = l2cap_ertm_init(chan);
		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}
}

static void l2cap_logical_finish_move(struct l2cap_chan *chan,
				      struct hci_chan *hchan)
{
	chan->hs_hcon = hchan->conn;
	chan->hs_hcon->l2cap_data = chan->conn;

	BT_DBG("move_state %d", chan->move_state);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent after a success
		 * response is received
		 */
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		break;
	case L2CAP_MOVE_WAIT_LOGICAL_CFM:
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);
		}
		break;
	default:
		/* Move was not in expected state, free the channel */
		__release_logical_link(chan);

		chan->move_state = L2CAP_MOVE_STABLE;
	}
}

/* Call with chan locked */
void l2cap_logical_cfm(struct l2cap_chan *chan, struct hci_chan *hchan,
		       u8 status)
{
	BT_DBG("chan %p, hchan %p, status %d", chan, hchan, status);

	if (status) {
		l2cap_logical_fail(chan);
		__release_logical_link(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		/* Ignore logical link if channel is on BR/EDR */
		if (chan->local_amp_id)
			l2cap_logical_finish_create(chan, hchan);
	} else {
		l2cap_logical_finish_move(chan, hchan);
	}
}

void l2cap_move_start(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	if (chan->local_amp_id == HCI_BREDR_ID) {
		if (chan->chan_policy != BT_CHANNEL_POLICY_AMP_PREFERRED)
			return;
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - start physical link setup */
	} else {
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		chan->move_id = 0;
		l2cap_move_setup(chan);
		l2cap_send_move_chan_req(chan, 0);
	}
}

static void l2cap_do_create(struct l2cap_chan *chan, int result,
			    u8 local_amp_id, u8 remote_amp_id)
{
	BT_DBG("chan %p state %s %u -> %u", chan, state_to_string(chan->state),
	       local_amp_id, remote_amp_id);

	chan->fcs = L2CAP_FCS_NONE;

	/* Outgoing channel on AMP */
	if (chan->state == BT_CONNECT) {
		if (result == L2CAP_CR_SUCCESS) {
			chan->local_amp_id = local_amp_id;
			l2cap_send_create_chan_req(chan, remote_amp_id);
		} else {
			/* Revert to BR/EDR connect */
			l2cap_send_conn_req(chan);
		}

		return;
	}

	/* Incoming channel on AMP */
	if (__l2cap_no_conn_pending(chan)) {
		struct l2cap_conn_rsp rsp;
		char buf[128];
		rsp.scid = cpu_to_le16(chan->dcid);
		rsp.dcid = cpu_to_le16(chan->scid);

		if (result == L2CAP_CR_SUCCESS) {
			/* Send successful response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_SUCCESS);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		} else {
			/* Send negative response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_NO_MEM);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		}

		l2cap_send_cmd(chan->conn, chan->ident, L2CAP_CREATE_CHAN_RSP,
			       sizeof(rsp), &rsp);

		if (result == L2CAP_CR_SUCCESS) {
			__l2cap_state_change(chan, BT_CONFIG);
			set_bit(CONF_REQ_SENT, &chan->conf_state);
			l2cap_send_cmd(chan->conn, l2cap_get_ident(chan->conn),
				       L2CAP_CONF_REQ,
				       l2cap_build_conf_req(chan, buf), buf);
			chan->num_conf_req++;
		}
	}
}

static void l2cap_do_move_initiate(struct l2cap_chan *chan, u8 local_amp_id,
				   u8 remote_amp_id)
{
	l2cap_move_setup(chan);
	chan->move_id = local_amp_id;
	chan->move_state = L2CAP_MOVE_WAIT_RSP;

	l2cap_send_move_chan_req(chan, remote_amp_id);
}

static void l2cap_do_move_respond(struct l2cap_chan *chan, int result)
{
	struct hci_chan *hchan = NULL;

	/* Placeholder - get hci_chan for logical link */

	if (hchan) {
		if (hchan->state == BT_CONNECTED) {
			/* Logical link is ready to go */
			chan->hs_hcon = hchan->conn;
			chan->hs_hcon->l2cap_data = chan->conn;
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);

			l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		} else {
			/* Wait for logical link to be ready */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		}
	} else {
		/* Logical link not available */
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_ALLOWED);
	}
}

static void l2cap_do_move_cancel(struct l2cap_chan *chan, int result)
{
	if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
		u8 rsp_result;
		if (result == -EINVAL)
			rsp_result = L2CAP_MR_BAD_ID;
		else
			rsp_result = L2CAP_MR_NOT_ALLOWED;

		l2cap_send_move_chan_rsp(chan, rsp_result);
	}

	chan->move_role = L2CAP_MOVE_ROLE_NONE;
	chan->move_state = L2CAP_MOVE_STABLE;

	/* Restart data transmission */
	l2cap_ertm_send(chan);
}

/* Invoke with locked chan */
void __l2cap_physical_cfm(struct l2cap_chan *chan, int result)
{
	u8 local_amp_id = chan->local_amp_id;
	u8 remote_amp_id = chan->remote_amp_id;

	BT_DBG("chan %p, result %d, local_amp_id %d, remote_amp_id %d",
	       chan, result, local_amp_id, remote_amp_id);

	if (chan->state == BT_DISCONN || chan->state == BT_CLOSED) {
		l2cap_chan_unlock(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		l2cap_do_create(chan, result, local_amp_id, remote_amp_id);
	} else if (result != L2CAP_MR_SUCCESS) {
		l2cap_do_move_cancel(chan, result);
	} else {
		switch (chan->move_role) {
		case L2CAP_MOVE_ROLE_INITIATOR:
			l2cap_do_move_initiate(chan, local_amp_id,
					       remote_amp_id);
			break;
		case L2CAP_MOVE_ROLE_RESPONDER:
			l2cap_do_move_respond(chan, result);
			break;
		default:
			l2cap_do_move_cancel(chan, result);
			break;
		}
	}
}

static inline int l2cap_move_channel_req(struct l2cap_conn *conn,
					 struct l2cap_cmd_hdr *cmd,
					 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_req *req = data;
	struct l2cap_move_chan_rsp rsp;
	struct l2cap_chan *chan;
	u16 icid = 0;
	u16 result = L2CAP_MR_NOT_ALLOWED;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	icid = le16_to_cpu(req->icid);

	BT_DBG("icid 0x%4.4x, dest_amp_id %d", icid, req->dest_amp_id);

	if (!enable_hs)
		return -EINVAL;

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		rsp.icid = cpu_to_le16(icid);
		rsp.result = __constant_cpu_to_le16(L2CAP_MR_NOT_ALLOWED);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_MOVE_CHAN_RSP,
			       sizeof(rsp), &rsp);
		return 0;
	}

	chan->ident = cmd->ident;

	if (chan->scid < L2CAP_CID_DYN_START ||
	    chan->chan_policy == BT_CHANNEL_POLICY_BREDR_ONLY ||
	    (chan->mode != L2CAP_MODE_ERTM &&
	     chan->mode != L2CAP_MODE_STREAMING)) {
		result = L2CAP_MR_NOT_ALLOWED;
		goto send_move_response;
	}

	if (chan->local_amp_id == req->dest_amp_id) {
		result = L2CAP_MR_SAME_ID;
		goto send_move_response;
	}

	if (req->dest_amp_id) {
		struct hci_dev *hdev;
		hdev = hci_dev_get(req->dest_amp_id);
		if (!hdev || hdev->dev_type != HCI_AMP ||
		    !test_bit(HCI_UP, &hdev->flags)) {
			if (hdev)
				hci_dev_put(hdev);

			result = L2CAP_MR_BAD_ID;
			goto send_move_response;
		}
		hci_dev_put(hdev);
	}

	/* Detect a move collision.  Only send a collision response
	 * if this side has "lost", otherwise proceed with the move.
	 * The winner has the larger bd_addr.
	 */
	if ((__chan_is_moving(chan) ||
	     chan->move_role != L2CAP_MOVE_ROLE_NONE) &&
	    bacmp(conn->src, conn->dst) > 0) {
		result = L2CAP_MR_COLLISION;
		goto send_move_response;
	}

	chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
	l2cap_move_setup(chan);
	chan->move_id = req->dest_amp_id;
	icid = chan->dcid;

	if (!req->dest_amp_id) {
		/* Moving to BR/EDR */
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
			result = L2CAP_MR_PEND;
		} else {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			result = L2CAP_MR_SUCCESS;
		}
	} else {
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - uncomment when amp functions are available */
		/*amp_accept_physical(chan, req->dest_amp_id);*/
		result = L2CAP_MR_PEND;
	}

send_move_response:
	l2cap_send_move_chan_rsp(chan, result);

	l2cap_chan_unlock(chan);

	return 0;
}

static void l2cap_move_continue(struct l2cap_conn *conn, u16 icid, u16 result)
{
	struct l2cap_chan *chan;
	struct hci_chan *hchan = NULL;

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan) {
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);
	if (result == L2CAP_MR_PEND)
		__set_chan_timer(chan, L2CAP_MOVE_ERTX_TIMEOUT);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent when logical link
		 * is complete.
		 */
		chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		break;
	case L2CAP_MOVE_WAIT_RSP_SUCCESS:
		if (result == L2CAP_MR_PEND) {
			break;
		} else if (test_bit(CONN_LOCAL_BUSY,
				    &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else {
			/* Logical link is up or moving to BR/EDR,
			 * proceed with move
			 */
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		}
		break;
	case L2CAP_MOVE_WAIT_RSP:
		/* Moving to AMP */
		if (result == L2CAP_MR_SUCCESS) {
			/* Remote is ready, send confirm immediately
			 * after logical link is ready
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		} else {
			/* Both logical link and move success
			 * are required to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_COMP;
		}

		/* Placeholder - get hci_chan for logical link */
		if (!hchan) {
			/* Logical link not available */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
			break;
		}

		/* If the logical link is not yet connected, do not
		 * send confirmation.
		 */
		if (hchan->state != BT_CONNECTED)
			break;

		/* Logical link is already ready to go */

		chan->hs_hcon = hchan->conn;
		chan->hs_hcon->l2cap_data = chan->conn;

		if (result == L2CAP_MR_SUCCESS) {
			/* Can confirm now */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else {
			/* Now only need move success
			 * to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		}

		l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		break;
	default:
		/* Any other amp move state means the move failed. */
		chan->move_id = chan->local_amp_id;
		l2cap_move_done(chan);
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
	}

	l2cap_chan_unlock(chan);
}

static void l2cap_move_fail(struct l2cap_conn *conn, u8 ident, u16 icid,
			    u16 result)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_ident(conn, ident);
	if (!chan) {
		/* Could not locate channel, icid is best guess */
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);

	if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
		if (result == L2CAP_MR_COLLISION) {
			chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
		} else {
			/* Cleanup - cancel move */
			chan->move_id = chan->local_amp_id;
			l2cap_move_done(chan);
		}
	}

	l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);

	l2cap_chan_unlock(chan);
}

static int l2cap_move_channel_rsp(struct l2cap_conn *conn,
				  struct l2cap_cmd_hdr *cmd,
				  u16 cmd_len, void *data)
{
	struct l2cap_move_chan_rsp *rsp = data;
	u16 icid, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);
	result = le16_to_cpu(rsp->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	if (result == L2CAP_MR_SUCCESS || result == L2CAP_MR_PEND)
		l2cap_move_continue(conn, icid, result);
	else
		l2cap_move_fail(conn, cmd->ident, icid, result);

	return 0;
}

static int l2cap_move_channel_confirm(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd,
				      u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm *cfm = data;
	struct l2cap_chan *chan;
	u16 icid, result;

	if (cmd_len != sizeof(*cfm))
		return -EPROTO;

	icid = le16_to_cpu(cfm->icid);
	result = le16_to_cpu(cfm->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		/* Spec requires a response even if the icid was not found */
		l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);
		return 0;
	}

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM) {
		if (result == L2CAP_MC_CONFIRMED) {
			chan->local_amp_id = chan->move_id;
			if (!chan->local_amp_id)
				__release_logical_link(chan);
		} else {
			chan->move_id = chan->local_amp_id;
		}

		l2cap_move_done(chan);
	}

	l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_move_channel_confirm_rsp(struct l2cap_conn *conn,
						 struct l2cap_cmd_hdr *cmd,
						 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm_rsp *rsp = data;
	struct l2cap_chan *chan;
	u16 icid;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);

	BT_DBG("icid 0x%4.4x", icid);

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan)
		return 0;

	__clear_chan_timer(chan);

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM_RSP) {
		chan->local_amp_id = chan->move_id;

		if (!chan->local_amp_id && chan->hs_hchan)
			__release_logical_link(chan);

		l2cap_move_done(chan);
	}

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_check_conn_param(u16 min, u16 max, u16 latency,
					 u16 to_multiplier)
{
	u16 max_latency;

	if (min > max || min < 6 || max > 3200)
		return -EINVAL;

	if (to_multiplier < 10 || to_multiplier > 3200)
		return -EINVAL;

	if (max >= to_multiplier * 8)
		return -EINVAL;

	max_latency = (to_multiplier * 8 / max) - 1;
	if (latency > 499 || latency > max_latency)
		return -EINVAL;

	return 0;
}

static inline int l2cap_conn_param_update_req(struct l2cap_conn *conn,
					      struct l2cap_cmd_hdr *cmd,
					      u8 *data)
{
	struct hci_conn *hcon = conn->hcon;
	struct l2cap_conn_param_update_req *req;
	struct l2cap_conn_param_update_rsp rsp;
	u16 min, max, latency, to_multiplier, cmd_len;
	int err;

	if (!(hcon->link_mode & HCI_LM_MASTER))
		return -EINVAL;

	cmd_len = __le16_to_cpu(cmd->len);
	if (cmd_len != sizeof(struct l2cap_conn_param_update_req))
		return -EPROTO;

	req = (struct l2cap_conn_param_update_req *) data;
	min		= __le16_to_cpu(req->min);
	max		= __le16_to_cpu(req->max);
	latency		= __le16_to_cpu(req->latency);
	to_multiplier	= __le16_to_cpu(req->to_multiplier);

	BT_DBG("min 0x%4.4x max 0x%4.4x latency: 0x%4.4x Timeout: 0x%4.4x",
	       min, max, latency, to_multiplier);

	memset(&rsp, 0, sizeof(rsp));

	err = l2cap_check_conn_param(min, max, latency, to_multiplier);
	if (err)
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_REJECTED);
	else
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_ACCEPTED);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CONN_PARAM_UPDATE_RSP,
		       sizeof(rsp), &rsp);

	if (!err)
		hci_le_conn_update(hcon, min, max, latency, to_multiplier);

	return 0;
}

static inline int l2cap_bredr_sig_cmd(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				      u8 *data)
{
	int err = 0;

	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}

	return err;
}

static inline int l2cap_le_sig_cmd(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u8 *data)
{
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		return 0;

	case L2CAP_CONN_PARAM_UPDATE_REQ:
		return l2cap_conn_param_update_req(conn, cmd, data);

	case L2CAP_CONN_PARAM_UPDATE_RSP:
		return 0;

	default:
		BT_ERR("Unknown LE signaling command 0x%2.2x", cmd->code);
		return -EINVAL;
	}
}

static inline void l2cap_sig_channel(struct l2cap_conn *conn,
				     struct sk_buff *skb)
{
	u8 *data = skb->data;
	int len = skb->len;
	struct l2cap_cmd_hdr cmd;
	int err;

	l2cap_raw_recv(conn, skb);

	while (len >= L2CAP_CMD_HDR_SIZE) {
		u16 cmd_len;
		memcpy(&cmd, data, L2CAP_CMD_HDR_SIZE);
		data += L2CAP_CMD_HDR_SIZE;
		len  -= L2CAP_CMD_HDR_SIZE;

		cmd_len = le16_to_cpu(cmd.len);

		BT_DBG("code 0x%2.2x len %d id 0x%2.2x", cmd.code, cmd_len,
		       cmd.ident);

		if (cmd_len > len || !cmd.ident) {
			BT_DBG("corrupted command");
			break;
		}

		if (conn->hcon->type == LE_LINK)
			err = l2cap_le_sig_cmd(conn, &cmd, data);
		else
			err = l2cap_bredr_sig_cmd(conn, &cmd, cmd_len, data);

		if (err) {
			struct l2cap_cmd_rej_unk rej;

			BT_ERR("Wrong link type (%d)", err);

			/* FIXME: Map err to a valid reason */
			rej.reason = __constant_cpu_to_le16(L2CAP_REJ_NOT_UNDERSTOOD);
			l2cap_send_cmd(conn, cmd.ident, L2CAP_COMMAND_REJ,
				       sizeof(rej), &rej);
		}

		data += cmd_len;
		len  -= cmd_len;
	}

	kfree_skb(skb);
}

static int l2cap_check_fcs(struct l2cap_chan *chan,  struct sk_buff *skb)
{
	u16 our_fcs, rcv_fcs;
	int hdr_size;

	if (test_bit(FLAG_EXT_CTRL, &chan->flags))
		hdr_size = L2CAP_EXT_HDR_SIZE;
	else
		hdr_size = L2CAP_ENH_HDR_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16) {
		skb_trim(skb, skb->len - L2CAP_FCS_SIZE);
		rcv_fcs = get_unaligned_le16(skb->data + skb->len);
		our_fcs = crc16(0, skb->data - hdr_size, skb->len + hdr_size);

		if (our_fcs != rcv_fcs)
			return -EBADMSG;
	}
	return 0;
}

static void l2cap_send_i_or_rr_or_rnr(struct l2cap_chan *chan)
{
	struct l2cap_ctrl control;

	BT_DBG("chan %p", chan);

	memset(&control, 0, sizeof(control));
	control.sframe = 1;
	control.final = 1;
	control.reqseq = chan->buffer_seq;
	set_bit(CONN_SEND_FBIT, &chan->conn_state);

	if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		control.super = L2CAP_SUPER_RNR;
		l2cap_send_sframe(chan, &control);
	}

	if (test_and_clear_bit(CONN_REMOTE_BUSY, &chan->conn_state) &&
	    chan->unacked_frames > 0)
		__set_retrans_timer(chan);

	/* Send pending iframes */
	l2cap_ertm_send(chan);

	if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state) &&
	    test_bit(CONN_SEND_FBIT, &chan->conn_state)) {
		/* F-bit wasn't sent in an s-frame or i-frame yet, so
		 * send it now.
		 */
		control.super = L2CAP_SUPER_RR;
		l2cap_send_sframe(chan, &control);
	}
}

static void append_skb_frag(struct sk_buff *skb, struct sk_buff *new_frag,
			    struct sk_buff **last_frag)
{
	/* skb->len reflects data in skb as well as all fragments
	 * skb->data_len reflects only data in fragments
	 */
	if (!skb_has_frag_list(skb))
		skb_shinfo(skb)->frag_list = new_frag;

	new_frag->next = NULL;

	(*last_frag)->next = new_frag;
	*last_frag = new_frag;

	skb->len += new_frag->len;
	skb->data_len += new_frag->len;
	skb->truesize += new_frag->truesize;
}

static int l2cap_reassemble_sdu(struct l2cap_chan *chan, struct sk_buff *skb,
				struct l2cap_ctrl *control)
{
	int err = -EINVAL;

	switch (control->sar) {
	case L2CAP_SAR_UNSEGMENTED:
		if (chan->sdu)
			break;

		err = chan->ops->recv(chan, skb);
		break;

	case L2CAP_SAR_START:
		if (chan->sdu)
			break;

		chan->sdu_len = get_unaligned_le16(skb->data);
		skb_pull(skb, L2CAP_SDULEN_SIZE);

		if (chan->sdu_len > chan->imtu) {
			err = -EMSGSIZE;
			break;
		}

		if (skb->len >= chan->sdu_len)
			break;

		chan->sdu = skb;
		chan->sdu_last_frag = skb;

		skb = NULL;
		err = 0;
		break;

	case L2CAP_SAR_CONTINUE:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len >= chan->sdu_len)
			break;

		err = 0;
		break;

	case L2CAP_SAR_END:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len != chan->sdu_len)
			break;

		err = chan->ops->recv(chan, chan->sdu);

		if (!err) {
			/* Reassembly complete */
			chan->sdu = NULL;
			chan->sdu_last_frag = NULL;
			chan->sdu_len = 0;
		}
		break;
	}

	if (err) {
		kfree_skb(skb);
		kfree_skb(chan->sdu);
		chan->sdu = NULL;
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;
	}

	return err;
}

static int l2cap_resegment(struct l2cap_chan *chan)
{
	/* Placeholder */
	return 0;
}

void l2cap_chan_busy(struct l2cap_chan *chan, int busy)
{
	u8 event;

	if (chan->mode != L2CAP_MODE_ERTM)
		return;

	event = busy ? L2CAP_EV_LOCAL_BUSY_DETECTED : L2CAP_EV_LOCAL_BUSY_CLEAR;
	l2cap_tx(chan, NULL, NULL, event);
}

static int l2cap_rx_queued_iframes(struct l2cap_chan *chan)
{
	int err = 0;
	/* Pass sequential frames to l2cap_reassemble_sdu()
	 * until a gap is encountered.
	 */

	BT_DBG("chan %p", chan);

	while (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		struct sk_buff *skb;
		BT_DBG("Searching for skb with txseq %d (queue len %d)",
		       chan->buffer_seq, skb_queue_len(&chan->srej_q));

		skb = l2cap_ertm_seq_in_queue(&chan->srej_q, chan->buffer_seq);

		if (!skb)
			break;

		skb_unlink(skb, &chan->srej_q);
		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);
		err = l2cap_reassemble_sdu(chan, skb, &bt_cb(skb)->control);
		if (err)
			break;
	}

	if (skb_queue_empty(&chan->srej_q)) {
		chan->rx_state = L2CAP_RX_STATE_RECV;
		l2cap_send_ack(chan);
	}

	return err;
}

static void l2cap_handle_srej(struct l2cap_chan *chan,
			      struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (skb == NULL) {
		BT_DBG("Seq %d not available for retransmission",
		       control->reqseq);
		return;
	}

	if (chan->max_tx != 0 && bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	if (control->poll) {
		l2cap_pass_to_tx(chan, control);

		set_bit(CONN_SEND_FBIT, &chan->conn_state);
		l2cap_retransmit(chan, control);
		l2cap_ertm_send(chan);

		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
			set_bit(CONN_SREJ_ACT, &chan->conn_state);
			chan->srej_save_reqseq = control->reqseq;
		}
	} else {
		l2cap_pass_to_tx_fbit(chan, control);

		if (control->final) {
			if (chan->srej_save_reqseq != control->reqseq ||
			    !test_and_clear_bit(CONN_SREJ_ACT,
						&chan->conn_state))
				l2cap_retransmit(chan, control);
		} else {
			l2cap_retransmit(chan, control);
			if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
				set_bit(CONN_SREJ_ACT, &chan->conn_state);
				chan->srej_save_reqseq = control->reqseq;
			}
		}
	}
}

static void l2cap_handle_rej(struct l2cap_chan *chan,
			     struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (chan->max_tx && skb &&
	    bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	l2cap_pass_to_tx(chan, control);

	if (control->final) {
		if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state))
			l2cap_retransmit_all(chan, control);
	} else {
		l2cap_retransmit_all(chan, control);
		l2cap_ertm_send(chan);
		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F)
			set_bit(CONN_REJ_ACT, &chan->conn_state);
	}
}

static u8 l2cap_classify_txseq(struct l2cap_chan *chan, u16 txseq)
{
	BT_DBG("chan %p, txseq %d", chan, txseq);

	BT_DBG("last_acked_seq %d, expected_tx_seq %d", chan->last_acked_seq,
	       chan->expected_tx_seq);

	if (chan->rx_state == L2CAP_RX_STATE_SREJ_SENT) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			/* See notes below regarding "double poll" and
			 * invalid packets.
			 */
			if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
				BT_DBG("Invalid/Ignore - after SREJ");
				return L2CAP_TXSEQ_INVALID_IGNORE;
			} else {
				BT_DBG("Invalid - in window after SREJ sent");
				return L2CAP_TXSEQ_INVALID;
			}
		}

		if (chan->srej_list.head == txseq) {
			BT_DBG("Expected SREJ");
			return L2CAP_TXSEQ_EXPECTED_SREJ;
		}

		if (l2cap_ertm_seq_in_queue(&chan->srej_q, txseq)) {
			BT_DBG("Duplicate SREJ - txseq already stored");
			return L2CAP_TXSEQ_DUPLICATE_SREJ;
		}

		if (l2cap_seq_list_contains(&chan->srej_list, txseq)) {
			BT_DBG("Unexpected SREJ - not requested");
			return L2CAP_TXSEQ_UNEXPECTED_SREJ;
		}
	}

	if (chan->expected_tx_seq == txseq) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		} else {
			BT_DBG("Expected");
			return L2CAP_TXSEQ_EXPECTED;
		}
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) <
	    __seq_offset(chan, chan->expected_tx_seq, chan->last_acked_seq)) {
		BT_DBG("Duplicate - expected_tx_seq later than txseq");
		return L2CAP_TXSEQ_DUPLICATE;
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) >= chan->tx_win) {
		/* A source of invalid packets is a "double poll" condition,
		 * where delays cause us to send multiple poll packets.  If
		 * the remote stack receives and processes both polls,
		 * sequence numbers can wrap around in such a way that a
		 * resent frame has a sequence number that looks like new data
		 * with a sequence gap.  This would trigger an erroneous SREJ
		 * request.
		 *
		 * Fortunately, this is impossible with a tx window that's
		 * less than half of the maximum sequence number, which allows
		 * invalid frames to be safely ignored.
		 *
		 * With tx window sizes greater than half of the tx window
		 * maximum, the frame is invalid and cannot be ignored.  This
		 * causes a disconnect.
		 */

		if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
			BT_DBG("Invalid/Ignore - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID_IGNORE;
		} else {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		}
	} else {
		BT_DBG("Unexpected - txseq indicates missing frames");
		return L2CAP_TXSEQ_UNEXPECTED;
	}
}

static int l2cap_rx_state_recv(struct l2cap_chan *chan,
			       struct l2cap_ctrl *control,
			       struct sk_buff *skb, u8 event)
{
	int err = 0;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, control->txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			l2cap_pass_to_tx(chan, control);

			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding expected seq %d",
				       control->txseq);
				break;
			}

			chan->expected_tx_seq = __next_seq(chan,
							   control->txseq);

			chan->buffer_seq = chan->expected_tx_seq;
			skb_in_use = 1;

			err = l2cap_reassemble_sdu(chan, skb, control);
			if (err)
				break;

			if (control->final) {
				if (!test_and_clear_bit(CONN_REJ_ACT,
							&chan->conn_state)) {
					control->final = 0;
					l2cap_retransmit_all(chan, control);
					l2cap_ertm_send(chan);
				}
			}

			if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state))
				l2cap_send_ack(chan);
			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			l2cap_pass_to_tx(chan, control);

			/* Can't issue SREJ frames in the local busy state.
			 * Drop this frame, it will be seen as missing
			 * when local busy is exited.
			 */
			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding unexpected seq %d",
				       control->txseq);
				break;
			}

			/* There was a gap in the sequence, so an SREJ
			 * must be sent for each missing frame.  The
			 * current frame is stored for later use.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			clear_bit(CONN_SREJ_ACT, &chan->conn_state);
			l2cap_seq_list_clear(&chan->srej_list);
			l2cap_send_srej(chan, control->txseq);

			chan->rx_state = L2CAP_RX_STATE_SREJ_SENT;
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state) &&
			    !__chan_is_moving(chan)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			l2cap_send_i_or_rr_or_rnr(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_ertm_send(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control && control->poll) {
			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_rr_or_rnr(chan, 0);
		}
		__clear_retrans_timer(chan);
		l2cap_seq_list_clear(&chan->retrans_list);
		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	default:
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_rx_state_srej_sent(struct l2cap_chan *chan,
				    struct l2cap_ctrl *control,
				    struct sk_buff *skb, u8 event)
{
	int err = 0;
	u16 txseq = control->txseq;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			/* Keep frame for reassembly later */
			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			chan->expected_tx_seq = __next_seq(chan, txseq);
			break;
		case L2CAP_TXSEQ_EXPECTED_SREJ:
			l2cap_seq_list_pop(&chan->srej_list);

			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			err = l2cap_rx_queued_iframes(chan);
			if (err)
				break;

			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			/* Got a frame that can't be reassembled yet.
			 * Save it for later, and send SREJs to cover
			 * the missing frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_UNEXPECTED_SREJ:
			/* This frame was requested with an SREJ, but
			 * some expected retransmitted frames are
			 * missing.  Request retransmission of missing
			 * SREJ'd frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej_list(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_DUPLICATE_SREJ:
			/* We've already queued this frame.  Drop this copy. */
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			/* Expecting a later sequence number, so this frame
			 * was already received.  Ignore it completely.
			 */
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT,
						&chan->conn_state)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames) {
				__set_retrans_timer(chan);
			}

			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_srej_tail(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_send_ack(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control->poll) {
			l2cap_send_srej_tail(chan);
		} else {
			struct l2cap_ctrl rr_control;
			memset(&rr_control, 0, sizeof(rr_control));
			rr_control.sframe = 1;
			rr_control.super = L2CAP_SUPER_RR;
			rr_control.reqseq = chan->buffer_seq;
			l2cap_send_sframe(chan, &rr_control);
		}

		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_finish_move(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	chan->rx_state = L2CAP_RX_STATE_RECV;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	return l2cap_resegment(chan);
}

static int l2cap_rx_state_wait_p(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	if (!control->poll)
		return -EPROTO;

	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	err = l2cap_finish_move(chan);
	if (err)
		return err;

	set_bit(CONN_SEND_FBIT, &chan->conn_state);
	l2cap_send_i_or_rr_or_rnr(chan);

	if (event == L2CAP_EV_RECV_IFRAME)
		return -EPROTO;

	return l2cap_rx_state_recv(chan, control, NULL, event);
}

static int l2cap_rx_state_wait_f(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	if (!control->final)
		return -EPROTO;

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	chan->rx_state = L2CAP_RX_STATE_RECV;
	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	err = l2cap_resegment(chan);

	if (!err)
		err = l2cap_rx_state_recv(chan, control, skb, event);

	return err;
}

static bool __valid_reqseq(struct l2cap_chan *chan, u16 reqseq)
{
	/* Make sure reqseq is for a packet that has been sent but not acked */
	u16 unacked;

	unacked = __seq_offset(chan, chan->next_tx_seq, chan->expected_ack_seq);
	return __seq_offset(chan, chan->next_tx_seq, reqseq) <= unacked;
}

static int l2cap_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
		    struct sk_buff *skb, u8 event)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d, state %d", chan,
	       control, skb, event, chan->rx_state);

	if (__valid_reqseq(chan, control->reqseq)) {
		switch (chan->rx_state) {
		case L2CAP_RX_STATE_RECV:
			err = l2cap_rx_state_recv(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_SREJ_SENT:
			err = l2cap_rx_state_srej_sent(chan, control, skb,
						       event);
			break;
		case L2CAP_RX_STATE_WAIT_P:
			err = l2cap_rx_state_wait_p(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_WAIT_F:
			err = l2cap_rx_state_wait_f(chan, control, skb, event);
			break;
		default:
			/* shut it down */
			break;
		}
	} else {
		BT_DBG("Invalid reqseq %d (next_tx_seq %d, expected_ack_seq %d",
		       control->reqseq, chan->next_tx_seq,
		       chan->expected_ack_seq);
		l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return err;
}

static int l2cap_stream_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
			   struct sk_buff *skb)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, state %d", chan, control, skb,
	       chan->rx_state);

	if (l2cap_classify_txseq(chan, control->txseq) ==
	    L2CAP_TXSEQ_EXPECTED) {
		l2cap_pass_to_tx(chan, control);

		BT_DBG("buffer_seq %d->%d", chan->buffer_seq,
		       __next_seq(chan, chan->buffer_seq));

		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);

		l2cap_reassemble_sdu(chan, skb, control);
	} else {
		if (chan->sdu) {
			kfree_skb(chan->sdu);
			chan->sdu = NULL;
		}
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;

		if (skb) {
			BT_DBG("Freeing %p", skb);
			kfree_skb(skb);
		}
	}

	chan->last_acked_seq = control->txseq;
	chan->expected_tx_seq = __next_seq(chan, control->txseq);

	return err;
}

static int l2cap_data_rcv(struct l2cap_chan *chan, struct sk_buff *skb)
{
	struct l2cap_ctrl *control = &bt_cb(skb)->control;
	u16 len;
	u8 event;

	__unpack_control(chan, skb);

	len = skb->len;

	/*
	 * We can just drop the corrupted I-frame here.
	 * Receiver will miss it and start proper recovery
	 * procedures and ask for retransmission.
	 */
	if (l2cap_check_fcs(chan, skb))
		goto drop;

	if (!control->sframe && control->sar == L2CAP_SAR_START)
		len -= L2CAP_SDULEN_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16)
		len -= L2CAP_FCS_SIZE;

	if (len > chan->mps) {
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto drop;
	}

	if (!control->sframe) {
		int err;

		BT_DBG("iframe sar %d, reqseq %d, final %d, txseq %d",
		       control->sar, control->reqseq, control->final,
		       control->txseq);

		/* Validate F-bit - F=0 always valid, F=1 only
		 * valid in TX WAIT_F
		 */
		if (control->final && chan->tx_state != L2CAP_TX_STATE_WAIT_F)
			goto drop;

		if (chan->mode != L2CAP_MODE_STREAMING) {
			event = L2CAP_EV_RECV_IFRAME;
			err = l2cap_rx(chan, control, skb, event);
		} else {
			err = l2cap_stream_rx(chan, control, skb);
		}

		if (err)
			l2cap_send_disconn_req(chan, ECONNRESET);
	} else {
		const u8 rx_func_to_event[4] = {
			L2CAP_EV_RECV_RR, L2CAP_EV_RECV_REJ,
			L2CAP_EV_RECV_RNR, L2CAP_EV_RECV_SREJ
		};

		/* Only I-frames are expected in streaming mode */
		if (chan->mode == L2CAP_MODE_STREAMING)
			goto drop;

		BT_DBG("sframe reqseq %d, final %d, poll %d, super %d",
		       control->reqseq, control->final, control->poll,
		       control->super);

		if (len != 0) {
			BT_ERR("Trailing bytes: %d in sframe", len);
			l2cap_send_disconn_req(chan, ECONNRESET);
			goto drop;
		}

		/* Validate F and P bits */
		if (control->final && (control->poll ||
				       chan->tx_state != L2CAP_TX_STATE_WAIT_F))
			goto drop;

		event = rx_func_to_event[control->super];
		if (l2cap_rx(chan, control, skb, event))
			l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return 0;

drop:
	kfree_skb(skb);
	return 0;
}

static void l2cap_data_channel(struct l2cap_conn *conn, u16 cid,
			       struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_scid(conn, cid);
	if (!chan) {
		if (cid == L2CAP_CID_A2MP) {
			chan = a2mp_channel_create(conn, skb);
			if (!chan) {
				kfree_skb(skb);
				return;
			}

			l2cap_chan_lock(chan);
		} else {
			BT_DBG("unknown cid 0x%4.4x", cid);
			/* Drop packet and return */
			kfree_skb(skb);
			return;
		}
	}

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_CONNECTED)
		goto drop;

	switch (chan->mode) {
	case L2CAP_MODE_BASIC:
		/* If socket recv buffers overflows we drop data here
		 * which is *bad* because L2CAP has to be reliable.
		 * But we don't have any other choice. L2CAP doesn't
		 * provide flow control mechanism. */

		if (chan->imtu < skb->len)
			goto drop;

		if (!chan->ops->recv(chan, skb))
			goto done;
		break;

	case L2CAP_MODE_ERTM:
	case L2CAP_MODE_STREAMING:
		l2cap_data_rcv(chan, skb);
		goto done;

	default:
		BT_DBG("chan %p: bad mode 0x%2.2x", chan, chan->mode);
		break;
	}

drop:
	kfree_skb(skb);

done:
	l2cap_chan_unlock(chan);
}

static void l2cap_conless_channel(struct l2cap_conn *conn, __le16 psm,
				  struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_psm(0, psm, conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_att_channel(struct l2cap_conn *conn,
			      struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_scid(0, L2CAP_CID_LE_DATA,
					 conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_recv_frame(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct l2cap_hdr *lh = (void *) skb->data;
	u16 cid, len;
	__le16 psm;

	skb_pull(skb, L2CAP_HDR_SIZE);
	cid = __le16_to_cpu(lh->cid);
	len = __le16_to_cpu(lh->len);

	if (len != skb->len) {
		kfree_skb(skb);
		return;
	}

	BT_DBG("len %d, cid 0x%4.4x", len, cid);

	switch (cid) {
	case L2CAP_CID_LE_SIGNALING:
	case L2CAP_CID_SIGNALING:
		l2cap_sig_channel(conn, skb);
		break;

	case L2CAP_CID_CONN_LESS:
		psm = get_unaligned((__le16 *) skb->data);
		skb_pull(skb, L2CAP_PSMLEN_SIZE);
		l2cap_conless_channel(conn, psm, skb);
		break;

	case L2CAP_CID_LE_DATA:
		l2cap_att_channel(conn, skb);
		break;

	case L2CAP_CID_SMP:
		if (smp_sig_channel(conn, skb))
			l2cap_conn_del(conn->hcon, EACCES);
		break;

	default:
		l2cap_data_channel(conn, cid, skb);
		break;
	}
}

/* ---- L2CAP interface with lower layer (HCI) ---- */

int l2cap_connect_ind(struct hci_dev *hdev, bdaddr_t *bdaddr)
{
	int exact = 0, lm1 = 0, lm2 = 0;
	struct l2cap_chan *c;

	BT_DBG("hdev %s, bdaddr %pMR", hdev->name, bdaddr);

	/* Find listening sockets and check their link_mode */
	read_lock(&chan_list_lock);
	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		if (c->state != BT_LISTEN)
			continue;

		if (!bacmp(&bt_sk(sk)->src, &hdev->bdaddr)) {
			lm1 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm1 |= HCI_LM_MASTER;
			exact++;
		} else if (!bacmp(&bt_sk(sk)->src, BDADDR_ANY)) {
			lm2 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm2 |= HCI_LM_MASTER;
		}
	}
	read_unlock(&chan_list_lock);

	return exact ? lm1 : lm2;
}

void l2cap_connect_cfm(struct hci_conn *hcon, u8 status)
{
	struct l2cap_conn *conn;

	BT_DBG("hcon %p bdaddr %pMR status %d", hcon, &hcon->dst, status);

	if (!status) {
		conn = l2cap_conn_add(hcon);
		if (conn)
			l2cap_conn_ready(conn);
	} else {
		l2cap_conn_del(hcon, bt_to_errno(status));
	}
}

int l2cap_disconn_ind(struct hci_conn *hcon)
{
	struct l2cap_conn *conn = hcon->l2cap_data;

	BT_DBG("hcon %p", hcon);

	if (!conn)
		return HCI_ERROR_REMOTE_USER_TERM;
	return conn->disc_reason;
}

void l2cap_disconn_cfm(struct hci_conn *hcon, u8 reason)
{
	BT_DBG("hcon %p reason %d", hcon, reason);

	l2cap_conn_del(hcon, bt_to_errno(reason));
}

static inline void l2cap_check_encryption(struct l2cap_chan *chan, u8 encrypt)
{
	if (chan->chan_type != L2CAP_CHAN_CONN_ORIENTED)
		return;

	if (encrypt == 0x00) {
		if (chan->sec_level == BT_SECURITY_MEDIUM) {
			__set_chan_timer(chan, L2CAP_ENC_TIMEOUT);
		} else if (chan->sec_level == BT_SECURITY_HIGH)
			l2cap_chan_close(chan, ECONNREFUSED);
	} else {
		if (chan->sec_level == BT_SECURITY_MEDIUM)
			__clear_chan_timer(chan);
	}
}

int l2cap_security_cfm(struct hci_conn *hcon, u8 status, u8 encrypt)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_chan *chan;

	if (!conn)
		return 0;

	BT_DBG("conn %p status 0x%2.2x encrypt %u", conn, status, encrypt);

	if (hcon->type == LE_LINK) {
		if (!status && encrypt)
			smp_distribute_keys(conn, 0);
		cancel_delayed_work(&conn->security_timer);
	}

	mutex_lock(&conn->chan_lock);

	list_for_each_entry(chan, &conn->chan_l, list) {
		l2cap_chan_lock(chan);

		BT_DBG("chan %p scid 0x%4.4x state %s", chan, chan->scid,
		       state_to_string(chan->state));

		if (chan->chan_type == L2CAP_CHAN_CONN_FIX_A2MP) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->scid == L2CAP_CID_LE_DATA) {
			if (!status && encrypt) {
				chan->sec_level = hcon->sec_level;
				l2cap_chan_ready(chan);
			}

			l2cap_chan_unlock(chan);
			continue;
		}

		if (!__l2cap_no_conn_pending(chan)) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (!status && (chan->state == BT_CONNECTED ||
				chan->state == BT_CONFIG)) {
			struct sock *sk = chan->sk;

			clear_bit(BT_SK_SUSPEND, &bt_sk(sk)->flags);
			sk->sk_state_change(sk);

			l2cap_check_encryption(chan, encrypt);
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->state == BT_CONNECT) {
			if (!status) {
				l2cap_start_connection(chan);
			} else {
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
			}
		} else if (chan->state == BT_CONNECT2) {
			struct sock *sk = chan->sk;
			struct l2cap_conn_rsp rsp;
			__u16 res, stat;

			lock_sock(sk);

			if (!status) {
				if (test_bit(BT_SK_DEFER_SETUP,
					     &bt_sk(sk)->flags)) {
					res = L2CAP_CR_PEND;
					stat = L2CAP_CS_AUTHOR_PEND;
					chan->ops->defer(chan);
				} else {
					__l2cap_state_change(chan, BT_CONFIG);
					res = L2CAP_CR_SUCCESS;
					stat = L2CAP_CS_NO_INFO;
				}
			} else {
				__l2cap_state_change(chan, BT_DISCONN);
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
				res = L2CAP_CR_SEC_BLOCK;
				stat = L2CAP_CS_NO_INFO;
			}

			release_sock(sk);

			rsp.scid   = cpu_to_le16(chan->dcid);
			rsp.dcid   = cpu_to_le16(chan->scid);
			rsp.result = cpu_to_le16(res);
			rsp.status = cpu_to_le16(stat);
			l2cap_send_cmd(conn, chan->ident, L2CAP_CONN_RSP,
				       sizeof(rsp), &rsp);

			if (!test_bit(CONF_REQ_SENT, &chan->conf_state) &&
			    res == L2CAP_CR_SUCCESS) {
				char buf[128];
				set_bit(CONF_REQ_SENT, &chan->conf_state);
				l2cap_send_cmd(conn, l2cap_get_ident(conn),
					       L2CAP_CONF_REQ,
					       l2cap_build_conf_req(chan, buf),
					       buf);
				chan->num_conf_req++;
			}
		}

		l2cap_chan_unlock(chan);
	}

	mutex_unlock(&conn->chan_lock);

	return 0;
}

int l2cap_recv_acldata(struct hci_conn *hcon, struct sk_buff *skb, u16 flags)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_hdr *hdr;
	int len;

	/* For AMP controller do not create l2cap conn */
	if (!conn && hcon->hdev->dev_type != HCI_BREDR)
		goto drop;

	if (!conn)
		conn = l2cap_conn_add(hcon);

	if (!conn)
		goto drop;

	BT_DBG("conn %p len %d flags 0x%x", conn, skb->len, flags);

	switch (flags) {
	case ACL_START:
	case ACL_START_NO_FLUSH:
	case ACL_COMPLETE:
		if (conn->rx_len) {
			BT_ERR("Unexpected start frame (len %d)", skb->len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
		}

		/* Start fragment always begin with Basic L2CAP header */
		if (skb->len < L2CAP_HDR_SIZE) {
			BT_ERR("Frame is too short (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		hdr = (struct l2cap_hdr *) skb->data;
		len = __le16_to_cpu(hdr->len) + L2CAP_HDR_SIZE;

		if (len == skb->len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, skb);
			return 0;
		}

		BT_DBG("Start: total len %d, frag len %d", len, skb->len);

		if (skb->len > len) {
			BT_ERR("Frame is too long (len %d, expected len %d)",
			       skb->len, len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		/* Allocate skb for the complete frame (with header) */
		conn->rx_skb = bt_skb_alloc(len, GFP_KERNEL);
		if (!conn->rx_skb)
			goto drop;

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len = len - skb->len;
		break;

	case ACL_CONT:
		BT_DBG("Cont: frag len %d (expecting %d)", skb->len, conn->rx_len);

		if (!conn->rx_len) {
			BT_ERR("Unexpected continuation frame (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		if (skb->len > conn->rx_len) {
			BT_ERR("Fragment is too long (len %d, expected %d)",
			       skb->len, conn->rx_len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len -= skb->len;

		if (!conn->rx_len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, conn->rx_skb);
			conn->rx_skb = NULL;
		}
		break;
	}

drop:
	kfree_skb(skb);
	return 0;
}

static int l2cap_debugfs_show(struct seq_file *f, void *p)
{
	struct l2cap_chan *c;

	read_lock(&chan_list_lock);

	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		seq_printf(f, "%pMR %pMR %d %d 0x%4.4x 0x%4.4x %d %d %d %d\n",
			   &bt_sk(sk)->src, &bt_sk(sk)->dst,
			   c->state, __le16_to_cpu(c->psm),
			   c->scid, c->dcid, c->imtu, c->omtu,
			   c->sec_level, c->mode);
	}

	read_unlock(&chan_list_lock);

	return 0;
}

static int l2cap_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, l2cap_debugfs_show, inode->i_private);
}

static const struct file_operations l2cap_debugfs_fops = {
	.open		= l2cap_debugfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *l2cap_debugfs;

int __init l2cap_init(void)
{
	int err;

	err = l2cap_init_sockets();
	if (err < 0)
		return err;

	if (bt_debugfs) {
		l2cap_debugfs = debugfs_create_file("l2cap", 0444, bt_debugfs,
						    NULL, &l2cap_debugfs_fops);
		if (!l2cap_debugfs)
			BT_ERR("Failed to create L2CAP debug file");
	}

	return 0;
}

void l2cap_exit(void)
{
	debugfs_remove(l2cap_debugfs);
	l2cap_cleanup_sockets();
}

module_param(disable_ertm, bool, 0644);
MODULE_PARM_DESC(disable_ertm, "Disable enhanced retransmission mode");
	if (test_bit(CONF_OUTPUT_DONE, &chan->conf_state)) {
		set_default_fcs(chan);

		if (chan->mode == L2CAP_MODE_ERTM ||
		    chan->mode == L2CAP_MODE_STREAMING)
			err = l2cap_ertm_init(chan);

		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}

done:
	l2cap_chan_unlock(chan);
	return err;
}

static inline int l2cap_disconnect_req(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_req *req = (struct l2cap_disconn_req *) data;
	struct l2cap_disconn_rsp rsp;
	u16 dcid, scid;
	struct l2cap_chan *chan;
	struct sock *sk;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	scid = __le16_to_cpu(req->scid);
	dcid = __le16_to_cpu(req->dcid);

	BT_DBG("scid 0x%4.4x dcid 0x%4.4x", scid, dcid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, dcid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	sk = chan->sk;

	rsp.dcid = cpu_to_le16(chan->scid);
	rsp.scid = cpu_to_le16(chan->dcid);
	l2cap_send_cmd(conn, cmd->ident, L2CAP_DISCONN_RSP, sizeof(rsp), &rsp);

	lock_sock(sk);
	sk->sk_shutdown = SHUTDOWN_MASK;
	release_sock(sk);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, ECONNRESET);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	sk = chan->sk;

	rsp.dcid = cpu_to_le16(chan->scid);
	rsp.scid = cpu_to_le16(chan->dcid);
	l2cap_send_cmd(conn, cmd->ident, L2CAP_DISCONN_RSP, sizeof(rsp), &rsp);

	lock_sock(sk);
	sk->sk_shutdown = SHUTDOWN_MASK;
	release_sock(sk);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, ECONNRESET);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_disconnect_rsp(struct l2cap_conn *conn,
				       struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				       u8 *data)
{
	struct l2cap_disconn_rsp *rsp = (struct l2cap_disconn_rsp *) data;
	u16 dcid, scid;
	struct l2cap_chan *chan;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	scid = __le16_to_cpu(rsp->scid);
	dcid = __le16_to_cpu(rsp->dcid);

	BT_DBG("dcid 0x%4.4x scid 0x%4.4x", dcid, scid);

	mutex_lock(&conn->chan_lock);

	chan = __l2cap_get_chan_by_scid(conn, scid);
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, 0);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}
	if (!chan) {
		mutex_unlock(&conn->chan_lock);
		return 0;
	}

	l2cap_chan_lock(chan);

	l2cap_chan_hold(chan);
	l2cap_chan_del(chan, 0);

	l2cap_chan_unlock(chan);

	chan->ops->close(chan);
	l2cap_chan_put(chan);

	mutex_unlock(&conn->chan_lock);

	return 0;
}

static inline int l2cap_information_req(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_req *req = (struct l2cap_info_req *) data;
	u16 type;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	type = __le16_to_cpu(req->type);

	BT_DBG("type 0x%4.4x", type);

	if (type == L2CAP_IT_FEAT_MASK) {
		u8 buf[8];
		u32 feat_mask = l2cap_feat_mask;
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;
		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FEAT_MASK);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		if (!disable_ertm)
			feat_mask |= L2CAP_FEAT_ERTM | L2CAP_FEAT_STREAMING
				| L2CAP_FEAT_FCS;
		if (enable_hs)
			feat_mask |= L2CAP_FEAT_EXT_FLOW
				| L2CAP_FEAT_EXT_WINDOW;

		put_unaligned_le32(feat_mask, rsp->data);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else if (type == L2CAP_IT_FIXED_CHAN) {
		u8 buf[12];
		struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) buf;

		if (enable_hs)
			l2cap_fixed_chan[0] |= L2CAP_FC_A2MP;
		else
			l2cap_fixed_chan[0] &= ~L2CAP_FC_A2MP;

		rsp->type   = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);
		rsp->result = __constant_cpu_to_le16(L2CAP_IR_SUCCESS);
		memcpy(rsp->data, l2cap_fixed_chan, sizeof(l2cap_fixed_chan));
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(buf),
			       buf);
	} else {
		struct l2cap_info_rsp rsp;
		rsp.type   = cpu_to_le16(type);
		rsp.result = __constant_cpu_to_le16(L2CAP_IR_NOTSUPP);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(rsp),
			       &rsp);
	}

	return 0;
}

static inline int l2cap_information_rsp(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) data;
	u16 type, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	type   = __le16_to_cpu(rsp->type);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("type 0x%4.4x result 0x%2.2x", type, result);

	/* L2CAP Info req/rsp are unbound to channels, add extra checks */
	if (cmd->ident != conn->info_ident ||
	    conn->info_state & L2CAP_INFO_FEAT_MASK_REQ_DONE)
		return 0;

	cancel_delayed_work(&conn->info_timer);

	if (result != L2CAP_IR_SUCCESS) {
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);

		return 0;
	}

	switch (type) {
	case L2CAP_IT_FEAT_MASK:
		conn->feat_mask = get_unaligned_le32(rsp->data);

		if (conn->feat_mask & L2CAP_FEAT_FIXED_CHAN) {
			struct l2cap_info_req req;
			req.type = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);

			conn->info_ident = l2cap_get_ident(conn);

			l2cap_send_cmd(conn, conn->info_ident,
				       L2CAP_INFO_REQ, sizeof(req), &req);
		} else {
			conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
			conn->info_ident = 0;

			l2cap_conn_start(conn);
		}
		break;

	case L2CAP_IT_FIXED_CHAN:
		conn->fixed_chan_mask = rsp->data[0];
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);
		break;
	}

	return 0;
}

static int l2cap_create_channel_req(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd,
				    u16 cmd_len, void *data)
{
	struct l2cap_create_chan_req *req = data;
	struct l2cap_create_chan_rsp rsp;
	struct l2cap_chan *chan;
	struct hci_dev *hdev;
	u16 psm, scid;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	if (!enable_hs)
		return -EINVAL;

	psm = le16_to_cpu(req->psm);
	scid = le16_to_cpu(req->scid);

	BT_DBG("psm 0x%2.2x, scid 0x%4.4x, amp_id %d", psm, scid, req->amp_id);

	/* For controller id 0 make BR/EDR connection */
	if (req->amp_id == HCI_BREDR_ID) {
		l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			      req->amp_id);
		return 0;
	}

	/* Validate AMP controller id */
	hdev = hci_dev_get(req->amp_id);
	if (!hdev)
		goto error;

	if (hdev->dev_type != HCI_AMP || !test_bit(HCI_UP, &hdev->flags)) {
		hci_dev_put(hdev);
		goto error;
	}

	chan = l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			     req->amp_id);
	if (chan) {
		struct amp_mgr *mgr = conn->hcon->amp_mgr;
		struct hci_conn *hs_hcon;

		hs_hcon = hci_conn_hash_lookup_ba(hdev, AMP_LINK, conn->dst);
		if (!hs_hcon) {
			hci_dev_put(hdev);
			return -EFAULT;
		}

		BT_DBG("mgr %p bredr_chan %p hs_hcon %p", mgr, chan, hs_hcon);

		mgr->bredr_chan = chan;
		chan->hs_hcon = hs_hcon;
		chan->fcs = L2CAP_FCS_NONE;
		conn->mtu = hdev->block_mtu;
	}

	hci_dev_put(hdev);

	return 0;

error:
	rsp.dcid = 0;
	rsp.scid = cpu_to_le16(scid);
	rsp.result = __constant_cpu_to_le16(L2CAP_CR_BAD_AMP);
	rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CREATE_CHAN_RSP,
		       sizeof(rsp), &rsp);

	return -EFAULT;
}

static void l2cap_send_move_chan_req(struct l2cap_chan *chan, u8 dest_amp_id)
{
	struct l2cap_move_chan_req req;
	u8 ident;

	BT_DBG("chan %p, dest_amp_id %d", chan, dest_amp_id);

	ident = l2cap_get_ident(chan->conn);
	chan->ident = ident;

	req.icid = cpu_to_le16(chan->scid);
	req.dest_amp_id = dest_amp_id;

	l2cap_send_cmd(chan->conn, ident, L2CAP_MOVE_CHAN_REQ, sizeof(req),
		       &req);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_rsp(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_rsp rsp;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	rsp.icid = cpu_to_le16(chan->dcid);
	rsp.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_RSP,
		       sizeof(rsp), &rsp);
}

static void l2cap_send_move_chan_cfm(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	chan->ident = l2cap_get_ident(chan->conn);

	cfm.icid = cpu_to_le16(chan->scid);
	cfm.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_cfm_icid(struct l2cap_conn *conn, u16 icid)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("conn %p, icid 0x%4.4x", conn, icid);

	cfm.icid = cpu_to_le16(icid);
	cfm.result = __constant_cpu_to_le16(L2CAP_MC_UNCONFIRMED);

	l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);
}

static void l2cap_send_move_chan_cfm_rsp(struct l2cap_conn *conn, u8 ident,
					 u16 icid)
{
	struct l2cap_move_chan_cfm_rsp rsp;

	BT_DBG("icid 0x%4.4x", icid);

	rsp.icid = cpu_to_le16(icid);
	l2cap_send_cmd(conn, ident, L2CAP_MOVE_CHAN_CFM_RSP, sizeof(rsp), &rsp);
}

static void __release_logical_link(struct l2cap_chan *chan)
{
	chan->hs_hchan = NULL;
	chan->hs_hcon = NULL;

	/* Placeholder - release the logical link */
}

static void l2cap_logical_fail(struct l2cap_chan *chan)
{
	/* Logical link setup failed */
	if (chan->state != BT_CONNECTED) {
		/* Create channel failure, disconnect */
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	switch (chan->move_role) {
	case L2CAP_MOVE_ROLE_RESPONDER:
		l2cap_move_done(chan);
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_SUPP);
		break;
	case L2CAP_MOVE_ROLE_INITIATOR:
		if (chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_COMP ||
		    chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_CFM) {
			/* Remote has only sent pending or
			 * success responses, clean up
			 */
			l2cap_move_done(chan);
		}

		/* Other amp move states imply that the move
		 * has already aborted
		 */
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
		break;
	}
}

static void l2cap_logical_finish_create(struct l2cap_chan *chan,
					struct hci_chan *hchan)
{
	struct l2cap_conf_rsp rsp;

	chan->hs_hchan = hchan;
	chan->hs_hcon->l2cap_data = chan->conn;

	l2cap_send_efs_conf_rsp(chan, &rsp, chan->ident, 0);

	if (test_bit(CONF_INPUT_DONE, &chan->conf_state)) {
		int err;

		set_default_fcs(chan);

		err = l2cap_ertm_init(chan);
		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}
}

static void l2cap_logical_finish_move(struct l2cap_chan *chan,
				      struct hci_chan *hchan)
{
	chan->hs_hcon = hchan->conn;
	chan->hs_hcon->l2cap_data = chan->conn;

	BT_DBG("move_state %d", chan->move_state);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent after a success
		 * response is received
		 */
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		break;
	case L2CAP_MOVE_WAIT_LOGICAL_CFM:
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);
		}
		break;
	default:
		/* Move was not in expected state, free the channel */
		__release_logical_link(chan);

		chan->move_state = L2CAP_MOVE_STABLE;
	}
}

/* Call with chan locked */
void l2cap_logical_cfm(struct l2cap_chan *chan, struct hci_chan *hchan,
		       u8 status)
{
	BT_DBG("chan %p, hchan %p, status %d", chan, hchan, status);

	if (status) {
		l2cap_logical_fail(chan);
		__release_logical_link(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		/* Ignore logical link if channel is on BR/EDR */
		if (chan->local_amp_id)
			l2cap_logical_finish_create(chan, hchan);
	} else {
		l2cap_logical_finish_move(chan, hchan);
	}
}

void l2cap_move_start(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	if (chan->local_amp_id == HCI_BREDR_ID) {
		if (chan->chan_policy != BT_CHANNEL_POLICY_AMP_PREFERRED)
			return;
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - start physical link setup */
	} else {
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		chan->move_id = 0;
		l2cap_move_setup(chan);
		l2cap_send_move_chan_req(chan, 0);
	}
}

static void l2cap_do_create(struct l2cap_chan *chan, int result,
			    u8 local_amp_id, u8 remote_amp_id)
{
	BT_DBG("chan %p state %s %u -> %u", chan, state_to_string(chan->state),
	       local_amp_id, remote_amp_id);

	chan->fcs = L2CAP_FCS_NONE;

	/* Outgoing channel on AMP */
	if (chan->state == BT_CONNECT) {
		if (result == L2CAP_CR_SUCCESS) {
			chan->local_amp_id = local_amp_id;
			l2cap_send_create_chan_req(chan, remote_amp_id);
		} else {
			/* Revert to BR/EDR connect */
			l2cap_send_conn_req(chan);
		}

		return;
	}

	/* Incoming channel on AMP */
	if (__l2cap_no_conn_pending(chan)) {
		struct l2cap_conn_rsp rsp;
		char buf[128];
		rsp.scid = cpu_to_le16(chan->dcid);
		rsp.dcid = cpu_to_le16(chan->scid);

		if (result == L2CAP_CR_SUCCESS) {
			/* Send successful response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_SUCCESS);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		} else {
			/* Send negative response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_NO_MEM);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		}

		l2cap_send_cmd(chan->conn, chan->ident, L2CAP_CREATE_CHAN_RSP,
			       sizeof(rsp), &rsp);

		if (result == L2CAP_CR_SUCCESS) {
			__l2cap_state_change(chan, BT_CONFIG);
			set_bit(CONF_REQ_SENT, &chan->conf_state);
			l2cap_send_cmd(chan->conn, l2cap_get_ident(chan->conn),
				       L2CAP_CONF_REQ,
				       l2cap_build_conf_req(chan, buf), buf);
			chan->num_conf_req++;
		}
	}
}

static void l2cap_do_move_initiate(struct l2cap_chan *chan, u8 local_amp_id,
				   u8 remote_amp_id)
{
	l2cap_move_setup(chan);
	chan->move_id = local_amp_id;
	chan->move_state = L2CAP_MOVE_WAIT_RSP;

	l2cap_send_move_chan_req(chan, remote_amp_id);
}

static void l2cap_do_move_respond(struct l2cap_chan *chan, int result)
{
	struct hci_chan *hchan = NULL;

	/* Placeholder - get hci_chan for logical link */

	if (hchan) {
		if (hchan->state == BT_CONNECTED) {
			/* Logical link is ready to go */
			chan->hs_hcon = hchan->conn;
			chan->hs_hcon->l2cap_data = chan->conn;
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);

			l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		} else {
			/* Wait for logical link to be ready */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		}
	} else {
		/* Logical link not available */
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_ALLOWED);
	}
}

static void l2cap_do_move_cancel(struct l2cap_chan *chan, int result)
{
	if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
		u8 rsp_result;
		if (result == -EINVAL)
			rsp_result = L2CAP_MR_BAD_ID;
		else
			rsp_result = L2CAP_MR_NOT_ALLOWED;

		l2cap_send_move_chan_rsp(chan, rsp_result);
	}

	chan->move_role = L2CAP_MOVE_ROLE_NONE;
	chan->move_state = L2CAP_MOVE_STABLE;

	/* Restart data transmission */
	l2cap_ertm_send(chan);
}

/* Invoke with locked chan */
void __l2cap_physical_cfm(struct l2cap_chan *chan, int result)
{
	u8 local_amp_id = chan->local_amp_id;
	u8 remote_amp_id = chan->remote_amp_id;

	BT_DBG("chan %p, result %d, local_amp_id %d, remote_amp_id %d",
	       chan, result, local_amp_id, remote_amp_id);

	if (chan->state == BT_DISCONN || chan->state == BT_CLOSED) {
		l2cap_chan_unlock(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		l2cap_do_create(chan, result, local_amp_id, remote_amp_id);
	} else if (result != L2CAP_MR_SUCCESS) {
		l2cap_do_move_cancel(chan, result);
	} else {
		switch (chan->move_role) {
		case L2CAP_MOVE_ROLE_INITIATOR:
			l2cap_do_move_initiate(chan, local_amp_id,
					       remote_amp_id);
			break;
		case L2CAP_MOVE_ROLE_RESPONDER:
			l2cap_do_move_respond(chan, result);
			break;
		default:
			l2cap_do_move_cancel(chan, result);
			break;
		}
	}
}

static inline int l2cap_move_channel_req(struct l2cap_conn *conn,
					 struct l2cap_cmd_hdr *cmd,
					 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_req *req = data;
	struct l2cap_move_chan_rsp rsp;
	struct l2cap_chan *chan;
	u16 icid = 0;
	u16 result = L2CAP_MR_NOT_ALLOWED;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	icid = le16_to_cpu(req->icid);

	BT_DBG("icid 0x%4.4x, dest_amp_id %d", icid, req->dest_amp_id);

	if (!enable_hs)
		return -EINVAL;

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		rsp.icid = cpu_to_le16(icid);
		rsp.result = __constant_cpu_to_le16(L2CAP_MR_NOT_ALLOWED);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_MOVE_CHAN_RSP,
			       sizeof(rsp), &rsp);
		return 0;
	}

	chan->ident = cmd->ident;

	if (chan->scid < L2CAP_CID_DYN_START ||
	    chan->chan_policy == BT_CHANNEL_POLICY_BREDR_ONLY ||
	    (chan->mode != L2CAP_MODE_ERTM &&
	     chan->mode != L2CAP_MODE_STREAMING)) {
		result = L2CAP_MR_NOT_ALLOWED;
		goto send_move_response;
	}

	if (chan->local_amp_id == req->dest_amp_id) {
		result = L2CAP_MR_SAME_ID;
		goto send_move_response;
	}

	if (req->dest_amp_id) {
		struct hci_dev *hdev;
		hdev = hci_dev_get(req->dest_amp_id);
		if (!hdev || hdev->dev_type != HCI_AMP ||
		    !test_bit(HCI_UP, &hdev->flags)) {
			if (hdev)
				hci_dev_put(hdev);

			result = L2CAP_MR_BAD_ID;
			goto send_move_response;
		}
		hci_dev_put(hdev);
	}

	/* Detect a move collision.  Only send a collision response
	 * if this side has "lost", otherwise proceed with the move.
	 * The winner has the larger bd_addr.
	 */
	if ((__chan_is_moving(chan) ||
	     chan->move_role != L2CAP_MOVE_ROLE_NONE) &&
	    bacmp(conn->src, conn->dst) > 0) {
		result = L2CAP_MR_COLLISION;
		goto send_move_response;
	}

	chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
	l2cap_move_setup(chan);
	chan->move_id = req->dest_amp_id;
	icid = chan->dcid;

	if (!req->dest_amp_id) {
		/* Moving to BR/EDR */
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
			result = L2CAP_MR_PEND;
		} else {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			result = L2CAP_MR_SUCCESS;
		}
	} else {
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - uncomment when amp functions are available */
		/*amp_accept_physical(chan, req->dest_amp_id);*/
		result = L2CAP_MR_PEND;
	}

send_move_response:
	l2cap_send_move_chan_rsp(chan, result);

	l2cap_chan_unlock(chan);

	return 0;
}

static void l2cap_move_continue(struct l2cap_conn *conn, u16 icid, u16 result)
{
	struct l2cap_chan *chan;
	struct hci_chan *hchan = NULL;

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan) {
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);
	if (result == L2CAP_MR_PEND)
		__set_chan_timer(chan, L2CAP_MOVE_ERTX_TIMEOUT);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent when logical link
		 * is complete.
		 */
		chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		break;
	case L2CAP_MOVE_WAIT_RSP_SUCCESS:
		if (result == L2CAP_MR_PEND) {
			break;
		} else if (test_bit(CONN_LOCAL_BUSY,
				    &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else {
			/* Logical link is up or moving to BR/EDR,
			 * proceed with move
			 */
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		}
		break;
	case L2CAP_MOVE_WAIT_RSP:
		/* Moving to AMP */
		if (result == L2CAP_MR_SUCCESS) {
			/* Remote is ready, send confirm immediately
			 * after logical link is ready
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		} else {
			/* Both logical link and move success
			 * are required to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_COMP;
		}

		/* Placeholder - get hci_chan for logical link */
		if (!hchan) {
			/* Logical link not available */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
			break;
		}

		/* If the logical link is not yet connected, do not
		 * send confirmation.
		 */
		if (hchan->state != BT_CONNECTED)
			break;

		/* Logical link is already ready to go */

		chan->hs_hcon = hchan->conn;
		chan->hs_hcon->l2cap_data = chan->conn;

		if (result == L2CAP_MR_SUCCESS) {
			/* Can confirm now */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else {
			/* Now only need move success
			 * to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		}

		l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		break;
	default:
		/* Any other amp move state means the move failed. */
		chan->move_id = chan->local_amp_id;
		l2cap_move_done(chan);
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
	}

	l2cap_chan_unlock(chan);
}

static void l2cap_move_fail(struct l2cap_conn *conn, u8 ident, u16 icid,
			    u16 result)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_ident(conn, ident);
	if (!chan) {
		/* Could not locate channel, icid is best guess */
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);

	if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
		if (result == L2CAP_MR_COLLISION) {
			chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
		} else {
			/* Cleanup - cancel move */
			chan->move_id = chan->local_amp_id;
			l2cap_move_done(chan);
		}
	}

	l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);

	l2cap_chan_unlock(chan);
}

static int l2cap_move_channel_rsp(struct l2cap_conn *conn,
				  struct l2cap_cmd_hdr *cmd,
				  u16 cmd_len, void *data)
{
	struct l2cap_move_chan_rsp *rsp = data;
	u16 icid, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);
	result = le16_to_cpu(rsp->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	if (result == L2CAP_MR_SUCCESS || result == L2CAP_MR_PEND)
		l2cap_move_continue(conn, icid, result);
	else
		l2cap_move_fail(conn, cmd->ident, icid, result);

	return 0;
}

static int l2cap_move_channel_confirm(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd,
				      u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm *cfm = data;
	struct l2cap_chan *chan;
	u16 icid, result;

	if (cmd_len != sizeof(*cfm))
		return -EPROTO;

	icid = le16_to_cpu(cfm->icid);
	result = le16_to_cpu(cfm->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		/* Spec requires a response even if the icid was not found */
		l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);
		return 0;
	}

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM) {
		if (result == L2CAP_MC_CONFIRMED) {
			chan->local_amp_id = chan->move_id;
			if (!chan->local_amp_id)
				__release_logical_link(chan);
		} else {
			chan->move_id = chan->local_amp_id;
		}

		l2cap_move_done(chan);
	}

	l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_move_channel_confirm_rsp(struct l2cap_conn *conn,
						 struct l2cap_cmd_hdr *cmd,
						 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm_rsp *rsp = data;
	struct l2cap_chan *chan;
	u16 icid;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);

	BT_DBG("icid 0x%4.4x", icid);

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan)
		return 0;

	__clear_chan_timer(chan);

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM_RSP) {
		chan->local_amp_id = chan->move_id;

		if (!chan->local_amp_id && chan->hs_hchan)
			__release_logical_link(chan);

		l2cap_move_done(chan);
	}

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_check_conn_param(u16 min, u16 max, u16 latency,
					 u16 to_multiplier)
{
	u16 max_latency;

	if (min > max || min < 6 || max > 3200)
		return -EINVAL;

	if (to_multiplier < 10 || to_multiplier > 3200)
		return -EINVAL;

	if (max >= to_multiplier * 8)
		return -EINVAL;

	max_latency = (to_multiplier * 8 / max) - 1;
	if (latency > 499 || latency > max_latency)
		return -EINVAL;

	return 0;
}

static inline int l2cap_conn_param_update_req(struct l2cap_conn *conn,
					      struct l2cap_cmd_hdr *cmd,
					      u8 *data)
{
	struct hci_conn *hcon = conn->hcon;
	struct l2cap_conn_param_update_req *req;
	struct l2cap_conn_param_update_rsp rsp;
	u16 min, max, latency, to_multiplier, cmd_len;
	int err;

	if (!(hcon->link_mode & HCI_LM_MASTER))
		return -EINVAL;

	cmd_len = __le16_to_cpu(cmd->len);
	if (cmd_len != sizeof(struct l2cap_conn_param_update_req))
		return -EPROTO;

	req = (struct l2cap_conn_param_update_req *) data;
	min		= __le16_to_cpu(req->min);
	max		= __le16_to_cpu(req->max);
	latency		= __le16_to_cpu(req->latency);
	to_multiplier	= __le16_to_cpu(req->to_multiplier);

	BT_DBG("min 0x%4.4x max 0x%4.4x latency: 0x%4.4x Timeout: 0x%4.4x",
	       min, max, latency, to_multiplier);

	memset(&rsp, 0, sizeof(rsp));

	err = l2cap_check_conn_param(min, max, latency, to_multiplier);
	if (err)
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_REJECTED);
	else
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_ACCEPTED);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CONN_PARAM_UPDATE_RSP,
		       sizeof(rsp), &rsp);

	if (!err)
		hci_le_conn_update(hcon, min, max, latency, to_multiplier);

	return 0;
}

static inline int l2cap_bredr_sig_cmd(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				      u8 *data)
{
	int err = 0;

	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}

	return err;
}

static inline int l2cap_le_sig_cmd(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u8 *data)
{
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		return 0;

	case L2CAP_CONN_PARAM_UPDATE_REQ:
		return l2cap_conn_param_update_req(conn, cmd, data);

	case L2CAP_CONN_PARAM_UPDATE_RSP:
		return 0;

	default:
		BT_ERR("Unknown LE signaling command 0x%2.2x", cmd->code);
		return -EINVAL;
	}
}

static inline void l2cap_sig_channel(struct l2cap_conn *conn,
				     struct sk_buff *skb)
{
	u8 *data = skb->data;
	int len = skb->len;
	struct l2cap_cmd_hdr cmd;
	int err;

	l2cap_raw_recv(conn, skb);

	while (len >= L2CAP_CMD_HDR_SIZE) {
		u16 cmd_len;
		memcpy(&cmd, data, L2CAP_CMD_HDR_SIZE);
		data += L2CAP_CMD_HDR_SIZE;
		len  -= L2CAP_CMD_HDR_SIZE;

		cmd_len = le16_to_cpu(cmd.len);

		BT_DBG("code 0x%2.2x len %d id 0x%2.2x", cmd.code, cmd_len,
		       cmd.ident);

		if (cmd_len > len || !cmd.ident) {
			BT_DBG("corrupted command");
			break;
		}

		if (conn->hcon->type == LE_LINK)
			err = l2cap_le_sig_cmd(conn, &cmd, data);
		else
			err = l2cap_bredr_sig_cmd(conn, &cmd, cmd_len, data);

		if (err) {
			struct l2cap_cmd_rej_unk rej;

			BT_ERR("Wrong link type (%d)", err);

			/* FIXME: Map err to a valid reason */
			rej.reason = __constant_cpu_to_le16(L2CAP_REJ_NOT_UNDERSTOOD);
			l2cap_send_cmd(conn, cmd.ident, L2CAP_COMMAND_REJ,
				       sizeof(rej), &rej);
		}

		data += cmd_len;
		len  -= cmd_len;
	}

	kfree_skb(skb);
}

static int l2cap_check_fcs(struct l2cap_chan *chan,  struct sk_buff *skb)
{
	u16 our_fcs, rcv_fcs;
	int hdr_size;

	if (test_bit(FLAG_EXT_CTRL, &chan->flags))
		hdr_size = L2CAP_EXT_HDR_SIZE;
	else
		hdr_size = L2CAP_ENH_HDR_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16) {
		skb_trim(skb, skb->len - L2CAP_FCS_SIZE);
		rcv_fcs = get_unaligned_le16(skb->data + skb->len);
		our_fcs = crc16(0, skb->data - hdr_size, skb->len + hdr_size);

		if (our_fcs != rcv_fcs)
			return -EBADMSG;
	}
	return 0;
}

static void l2cap_send_i_or_rr_or_rnr(struct l2cap_chan *chan)
{
	struct l2cap_ctrl control;

	BT_DBG("chan %p", chan);

	memset(&control, 0, sizeof(control));
	control.sframe = 1;
	control.final = 1;
	control.reqseq = chan->buffer_seq;
	set_bit(CONN_SEND_FBIT, &chan->conn_state);

	if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		control.super = L2CAP_SUPER_RNR;
		l2cap_send_sframe(chan, &control);
	}

	if (test_and_clear_bit(CONN_REMOTE_BUSY, &chan->conn_state) &&
	    chan->unacked_frames > 0)
		__set_retrans_timer(chan);

	/* Send pending iframes */
	l2cap_ertm_send(chan);

	if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state) &&
	    test_bit(CONN_SEND_FBIT, &chan->conn_state)) {
		/* F-bit wasn't sent in an s-frame or i-frame yet, so
		 * send it now.
		 */
		control.super = L2CAP_SUPER_RR;
		l2cap_send_sframe(chan, &control);
	}
}

static void append_skb_frag(struct sk_buff *skb, struct sk_buff *new_frag,
			    struct sk_buff **last_frag)
{
	/* skb->len reflects data in skb as well as all fragments
	 * skb->data_len reflects only data in fragments
	 */
	if (!skb_has_frag_list(skb))
		skb_shinfo(skb)->frag_list = new_frag;

	new_frag->next = NULL;

	(*last_frag)->next = new_frag;
	*last_frag = new_frag;

	skb->len += new_frag->len;
	skb->data_len += new_frag->len;
	skb->truesize += new_frag->truesize;
}

static int l2cap_reassemble_sdu(struct l2cap_chan *chan, struct sk_buff *skb,
				struct l2cap_ctrl *control)
{
	int err = -EINVAL;

	switch (control->sar) {
	case L2CAP_SAR_UNSEGMENTED:
		if (chan->sdu)
			break;

		err = chan->ops->recv(chan, skb);
		break;

	case L2CAP_SAR_START:
		if (chan->sdu)
			break;

		chan->sdu_len = get_unaligned_le16(skb->data);
		skb_pull(skb, L2CAP_SDULEN_SIZE);

		if (chan->sdu_len > chan->imtu) {
			err = -EMSGSIZE;
			break;
		}

		if (skb->len >= chan->sdu_len)
			break;

		chan->sdu = skb;
		chan->sdu_last_frag = skb;

		skb = NULL;
		err = 0;
		break;

	case L2CAP_SAR_CONTINUE:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len >= chan->sdu_len)
			break;

		err = 0;
		break;

	case L2CAP_SAR_END:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len != chan->sdu_len)
			break;

		err = chan->ops->recv(chan, chan->sdu);

		if (!err) {
			/* Reassembly complete */
			chan->sdu = NULL;
			chan->sdu_last_frag = NULL;
			chan->sdu_len = 0;
		}
		break;
	}

	if (err) {
		kfree_skb(skb);
		kfree_skb(chan->sdu);
		chan->sdu = NULL;
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;
	}

	return err;
}

static int l2cap_resegment(struct l2cap_chan *chan)
{
	/* Placeholder */
	return 0;
}

void l2cap_chan_busy(struct l2cap_chan *chan, int busy)
{
	u8 event;

	if (chan->mode != L2CAP_MODE_ERTM)
		return;

	event = busy ? L2CAP_EV_LOCAL_BUSY_DETECTED : L2CAP_EV_LOCAL_BUSY_CLEAR;
	l2cap_tx(chan, NULL, NULL, event);
}

static int l2cap_rx_queued_iframes(struct l2cap_chan *chan)
{
	int err = 0;
	/* Pass sequential frames to l2cap_reassemble_sdu()
	 * until a gap is encountered.
	 */

	BT_DBG("chan %p", chan);

	while (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		struct sk_buff *skb;
		BT_DBG("Searching for skb with txseq %d (queue len %d)",
		       chan->buffer_seq, skb_queue_len(&chan->srej_q));

		skb = l2cap_ertm_seq_in_queue(&chan->srej_q, chan->buffer_seq);

		if (!skb)
			break;

		skb_unlink(skb, &chan->srej_q);
		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);
		err = l2cap_reassemble_sdu(chan, skb, &bt_cb(skb)->control);
		if (err)
			break;
	}

	if (skb_queue_empty(&chan->srej_q)) {
		chan->rx_state = L2CAP_RX_STATE_RECV;
		l2cap_send_ack(chan);
	}

	return err;
}

static void l2cap_handle_srej(struct l2cap_chan *chan,
			      struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (skb == NULL) {
		BT_DBG("Seq %d not available for retransmission",
		       control->reqseq);
		return;
	}

	if (chan->max_tx != 0 && bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	if (control->poll) {
		l2cap_pass_to_tx(chan, control);

		set_bit(CONN_SEND_FBIT, &chan->conn_state);
		l2cap_retransmit(chan, control);
		l2cap_ertm_send(chan);

		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
			set_bit(CONN_SREJ_ACT, &chan->conn_state);
			chan->srej_save_reqseq = control->reqseq;
		}
	} else {
		l2cap_pass_to_tx_fbit(chan, control);

		if (control->final) {
			if (chan->srej_save_reqseq != control->reqseq ||
			    !test_and_clear_bit(CONN_SREJ_ACT,
						&chan->conn_state))
				l2cap_retransmit(chan, control);
		} else {
			l2cap_retransmit(chan, control);
			if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
				set_bit(CONN_SREJ_ACT, &chan->conn_state);
				chan->srej_save_reqseq = control->reqseq;
			}
		}
	}
}

static void l2cap_handle_rej(struct l2cap_chan *chan,
			     struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (chan->max_tx && skb &&
	    bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	l2cap_pass_to_tx(chan, control);

	if (control->final) {
		if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state))
			l2cap_retransmit_all(chan, control);
	} else {
		l2cap_retransmit_all(chan, control);
		l2cap_ertm_send(chan);
		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F)
			set_bit(CONN_REJ_ACT, &chan->conn_state);
	}
}

static u8 l2cap_classify_txseq(struct l2cap_chan *chan, u16 txseq)
{
	BT_DBG("chan %p, txseq %d", chan, txseq);

	BT_DBG("last_acked_seq %d, expected_tx_seq %d", chan->last_acked_seq,
	       chan->expected_tx_seq);

	if (chan->rx_state == L2CAP_RX_STATE_SREJ_SENT) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			/* See notes below regarding "double poll" and
			 * invalid packets.
			 */
			if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
				BT_DBG("Invalid/Ignore - after SREJ");
				return L2CAP_TXSEQ_INVALID_IGNORE;
			} else {
				BT_DBG("Invalid - in window after SREJ sent");
				return L2CAP_TXSEQ_INVALID;
			}
		}

		if (chan->srej_list.head == txseq) {
			BT_DBG("Expected SREJ");
			return L2CAP_TXSEQ_EXPECTED_SREJ;
		}

		if (l2cap_ertm_seq_in_queue(&chan->srej_q, txseq)) {
			BT_DBG("Duplicate SREJ - txseq already stored");
			return L2CAP_TXSEQ_DUPLICATE_SREJ;
		}

		if (l2cap_seq_list_contains(&chan->srej_list, txseq)) {
			BT_DBG("Unexpected SREJ - not requested");
			return L2CAP_TXSEQ_UNEXPECTED_SREJ;
		}
	}

	if (chan->expected_tx_seq == txseq) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		} else {
			BT_DBG("Expected");
			return L2CAP_TXSEQ_EXPECTED;
		}
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) <
	    __seq_offset(chan, chan->expected_tx_seq, chan->last_acked_seq)) {
		BT_DBG("Duplicate - expected_tx_seq later than txseq");
		return L2CAP_TXSEQ_DUPLICATE;
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) >= chan->tx_win) {
		/* A source of invalid packets is a "double poll" condition,
		 * where delays cause us to send multiple poll packets.  If
		 * the remote stack receives and processes both polls,
		 * sequence numbers can wrap around in such a way that a
		 * resent frame has a sequence number that looks like new data
		 * with a sequence gap.  This would trigger an erroneous SREJ
		 * request.
		 *
		 * Fortunately, this is impossible with a tx window that's
		 * less than half of the maximum sequence number, which allows
		 * invalid frames to be safely ignored.
		 *
		 * With tx window sizes greater than half of the tx window
		 * maximum, the frame is invalid and cannot be ignored.  This
		 * causes a disconnect.
		 */

		if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
			BT_DBG("Invalid/Ignore - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID_IGNORE;
		} else {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		}
	} else {
		BT_DBG("Unexpected - txseq indicates missing frames");
		return L2CAP_TXSEQ_UNEXPECTED;
	}
}

static int l2cap_rx_state_recv(struct l2cap_chan *chan,
			       struct l2cap_ctrl *control,
			       struct sk_buff *skb, u8 event)
{
	int err = 0;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, control->txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			l2cap_pass_to_tx(chan, control);

			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding expected seq %d",
				       control->txseq);
				break;
			}

			chan->expected_tx_seq = __next_seq(chan,
							   control->txseq);

			chan->buffer_seq = chan->expected_tx_seq;
			skb_in_use = 1;

			err = l2cap_reassemble_sdu(chan, skb, control);
			if (err)
				break;

			if (control->final) {
				if (!test_and_clear_bit(CONN_REJ_ACT,
							&chan->conn_state)) {
					control->final = 0;
					l2cap_retransmit_all(chan, control);
					l2cap_ertm_send(chan);
				}
			}

			if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state))
				l2cap_send_ack(chan);
			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			l2cap_pass_to_tx(chan, control);

			/* Can't issue SREJ frames in the local busy state.
			 * Drop this frame, it will be seen as missing
			 * when local busy is exited.
			 */
			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding unexpected seq %d",
				       control->txseq);
				break;
			}

			/* There was a gap in the sequence, so an SREJ
			 * must be sent for each missing frame.  The
			 * current frame is stored for later use.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			clear_bit(CONN_SREJ_ACT, &chan->conn_state);
			l2cap_seq_list_clear(&chan->srej_list);
			l2cap_send_srej(chan, control->txseq);

			chan->rx_state = L2CAP_RX_STATE_SREJ_SENT;
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state) &&
			    !__chan_is_moving(chan)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			l2cap_send_i_or_rr_or_rnr(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_ertm_send(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control && control->poll) {
			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_rr_or_rnr(chan, 0);
		}
		__clear_retrans_timer(chan);
		l2cap_seq_list_clear(&chan->retrans_list);
		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	default:
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_rx_state_srej_sent(struct l2cap_chan *chan,
				    struct l2cap_ctrl *control,
				    struct sk_buff *skb, u8 event)
{
	int err = 0;
	u16 txseq = control->txseq;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			/* Keep frame for reassembly later */
			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			chan->expected_tx_seq = __next_seq(chan, txseq);
			break;
		case L2CAP_TXSEQ_EXPECTED_SREJ:
			l2cap_seq_list_pop(&chan->srej_list);

			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			err = l2cap_rx_queued_iframes(chan);
			if (err)
				break;

			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			/* Got a frame that can't be reassembled yet.
			 * Save it for later, and send SREJs to cover
			 * the missing frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_UNEXPECTED_SREJ:
			/* This frame was requested with an SREJ, but
			 * some expected retransmitted frames are
			 * missing.  Request retransmission of missing
			 * SREJ'd frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej_list(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_DUPLICATE_SREJ:
			/* We've already queued this frame.  Drop this copy. */
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			/* Expecting a later sequence number, so this frame
			 * was already received.  Ignore it completely.
			 */
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT,
						&chan->conn_state)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames) {
				__set_retrans_timer(chan);
			}

			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_srej_tail(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_send_ack(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control->poll) {
			l2cap_send_srej_tail(chan);
		} else {
			struct l2cap_ctrl rr_control;
			memset(&rr_control, 0, sizeof(rr_control));
			rr_control.sframe = 1;
			rr_control.super = L2CAP_SUPER_RR;
			rr_control.reqseq = chan->buffer_seq;
			l2cap_send_sframe(chan, &rr_control);
		}

		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_finish_move(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	chan->rx_state = L2CAP_RX_STATE_RECV;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	return l2cap_resegment(chan);
}

static int l2cap_rx_state_wait_p(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	if (!control->poll)
		return -EPROTO;

	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	err = l2cap_finish_move(chan);
	if (err)
		return err;

	set_bit(CONN_SEND_FBIT, &chan->conn_state);
	l2cap_send_i_or_rr_or_rnr(chan);

	if (event == L2CAP_EV_RECV_IFRAME)
		return -EPROTO;

	return l2cap_rx_state_recv(chan, control, NULL, event);
}

static int l2cap_rx_state_wait_f(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	if (!control->final)
		return -EPROTO;

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	chan->rx_state = L2CAP_RX_STATE_RECV;
	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	err = l2cap_resegment(chan);

	if (!err)
		err = l2cap_rx_state_recv(chan, control, skb, event);

	return err;
}

static bool __valid_reqseq(struct l2cap_chan *chan, u16 reqseq)
{
	/* Make sure reqseq is for a packet that has been sent but not acked */
	u16 unacked;

	unacked = __seq_offset(chan, chan->next_tx_seq, chan->expected_ack_seq);
	return __seq_offset(chan, chan->next_tx_seq, reqseq) <= unacked;
}

static int l2cap_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
		    struct sk_buff *skb, u8 event)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d, state %d", chan,
	       control, skb, event, chan->rx_state);

	if (__valid_reqseq(chan, control->reqseq)) {
		switch (chan->rx_state) {
		case L2CAP_RX_STATE_RECV:
			err = l2cap_rx_state_recv(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_SREJ_SENT:
			err = l2cap_rx_state_srej_sent(chan, control, skb,
						       event);
			break;
		case L2CAP_RX_STATE_WAIT_P:
			err = l2cap_rx_state_wait_p(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_WAIT_F:
			err = l2cap_rx_state_wait_f(chan, control, skb, event);
			break;
		default:
			/* shut it down */
			break;
		}
	} else {
		BT_DBG("Invalid reqseq %d (next_tx_seq %d, expected_ack_seq %d",
		       control->reqseq, chan->next_tx_seq,
		       chan->expected_ack_seq);
		l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return err;
}

static int l2cap_stream_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
			   struct sk_buff *skb)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, state %d", chan, control, skb,
	       chan->rx_state);

	if (l2cap_classify_txseq(chan, control->txseq) ==
	    L2CAP_TXSEQ_EXPECTED) {
		l2cap_pass_to_tx(chan, control);

		BT_DBG("buffer_seq %d->%d", chan->buffer_seq,
		       __next_seq(chan, chan->buffer_seq));

		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);

		l2cap_reassemble_sdu(chan, skb, control);
	} else {
		if (chan->sdu) {
			kfree_skb(chan->sdu);
			chan->sdu = NULL;
		}
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;

		if (skb) {
			BT_DBG("Freeing %p", skb);
			kfree_skb(skb);
		}
	}

	chan->last_acked_seq = control->txseq;
	chan->expected_tx_seq = __next_seq(chan, control->txseq);

	return err;
}

static int l2cap_data_rcv(struct l2cap_chan *chan, struct sk_buff *skb)
{
	struct l2cap_ctrl *control = &bt_cb(skb)->control;
	u16 len;
	u8 event;

	__unpack_control(chan, skb);

	len = skb->len;

	/*
	 * We can just drop the corrupted I-frame here.
	 * Receiver will miss it and start proper recovery
	 * procedures and ask for retransmission.
	 */
	if (l2cap_check_fcs(chan, skb))
		goto drop;

	if (!control->sframe && control->sar == L2CAP_SAR_START)
		len -= L2CAP_SDULEN_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16)
		len -= L2CAP_FCS_SIZE;

	if (len > chan->mps) {
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto drop;
	}

	if (!control->sframe) {
		int err;

		BT_DBG("iframe sar %d, reqseq %d, final %d, txseq %d",
		       control->sar, control->reqseq, control->final,
		       control->txseq);

		/* Validate F-bit - F=0 always valid, F=1 only
		 * valid in TX WAIT_F
		 */
		if (control->final && chan->tx_state != L2CAP_TX_STATE_WAIT_F)
			goto drop;

		if (chan->mode != L2CAP_MODE_STREAMING) {
			event = L2CAP_EV_RECV_IFRAME;
			err = l2cap_rx(chan, control, skb, event);
		} else {
			err = l2cap_stream_rx(chan, control, skb);
		}

		if (err)
			l2cap_send_disconn_req(chan, ECONNRESET);
	} else {
		const u8 rx_func_to_event[4] = {
			L2CAP_EV_RECV_RR, L2CAP_EV_RECV_REJ,
			L2CAP_EV_RECV_RNR, L2CAP_EV_RECV_SREJ
		};

		/* Only I-frames are expected in streaming mode */
		if (chan->mode == L2CAP_MODE_STREAMING)
			goto drop;

		BT_DBG("sframe reqseq %d, final %d, poll %d, super %d",
		       control->reqseq, control->final, control->poll,
		       control->super);

		if (len != 0) {
			BT_ERR("Trailing bytes: %d in sframe", len);
			l2cap_send_disconn_req(chan, ECONNRESET);
			goto drop;
		}

		/* Validate F and P bits */
		if (control->final && (control->poll ||
				       chan->tx_state != L2CAP_TX_STATE_WAIT_F))
			goto drop;

		event = rx_func_to_event[control->super];
		if (l2cap_rx(chan, control, skb, event))
			l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return 0;

drop:
	kfree_skb(skb);
	return 0;
}

static void l2cap_data_channel(struct l2cap_conn *conn, u16 cid,
			       struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_scid(conn, cid);
	if (!chan) {
		if (cid == L2CAP_CID_A2MP) {
			chan = a2mp_channel_create(conn, skb);
			if (!chan) {
				kfree_skb(skb);
				return;
			}

			l2cap_chan_lock(chan);
		} else {
			BT_DBG("unknown cid 0x%4.4x", cid);
			/* Drop packet and return */
			kfree_skb(skb);
			return;
		}
	}

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_CONNECTED)
		goto drop;

	switch (chan->mode) {
	case L2CAP_MODE_BASIC:
		/* If socket recv buffers overflows we drop data here
		 * which is *bad* because L2CAP has to be reliable.
		 * But we don't have any other choice. L2CAP doesn't
		 * provide flow control mechanism. */

		if (chan->imtu < skb->len)
			goto drop;

		if (!chan->ops->recv(chan, skb))
			goto done;
		break;

	case L2CAP_MODE_ERTM:
	case L2CAP_MODE_STREAMING:
		l2cap_data_rcv(chan, skb);
		goto done;

	default:
		BT_DBG("chan %p: bad mode 0x%2.2x", chan, chan->mode);
		break;
	}

drop:
	kfree_skb(skb);

done:
	l2cap_chan_unlock(chan);
}

static void l2cap_conless_channel(struct l2cap_conn *conn, __le16 psm,
				  struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_psm(0, psm, conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_att_channel(struct l2cap_conn *conn,
			      struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_scid(0, L2CAP_CID_LE_DATA,
					 conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_recv_frame(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct l2cap_hdr *lh = (void *) skb->data;
	u16 cid, len;
	__le16 psm;

	skb_pull(skb, L2CAP_HDR_SIZE);
	cid = __le16_to_cpu(lh->cid);
	len = __le16_to_cpu(lh->len);

	if (len != skb->len) {
		kfree_skb(skb);
		return;
	}

	BT_DBG("len %d, cid 0x%4.4x", len, cid);

	switch (cid) {
	case L2CAP_CID_LE_SIGNALING:
	case L2CAP_CID_SIGNALING:
		l2cap_sig_channel(conn, skb);
		break;

	case L2CAP_CID_CONN_LESS:
		psm = get_unaligned((__le16 *) skb->data);
		skb_pull(skb, L2CAP_PSMLEN_SIZE);
		l2cap_conless_channel(conn, psm, skb);
		break;

	case L2CAP_CID_LE_DATA:
		l2cap_att_channel(conn, skb);
		break;

	case L2CAP_CID_SMP:
		if (smp_sig_channel(conn, skb))
			l2cap_conn_del(conn->hcon, EACCES);
		break;

	default:
		l2cap_data_channel(conn, cid, skb);
		break;
	}
}

/* ---- L2CAP interface with lower layer (HCI) ---- */

int l2cap_connect_ind(struct hci_dev *hdev, bdaddr_t *bdaddr)
{
	int exact = 0, lm1 = 0, lm2 = 0;
	struct l2cap_chan *c;

	BT_DBG("hdev %s, bdaddr %pMR", hdev->name, bdaddr);

	/* Find listening sockets and check their link_mode */
	read_lock(&chan_list_lock);
	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		if (c->state != BT_LISTEN)
			continue;

		if (!bacmp(&bt_sk(sk)->src, &hdev->bdaddr)) {
			lm1 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm1 |= HCI_LM_MASTER;
			exact++;
		} else if (!bacmp(&bt_sk(sk)->src, BDADDR_ANY)) {
			lm2 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm2 |= HCI_LM_MASTER;
		}
	}
	read_unlock(&chan_list_lock);

	return exact ? lm1 : lm2;
}

void l2cap_connect_cfm(struct hci_conn *hcon, u8 status)
{
	struct l2cap_conn *conn;

	BT_DBG("hcon %p bdaddr %pMR status %d", hcon, &hcon->dst, status);

	if (!status) {
		conn = l2cap_conn_add(hcon);
		if (conn)
			l2cap_conn_ready(conn);
	} else {
		l2cap_conn_del(hcon, bt_to_errno(status));
	}
}

int l2cap_disconn_ind(struct hci_conn *hcon)
{
	struct l2cap_conn *conn = hcon->l2cap_data;

	BT_DBG("hcon %p", hcon);

	if (!conn)
		return HCI_ERROR_REMOTE_USER_TERM;
	return conn->disc_reason;
}

void l2cap_disconn_cfm(struct hci_conn *hcon, u8 reason)
{
	BT_DBG("hcon %p reason %d", hcon, reason);

	l2cap_conn_del(hcon, bt_to_errno(reason));
}

static inline void l2cap_check_encryption(struct l2cap_chan *chan, u8 encrypt)
{
	if (chan->chan_type != L2CAP_CHAN_CONN_ORIENTED)
		return;

	if (encrypt == 0x00) {
		if (chan->sec_level == BT_SECURITY_MEDIUM) {
			__set_chan_timer(chan, L2CAP_ENC_TIMEOUT);
		} else if (chan->sec_level == BT_SECURITY_HIGH)
			l2cap_chan_close(chan, ECONNREFUSED);
	} else {
		if (chan->sec_level == BT_SECURITY_MEDIUM)
			__clear_chan_timer(chan);
	}
}

int l2cap_security_cfm(struct hci_conn *hcon, u8 status, u8 encrypt)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_chan *chan;

	if (!conn)
		return 0;

	BT_DBG("conn %p status 0x%2.2x encrypt %u", conn, status, encrypt);

	if (hcon->type == LE_LINK) {
		if (!status && encrypt)
			smp_distribute_keys(conn, 0);
		cancel_delayed_work(&conn->security_timer);
	}

	mutex_lock(&conn->chan_lock);

	list_for_each_entry(chan, &conn->chan_l, list) {
		l2cap_chan_lock(chan);

		BT_DBG("chan %p scid 0x%4.4x state %s", chan, chan->scid,
		       state_to_string(chan->state));

		if (chan->chan_type == L2CAP_CHAN_CONN_FIX_A2MP) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->scid == L2CAP_CID_LE_DATA) {
			if (!status && encrypt) {
				chan->sec_level = hcon->sec_level;
				l2cap_chan_ready(chan);
			}

			l2cap_chan_unlock(chan);
			continue;
		}

		if (!__l2cap_no_conn_pending(chan)) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (!status && (chan->state == BT_CONNECTED ||
				chan->state == BT_CONFIG)) {
			struct sock *sk = chan->sk;

			clear_bit(BT_SK_SUSPEND, &bt_sk(sk)->flags);
			sk->sk_state_change(sk);

			l2cap_check_encryption(chan, encrypt);
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->state == BT_CONNECT) {
			if (!status) {
				l2cap_start_connection(chan);
			} else {
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
			}
		} else if (chan->state == BT_CONNECT2) {
			struct sock *sk = chan->sk;
			struct l2cap_conn_rsp rsp;
			__u16 res, stat;

			lock_sock(sk);

			if (!status) {
				if (test_bit(BT_SK_DEFER_SETUP,
					     &bt_sk(sk)->flags)) {
					res = L2CAP_CR_PEND;
					stat = L2CAP_CS_AUTHOR_PEND;
					chan->ops->defer(chan);
				} else {
					__l2cap_state_change(chan, BT_CONFIG);
					res = L2CAP_CR_SUCCESS;
					stat = L2CAP_CS_NO_INFO;
				}
			} else {
				__l2cap_state_change(chan, BT_DISCONN);
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
				res = L2CAP_CR_SEC_BLOCK;
				stat = L2CAP_CS_NO_INFO;
			}

			release_sock(sk);

			rsp.scid   = cpu_to_le16(chan->dcid);
			rsp.dcid   = cpu_to_le16(chan->scid);
			rsp.result = cpu_to_le16(res);
			rsp.status = cpu_to_le16(stat);
			l2cap_send_cmd(conn, chan->ident, L2CAP_CONN_RSP,
				       sizeof(rsp), &rsp);

			if (!test_bit(CONF_REQ_SENT, &chan->conf_state) &&
			    res == L2CAP_CR_SUCCESS) {
				char buf[128];
				set_bit(CONF_REQ_SENT, &chan->conf_state);
				l2cap_send_cmd(conn, l2cap_get_ident(conn),
					       L2CAP_CONF_REQ,
					       l2cap_build_conf_req(chan, buf),
					       buf);
				chan->num_conf_req++;
			}
		}

		l2cap_chan_unlock(chan);
	}

	mutex_unlock(&conn->chan_lock);

	return 0;
}

int l2cap_recv_acldata(struct hci_conn *hcon, struct sk_buff *skb, u16 flags)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_hdr *hdr;
	int len;

	/* For AMP controller do not create l2cap conn */
	if (!conn && hcon->hdev->dev_type != HCI_BREDR)
		goto drop;

	if (!conn)
		conn = l2cap_conn_add(hcon);

	if (!conn)
		goto drop;

	BT_DBG("conn %p len %d flags 0x%x", conn, skb->len, flags);

	switch (flags) {
	case ACL_START:
	case ACL_START_NO_FLUSH:
	case ACL_COMPLETE:
		if (conn->rx_len) {
			BT_ERR("Unexpected start frame (len %d)", skb->len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
		}

		/* Start fragment always begin with Basic L2CAP header */
		if (skb->len < L2CAP_HDR_SIZE) {
			BT_ERR("Frame is too short (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		hdr = (struct l2cap_hdr *) skb->data;
		len = __le16_to_cpu(hdr->len) + L2CAP_HDR_SIZE;

		if (len == skb->len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, skb);
			return 0;
		}

		BT_DBG("Start: total len %d, frag len %d", len, skb->len);

		if (skb->len > len) {
			BT_ERR("Frame is too long (len %d, expected len %d)",
			       skb->len, len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		/* Allocate skb for the complete frame (with header) */
		conn->rx_skb = bt_skb_alloc(len, GFP_KERNEL);
		if (!conn->rx_skb)
			goto drop;

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len = len - skb->len;
		break;

	case ACL_CONT:
		BT_DBG("Cont: frag len %d (expecting %d)", skb->len, conn->rx_len);

		if (!conn->rx_len) {
			BT_ERR("Unexpected continuation frame (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		if (skb->len > conn->rx_len) {
			BT_ERR("Fragment is too long (len %d, expected %d)",
			       skb->len, conn->rx_len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len -= skb->len;

		if (!conn->rx_len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, conn->rx_skb);
			conn->rx_skb = NULL;
		}
		break;
	}

drop:
	kfree_skb(skb);
	return 0;
}

static int l2cap_debugfs_show(struct seq_file *f, void *p)
{
	struct l2cap_chan *c;

	read_lock(&chan_list_lock);

	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		seq_printf(f, "%pMR %pMR %d %d 0x%4.4x 0x%4.4x %d %d %d %d\n",
			   &bt_sk(sk)->src, &bt_sk(sk)->dst,
			   c->state, __le16_to_cpu(c->psm),
			   c->scid, c->dcid, c->imtu, c->omtu,
			   c->sec_level, c->mode);
	}

	read_unlock(&chan_list_lock);

	return 0;
}

static int l2cap_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, l2cap_debugfs_show, inode->i_private);
}

static const struct file_operations l2cap_debugfs_fops = {
	.open		= l2cap_debugfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *l2cap_debugfs;

int __init l2cap_init(void)
{
	int err;

	err = l2cap_init_sockets();
	if (err < 0)
		return err;

	if (bt_debugfs) {
		l2cap_debugfs = debugfs_create_file("l2cap", 0444, bt_debugfs,
						    NULL, &l2cap_debugfs_fops);
		if (!l2cap_debugfs)
			BT_ERR("Failed to create L2CAP debug file");
	}

	return 0;
}

void l2cap_exit(void)
{
	debugfs_remove(l2cap_debugfs);
	l2cap_cleanup_sockets();
}

module_param(disable_ertm, bool, 0644);
MODULE_PARM_DESC(disable_ertm, "Disable enhanced retransmission mode");
	} else {
		struct l2cap_info_rsp rsp;
		rsp.type   = cpu_to_le16(type);
		rsp.result = __constant_cpu_to_le16(L2CAP_IR_NOTSUPP);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_INFO_RSP, sizeof(rsp),
			       &rsp);
	}

	return 0;
}

static inline int l2cap_information_rsp(struct l2cap_conn *conn,
					struct l2cap_cmd_hdr *cmd, u16 cmd_len,
					u8 *data)
{
	struct l2cap_info_rsp *rsp = (struct l2cap_info_rsp *) data;
	u16 type, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	type   = __le16_to_cpu(rsp->type);
	result = __le16_to_cpu(rsp->result);

	BT_DBG("type 0x%4.4x result 0x%2.2x", type, result);

	/* L2CAP Info req/rsp are unbound to channels, add extra checks */
	if (cmd->ident != conn->info_ident ||
	    conn->info_state & L2CAP_INFO_FEAT_MASK_REQ_DONE)
		return 0;

	cancel_delayed_work(&conn->info_timer);

	if (result != L2CAP_IR_SUCCESS) {
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);

		return 0;
	}

	switch (type) {
	case L2CAP_IT_FEAT_MASK:
		conn->feat_mask = get_unaligned_le32(rsp->data);

		if (conn->feat_mask & L2CAP_FEAT_FIXED_CHAN) {
			struct l2cap_info_req req;
			req.type = __constant_cpu_to_le16(L2CAP_IT_FIXED_CHAN);

			conn->info_ident = l2cap_get_ident(conn);

			l2cap_send_cmd(conn, conn->info_ident,
				       L2CAP_INFO_REQ, sizeof(req), &req);
		} else {
			conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
			conn->info_ident = 0;

			l2cap_conn_start(conn);
		}
		break;

	case L2CAP_IT_FIXED_CHAN:
		conn->fixed_chan_mask = rsp->data[0];
		conn->info_state |= L2CAP_INFO_FEAT_MASK_REQ_DONE;
		conn->info_ident = 0;

		l2cap_conn_start(conn);
		break;
	}

	return 0;
}

static int l2cap_create_channel_req(struct l2cap_conn *conn,
				    struct l2cap_cmd_hdr *cmd,
				    u16 cmd_len, void *data)
{
	struct l2cap_create_chan_req *req = data;
	struct l2cap_create_chan_rsp rsp;
	struct l2cap_chan *chan;
	struct hci_dev *hdev;
	u16 psm, scid;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	if (!enable_hs)
		return -EINVAL;

	psm = le16_to_cpu(req->psm);
	scid = le16_to_cpu(req->scid);

	BT_DBG("psm 0x%2.2x, scid 0x%4.4x, amp_id %d", psm, scid, req->amp_id);

	/* For controller id 0 make BR/EDR connection */
	if (req->amp_id == HCI_BREDR_ID) {
		l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			      req->amp_id);
		return 0;
	}

	/* Validate AMP controller id */
	hdev = hci_dev_get(req->amp_id);
	if (!hdev)
		goto error;

	if (hdev->dev_type != HCI_AMP || !test_bit(HCI_UP, &hdev->flags)) {
		hci_dev_put(hdev);
		goto error;
	}

	chan = l2cap_connect(conn, cmd, data, L2CAP_CREATE_CHAN_RSP,
			     req->amp_id);
	if (chan) {
		struct amp_mgr *mgr = conn->hcon->amp_mgr;
		struct hci_conn *hs_hcon;

		hs_hcon = hci_conn_hash_lookup_ba(hdev, AMP_LINK, conn->dst);
		if (!hs_hcon) {
			hci_dev_put(hdev);
			return -EFAULT;
		}

		BT_DBG("mgr %p bredr_chan %p hs_hcon %p", mgr, chan, hs_hcon);

		mgr->bredr_chan = chan;
		chan->hs_hcon = hs_hcon;
		chan->fcs = L2CAP_FCS_NONE;
		conn->mtu = hdev->block_mtu;
	}

	hci_dev_put(hdev);

	return 0;

error:
	rsp.dcid = 0;
	rsp.scid = cpu_to_le16(scid);
	rsp.result = __constant_cpu_to_le16(L2CAP_CR_BAD_AMP);
	rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CREATE_CHAN_RSP,
		       sizeof(rsp), &rsp);

	return -EFAULT;
}

static void l2cap_send_move_chan_req(struct l2cap_chan *chan, u8 dest_amp_id)
{
	struct l2cap_move_chan_req req;
	u8 ident;

	BT_DBG("chan %p, dest_amp_id %d", chan, dest_amp_id);

	ident = l2cap_get_ident(chan->conn);
	chan->ident = ident;

	req.icid = cpu_to_le16(chan->scid);
	req.dest_amp_id = dest_amp_id;

	l2cap_send_cmd(chan->conn, ident, L2CAP_MOVE_CHAN_REQ, sizeof(req),
		       &req);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_rsp(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_rsp rsp;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	rsp.icid = cpu_to_le16(chan->dcid);
	rsp.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_RSP,
		       sizeof(rsp), &rsp);
}

static void l2cap_send_move_chan_cfm(struct l2cap_chan *chan, u16 result)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("chan %p, result 0x%4.4x", chan, result);

	chan->ident = l2cap_get_ident(chan->conn);

	cfm.icid = cpu_to_le16(chan->scid);
	cfm.result = cpu_to_le16(result);

	l2cap_send_cmd(chan->conn, chan->ident, L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);

	__set_chan_timer(chan, L2CAP_MOVE_TIMEOUT);
}

static void l2cap_send_move_chan_cfm_icid(struct l2cap_conn *conn, u16 icid)
{
	struct l2cap_move_chan_cfm cfm;

	BT_DBG("conn %p, icid 0x%4.4x", conn, icid);

	cfm.icid = cpu_to_le16(icid);
	cfm.result = __constant_cpu_to_le16(L2CAP_MC_UNCONFIRMED);

	l2cap_send_cmd(conn, l2cap_get_ident(conn), L2CAP_MOVE_CHAN_CFM,
		       sizeof(cfm), &cfm);
}

static void l2cap_send_move_chan_cfm_rsp(struct l2cap_conn *conn, u8 ident,
					 u16 icid)
{
	struct l2cap_move_chan_cfm_rsp rsp;

	BT_DBG("icid 0x%4.4x", icid);

	rsp.icid = cpu_to_le16(icid);
	l2cap_send_cmd(conn, ident, L2CAP_MOVE_CHAN_CFM_RSP, sizeof(rsp), &rsp);
}

static void __release_logical_link(struct l2cap_chan *chan)
{
	chan->hs_hchan = NULL;
	chan->hs_hcon = NULL;

	/* Placeholder - release the logical link */
}

static void l2cap_logical_fail(struct l2cap_chan *chan)
{
	/* Logical link setup failed */
	if (chan->state != BT_CONNECTED) {
		/* Create channel failure, disconnect */
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	switch (chan->move_role) {
	case L2CAP_MOVE_ROLE_RESPONDER:
		l2cap_move_done(chan);
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_SUPP);
		break;
	case L2CAP_MOVE_ROLE_INITIATOR:
		if (chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_COMP ||
		    chan->move_state == L2CAP_MOVE_WAIT_LOGICAL_CFM) {
			/* Remote has only sent pending or
			 * success responses, clean up
			 */
			l2cap_move_done(chan);
		}

		/* Other amp move states imply that the move
		 * has already aborted
		 */
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
		break;
	}
}

static void l2cap_logical_finish_create(struct l2cap_chan *chan,
					struct hci_chan *hchan)
{
	struct l2cap_conf_rsp rsp;

	chan->hs_hchan = hchan;
	chan->hs_hcon->l2cap_data = chan->conn;

	l2cap_send_efs_conf_rsp(chan, &rsp, chan->ident, 0);

	if (test_bit(CONF_INPUT_DONE, &chan->conf_state)) {
		int err;

		set_default_fcs(chan);

		err = l2cap_ertm_init(chan);
		if (err < 0)
			l2cap_send_disconn_req(chan, -err);
		else
			l2cap_chan_ready(chan);
	}
}

static void l2cap_logical_finish_move(struct l2cap_chan *chan,
				      struct hci_chan *hchan)
{
	chan->hs_hcon = hchan->conn;
	chan->hs_hcon->l2cap_data = chan->conn;

	BT_DBG("move_state %d", chan->move_state);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent after a success
		 * response is received
		 */
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		break;
	case L2CAP_MOVE_WAIT_LOGICAL_CFM:
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);
		}
		break;
	default:
		/* Move was not in expected state, free the channel */
		__release_logical_link(chan);

		chan->move_state = L2CAP_MOVE_STABLE;
	}
}

/* Call with chan locked */
void l2cap_logical_cfm(struct l2cap_chan *chan, struct hci_chan *hchan,
		       u8 status)
{
	BT_DBG("chan %p, hchan %p, status %d", chan, hchan, status);

	if (status) {
		l2cap_logical_fail(chan);
		__release_logical_link(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		/* Ignore logical link if channel is on BR/EDR */
		if (chan->local_amp_id)
			l2cap_logical_finish_create(chan, hchan);
	} else {
		l2cap_logical_finish_move(chan, hchan);
	}
}

void l2cap_move_start(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	if (chan->local_amp_id == HCI_BREDR_ID) {
		if (chan->chan_policy != BT_CHANNEL_POLICY_AMP_PREFERRED)
			return;
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - start physical link setup */
	} else {
		chan->move_role = L2CAP_MOVE_ROLE_INITIATOR;
		chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		chan->move_id = 0;
		l2cap_move_setup(chan);
		l2cap_send_move_chan_req(chan, 0);
	}
}

static void l2cap_do_create(struct l2cap_chan *chan, int result,
			    u8 local_amp_id, u8 remote_amp_id)
{
	BT_DBG("chan %p state %s %u -> %u", chan, state_to_string(chan->state),
	       local_amp_id, remote_amp_id);

	chan->fcs = L2CAP_FCS_NONE;

	/* Outgoing channel on AMP */
	if (chan->state == BT_CONNECT) {
		if (result == L2CAP_CR_SUCCESS) {
			chan->local_amp_id = local_amp_id;
			l2cap_send_create_chan_req(chan, remote_amp_id);
		} else {
			/* Revert to BR/EDR connect */
			l2cap_send_conn_req(chan);
		}

		return;
	}

	/* Incoming channel on AMP */
	if (__l2cap_no_conn_pending(chan)) {
		struct l2cap_conn_rsp rsp;
		char buf[128];
		rsp.scid = cpu_to_le16(chan->dcid);
		rsp.dcid = cpu_to_le16(chan->scid);

		if (result == L2CAP_CR_SUCCESS) {
			/* Send successful response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_SUCCESS);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		} else {
			/* Send negative response */
			rsp.result = __constant_cpu_to_le16(L2CAP_CR_NO_MEM);
			rsp.status = __constant_cpu_to_le16(L2CAP_CS_NO_INFO);
		}

		l2cap_send_cmd(chan->conn, chan->ident, L2CAP_CREATE_CHAN_RSP,
			       sizeof(rsp), &rsp);

		if (result == L2CAP_CR_SUCCESS) {
			__l2cap_state_change(chan, BT_CONFIG);
			set_bit(CONF_REQ_SENT, &chan->conf_state);
			l2cap_send_cmd(chan->conn, l2cap_get_ident(chan->conn),
				       L2CAP_CONF_REQ,
				       l2cap_build_conf_req(chan, buf), buf);
			chan->num_conf_req++;
		}
	}
}

static void l2cap_do_move_initiate(struct l2cap_chan *chan, u8 local_amp_id,
				   u8 remote_amp_id)
{
	l2cap_move_setup(chan);
	chan->move_id = local_amp_id;
	chan->move_state = L2CAP_MOVE_WAIT_RSP;

	l2cap_send_move_chan_req(chan, remote_amp_id);
}

static void l2cap_do_move_respond(struct l2cap_chan *chan, int result)
{
	struct hci_chan *hchan = NULL;

	/* Placeholder - get hci_chan for logical link */

	if (hchan) {
		if (hchan->state == BT_CONNECTED) {
			/* Logical link is ready to go */
			chan->hs_hcon = hchan->conn;
			chan->hs_hcon->l2cap_data = chan->conn;
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			l2cap_send_move_chan_rsp(chan, L2CAP_MR_SUCCESS);

			l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		} else {
			/* Wait for logical link to be ready */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		}
	} else {
		/* Logical link not available */
		l2cap_send_move_chan_rsp(chan, L2CAP_MR_NOT_ALLOWED);
	}
}

static void l2cap_do_move_cancel(struct l2cap_chan *chan, int result)
{
	if (chan->move_role == L2CAP_MOVE_ROLE_RESPONDER) {
		u8 rsp_result;
		if (result == -EINVAL)
			rsp_result = L2CAP_MR_BAD_ID;
		else
			rsp_result = L2CAP_MR_NOT_ALLOWED;

		l2cap_send_move_chan_rsp(chan, rsp_result);
	}

	chan->move_role = L2CAP_MOVE_ROLE_NONE;
	chan->move_state = L2CAP_MOVE_STABLE;

	/* Restart data transmission */
	l2cap_ertm_send(chan);
}

/* Invoke with locked chan */
void __l2cap_physical_cfm(struct l2cap_chan *chan, int result)
{
	u8 local_amp_id = chan->local_amp_id;
	u8 remote_amp_id = chan->remote_amp_id;

	BT_DBG("chan %p, result %d, local_amp_id %d, remote_amp_id %d",
	       chan, result, local_amp_id, remote_amp_id);

	if (chan->state == BT_DISCONN || chan->state == BT_CLOSED) {
		l2cap_chan_unlock(chan);
		return;
	}

	if (chan->state != BT_CONNECTED) {
		l2cap_do_create(chan, result, local_amp_id, remote_amp_id);
	} else if (result != L2CAP_MR_SUCCESS) {
		l2cap_do_move_cancel(chan, result);
	} else {
		switch (chan->move_role) {
		case L2CAP_MOVE_ROLE_INITIATOR:
			l2cap_do_move_initiate(chan, local_amp_id,
					       remote_amp_id);
			break;
		case L2CAP_MOVE_ROLE_RESPONDER:
			l2cap_do_move_respond(chan, result);
			break;
		default:
			l2cap_do_move_cancel(chan, result);
			break;
		}
	}
}

static inline int l2cap_move_channel_req(struct l2cap_conn *conn,
					 struct l2cap_cmd_hdr *cmd,
					 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_req *req = data;
	struct l2cap_move_chan_rsp rsp;
	struct l2cap_chan *chan;
	u16 icid = 0;
	u16 result = L2CAP_MR_NOT_ALLOWED;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	icid = le16_to_cpu(req->icid);

	BT_DBG("icid 0x%4.4x, dest_amp_id %d", icid, req->dest_amp_id);

	if (!enable_hs)
		return -EINVAL;

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		rsp.icid = cpu_to_le16(icid);
		rsp.result = __constant_cpu_to_le16(L2CAP_MR_NOT_ALLOWED);
		l2cap_send_cmd(conn, cmd->ident, L2CAP_MOVE_CHAN_RSP,
			       sizeof(rsp), &rsp);
		return 0;
	}

	chan->ident = cmd->ident;

	if (chan->scid < L2CAP_CID_DYN_START ||
	    chan->chan_policy == BT_CHANNEL_POLICY_BREDR_ONLY ||
	    (chan->mode != L2CAP_MODE_ERTM &&
	     chan->mode != L2CAP_MODE_STREAMING)) {
		result = L2CAP_MR_NOT_ALLOWED;
		goto send_move_response;
	}

	if (chan->local_amp_id == req->dest_amp_id) {
		result = L2CAP_MR_SAME_ID;
		goto send_move_response;
	}

	if (req->dest_amp_id) {
		struct hci_dev *hdev;
		hdev = hci_dev_get(req->dest_amp_id);
		if (!hdev || hdev->dev_type != HCI_AMP ||
		    !test_bit(HCI_UP, &hdev->flags)) {
			if (hdev)
				hci_dev_put(hdev);

			result = L2CAP_MR_BAD_ID;
			goto send_move_response;
		}
		hci_dev_put(hdev);
	}

	/* Detect a move collision.  Only send a collision response
	 * if this side has "lost", otherwise proceed with the move.
	 * The winner has the larger bd_addr.
	 */
	if ((__chan_is_moving(chan) ||
	     chan->move_role != L2CAP_MOVE_ROLE_NONE) &&
	    bacmp(conn->src, conn->dst) > 0) {
		result = L2CAP_MR_COLLISION;
		goto send_move_response;
	}

	chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
	l2cap_move_setup(chan);
	chan->move_id = req->dest_amp_id;
	icid = chan->dcid;

	if (!req->dest_amp_id) {
		/* Moving to BR/EDR */
		if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
			result = L2CAP_MR_PEND;
		} else {
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM;
			result = L2CAP_MR_SUCCESS;
		}
	} else {
		chan->move_state = L2CAP_MOVE_WAIT_PREPARE;
		/* Placeholder - uncomment when amp functions are available */
		/*amp_accept_physical(chan, req->dest_amp_id);*/
		result = L2CAP_MR_PEND;
	}

send_move_response:
	l2cap_send_move_chan_rsp(chan, result);

	l2cap_chan_unlock(chan);

	return 0;
}

static void l2cap_move_continue(struct l2cap_conn *conn, u16 icid, u16 result)
{
	struct l2cap_chan *chan;
	struct hci_chan *hchan = NULL;

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan) {
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);
	if (result == L2CAP_MR_PEND)
		__set_chan_timer(chan, L2CAP_MOVE_ERTX_TIMEOUT);

	switch (chan->move_state) {
	case L2CAP_MOVE_WAIT_LOGICAL_COMP:
		/* Move confirm will be sent when logical link
		 * is complete.
		 */
		chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		break;
	case L2CAP_MOVE_WAIT_RSP_SUCCESS:
		if (result == L2CAP_MR_PEND) {
			break;
		} else if (test_bit(CONN_LOCAL_BUSY,
				    &chan->conn_state)) {
			chan->move_state = L2CAP_MOVE_WAIT_LOCAL_BUSY;
		} else {
			/* Logical link is up or moving to BR/EDR,
			 * proceed with move
			 */
			chan->move_state = L2CAP_MOVE_WAIT_CONFIRM_RSP;
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		}
		break;
	case L2CAP_MOVE_WAIT_RSP:
		/* Moving to AMP */
		if (result == L2CAP_MR_SUCCESS) {
			/* Remote is ready, send confirm immediately
			 * after logical link is ready
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_CFM;
		} else {
			/* Both logical link and move success
			 * are required to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_LOGICAL_COMP;
		}

		/* Placeholder - get hci_chan for logical link */
		if (!hchan) {
			/* Logical link not available */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
			break;
		}

		/* If the logical link is not yet connected, do not
		 * send confirmation.
		 */
		if (hchan->state != BT_CONNECTED)
			break;

		/* Logical link is already ready to go */

		chan->hs_hcon = hchan->conn;
		chan->hs_hcon->l2cap_data = chan->conn;

		if (result == L2CAP_MR_SUCCESS) {
			/* Can confirm now */
			l2cap_send_move_chan_cfm(chan, L2CAP_MC_CONFIRMED);
		} else {
			/* Now only need move success
			 * to confirm
			 */
			chan->move_state = L2CAP_MOVE_WAIT_RSP_SUCCESS;
		}

		l2cap_logical_cfm(chan, hchan, L2CAP_MR_SUCCESS);
		break;
	default:
		/* Any other amp move state means the move failed. */
		chan->move_id = chan->local_amp_id;
		l2cap_move_done(chan);
		l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);
	}

	l2cap_chan_unlock(chan);
}

static void l2cap_move_fail(struct l2cap_conn *conn, u8 ident, u16 icid,
			    u16 result)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_ident(conn, ident);
	if (!chan) {
		/* Could not locate channel, icid is best guess */
		l2cap_send_move_chan_cfm_icid(conn, icid);
		return;
	}

	__clear_chan_timer(chan);

	if (chan->move_role == L2CAP_MOVE_ROLE_INITIATOR) {
		if (result == L2CAP_MR_COLLISION) {
			chan->move_role = L2CAP_MOVE_ROLE_RESPONDER;
		} else {
			/* Cleanup - cancel move */
			chan->move_id = chan->local_amp_id;
			l2cap_move_done(chan);
		}
	}

	l2cap_send_move_chan_cfm(chan, L2CAP_MC_UNCONFIRMED);

	l2cap_chan_unlock(chan);
}

static int l2cap_move_channel_rsp(struct l2cap_conn *conn,
				  struct l2cap_cmd_hdr *cmd,
				  u16 cmd_len, void *data)
{
	struct l2cap_move_chan_rsp *rsp = data;
	u16 icid, result;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);
	result = le16_to_cpu(rsp->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	if (result == L2CAP_MR_SUCCESS || result == L2CAP_MR_PEND)
		l2cap_move_continue(conn, icid, result);
	else
		l2cap_move_fail(conn, cmd->ident, icid, result);

	return 0;
}

static int l2cap_move_channel_confirm(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd,
				      u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm *cfm = data;
	struct l2cap_chan *chan;
	u16 icid, result;

	if (cmd_len != sizeof(*cfm))
		return -EPROTO;

	icid = le16_to_cpu(cfm->icid);
	result = le16_to_cpu(cfm->result);

	BT_DBG("icid 0x%4.4x, result 0x%4.4x", icid, result);

	chan = l2cap_get_chan_by_dcid(conn, icid);
	if (!chan) {
		/* Spec requires a response even if the icid was not found */
		l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);
		return 0;
	}

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM) {
		if (result == L2CAP_MC_CONFIRMED) {
			chan->local_amp_id = chan->move_id;
			if (!chan->local_amp_id)
				__release_logical_link(chan);
		} else {
			chan->move_id = chan->local_amp_id;
		}

		l2cap_move_done(chan);
	}

	l2cap_send_move_chan_cfm_rsp(conn, cmd->ident, icid);

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_move_channel_confirm_rsp(struct l2cap_conn *conn,
						 struct l2cap_cmd_hdr *cmd,
						 u16 cmd_len, void *data)
{
	struct l2cap_move_chan_cfm_rsp *rsp = data;
	struct l2cap_chan *chan;
	u16 icid;

	if (cmd_len != sizeof(*rsp))
		return -EPROTO;

	icid = le16_to_cpu(rsp->icid);

	BT_DBG("icid 0x%4.4x", icid);

	chan = l2cap_get_chan_by_scid(conn, icid);
	if (!chan)
		return 0;

	__clear_chan_timer(chan);

	if (chan->move_state == L2CAP_MOVE_WAIT_CONFIRM_RSP) {
		chan->local_amp_id = chan->move_id;

		if (!chan->local_amp_id && chan->hs_hchan)
			__release_logical_link(chan);

		l2cap_move_done(chan);
	}

	l2cap_chan_unlock(chan);

	return 0;
}

static inline int l2cap_check_conn_param(u16 min, u16 max, u16 latency,
					 u16 to_multiplier)
{
	u16 max_latency;

	if (min > max || min < 6 || max > 3200)
		return -EINVAL;

	if (to_multiplier < 10 || to_multiplier > 3200)
		return -EINVAL;

	if (max >= to_multiplier * 8)
		return -EINVAL;

	max_latency = (to_multiplier * 8 / max) - 1;
	if (latency > 499 || latency > max_latency)
		return -EINVAL;

	return 0;
}

static inline int l2cap_conn_param_update_req(struct l2cap_conn *conn,
					      struct l2cap_cmd_hdr *cmd,
					      u8 *data)
{
	struct hci_conn *hcon = conn->hcon;
	struct l2cap_conn_param_update_req *req;
	struct l2cap_conn_param_update_rsp rsp;
	u16 min, max, latency, to_multiplier, cmd_len;
	int err;

	if (!(hcon->link_mode & HCI_LM_MASTER))
		return -EINVAL;

	cmd_len = __le16_to_cpu(cmd->len);
	if (cmd_len != sizeof(struct l2cap_conn_param_update_req))
		return -EPROTO;

	req = (struct l2cap_conn_param_update_req *) data;
	min		= __le16_to_cpu(req->min);
	max		= __le16_to_cpu(req->max);
	latency		= __le16_to_cpu(req->latency);
	to_multiplier	= __le16_to_cpu(req->to_multiplier);

	BT_DBG("min 0x%4.4x max 0x%4.4x latency: 0x%4.4x Timeout: 0x%4.4x",
	       min, max, latency, to_multiplier);

	memset(&rsp, 0, sizeof(rsp));

	err = l2cap_check_conn_param(min, max, latency, to_multiplier);
	if (err)
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_REJECTED);
	else
		rsp.result = __constant_cpu_to_le16(L2CAP_CONN_PARAM_ACCEPTED);

	l2cap_send_cmd(conn, cmd->ident, L2CAP_CONN_PARAM_UPDATE_RSP,
		       sizeof(rsp), &rsp);

	if (!err)
		hci_le_conn_update(hcon, min, max, latency, to_multiplier);

	return 0;
}

static inline int l2cap_bredr_sig_cmd(struct l2cap_conn *conn,
				      struct l2cap_cmd_hdr *cmd, u16 cmd_len,
				      u8 *data)
{
	int err = 0;

	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}

	return err;
}

static inline int l2cap_le_sig_cmd(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u8 *data)
{
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		return 0;

	case L2CAP_CONN_PARAM_UPDATE_REQ:
		return l2cap_conn_param_update_req(conn, cmd, data);

	case L2CAP_CONN_PARAM_UPDATE_RSP:
		return 0;

	default:
		BT_ERR("Unknown LE signaling command 0x%2.2x", cmd->code);
		return -EINVAL;
	}
}

static inline void l2cap_sig_channel(struct l2cap_conn *conn,
				     struct sk_buff *skb)
{
	u8 *data = skb->data;
	int len = skb->len;
	struct l2cap_cmd_hdr cmd;
	int err;

	l2cap_raw_recv(conn, skb);

	while (len >= L2CAP_CMD_HDR_SIZE) {
		u16 cmd_len;
		memcpy(&cmd, data, L2CAP_CMD_HDR_SIZE);
		data += L2CAP_CMD_HDR_SIZE;
		len  -= L2CAP_CMD_HDR_SIZE;

		cmd_len = le16_to_cpu(cmd.len);

		BT_DBG("code 0x%2.2x len %d id 0x%2.2x", cmd.code, cmd_len,
		       cmd.ident);

		if (cmd_len > len || !cmd.ident) {
			BT_DBG("corrupted command");
			break;
		}

		if (conn->hcon->type == LE_LINK)
			err = l2cap_le_sig_cmd(conn, &cmd, data);
		else
			err = l2cap_bredr_sig_cmd(conn, &cmd, cmd_len, data);

		if (err) {
			struct l2cap_cmd_rej_unk rej;

			BT_ERR("Wrong link type (%d)", err);

			/* FIXME: Map err to a valid reason */
			rej.reason = __constant_cpu_to_le16(L2CAP_REJ_NOT_UNDERSTOOD);
			l2cap_send_cmd(conn, cmd.ident, L2CAP_COMMAND_REJ,
				       sizeof(rej), &rej);
		}

		data += cmd_len;
		len  -= cmd_len;
	}

	kfree_skb(skb);
}

static int l2cap_check_fcs(struct l2cap_chan *chan,  struct sk_buff *skb)
{
	u16 our_fcs, rcv_fcs;
	int hdr_size;

	if (test_bit(FLAG_EXT_CTRL, &chan->flags))
		hdr_size = L2CAP_EXT_HDR_SIZE;
	else
		hdr_size = L2CAP_ENH_HDR_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16) {
		skb_trim(skb, skb->len - L2CAP_FCS_SIZE);
		rcv_fcs = get_unaligned_le16(skb->data + skb->len);
		our_fcs = crc16(0, skb->data - hdr_size, skb->len + hdr_size);

		if (our_fcs != rcv_fcs)
			return -EBADMSG;
	}
	return 0;
}

static void l2cap_send_i_or_rr_or_rnr(struct l2cap_chan *chan)
{
	struct l2cap_ctrl control;

	BT_DBG("chan %p", chan);

	memset(&control, 0, sizeof(control));
	control.sframe = 1;
	control.final = 1;
	control.reqseq = chan->buffer_seq;
	set_bit(CONN_SEND_FBIT, &chan->conn_state);

	if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		control.super = L2CAP_SUPER_RNR;
		l2cap_send_sframe(chan, &control);
	}

	if (test_and_clear_bit(CONN_REMOTE_BUSY, &chan->conn_state) &&
	    chan->unacked_frames > 0)
		__set_retrans_timer(chan);

	/* Send pending iframes */
	l2cap_ertm_send(chan);

	if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state) &&
	    test_bit(CONN_SEND_FBIT, &chan->conn_state)) {
		/* F-bit wasn't sent in an s-frame or i-frame yet, so
		 * send it now.
		 */
		control.super = L2CAP_SUPER_RR;
		l2cap_send_sframe(chan, &control);
	}
}

static void append_skb_frag(struct sk_buff *skb, struct sk_buff *new_frag,
			    struct sk_buff **last_frag)
{
	/* skb->len reflects data in skb as well as all fragments
	 * skb->data_len reflects only data in fragments
	 */
	if (!skb_has_frag_list(skb))
		skb_shinfo(skb)->frag_list = new_frag;

	new_frag->next = NULL;

	(*last_frag)->next = new_frag;
	*last_frag = new_frag;

	skb->len += new_frag->len;
	skb->data_len += new_frag->len;
	skb->truesize += new_frag->truesize;
}

static int l2cap_reassemble_sdu(struct l2cap_chan *chan, struct sk_buff *skb,
				struct l2cap_ctrl *control)
{
	int err = -EINVAL;

	switch (control->sar) {
	case L2CAP_SAR_UNSEGMENTED:
		if (chan->sdu)
			break;

		err = chan->ops->recv(chan, skb);
		break;

	case L2CAP_SAR_START:
		if (chan->sdu)
			break;

		chan->sdu_len = get_unaligned_le16(skb->data);
		skb_pull(skb, L2CAP_SDULEN_SIZE);

		if (chan->sdu_len > chan->imtu) {
			err = -EMSGSIZE;
			break;
		}

		if (skb->len >= chan->sdu_len)
			break;

		chan->sdu = skb;
		chan->sdu_last_frag = skb;

		skb = NULL;
		err = 0;
		break;

	case L2CAP_SAR_CONTINUE:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len >= chan->sdu_len)
			break;

		err = 0;
		break;

	case L2CAP_SAR_END:
		if (!chan->sdu)
			break;

		append_skb_frag(chan->sdu, skb,
				&chan->sdu_last_frag);
		skb = NULL;

		if (chan->sdu->len != chan->sdu_len)
			break;

		err = chan->ops->recv(chan, chan->sdu);

		if (!err) {
			/* Reassembly complete */
			chan->sdu = NULL;
			chan->sdu_last_frag = NULL;
			chan->sdu_len = 0;
		}
		break;
	}

	if (err) {
		kfree_skb(skb);
		kfree_skb(chan->sdu);
		chan->sdu = NULL;
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;
	}

	return err;
}

static int l2cap_resegment(struct l2cap_chan *chan)
{
	/* Placeholder */
	return 0;
}

void l2cap_chan_busy(struct l2cap_chan *chan, int busy)
{
	u8 event;

	if (chan->mode != L2CAP_MODE_ERTM)
		return;

	event = busy ? L2CAP_EV_LOCAL_BUSY_DETECTED : L2CAP_EV_LOCAL_BUSY_CLEAR;
	l2cap_tx(chan, NULL, NULL, event);
}

static int l2cap_rx_queued_iframes(struct l2cap_chan *chan)
{
	int err = 0;
	/* Pass sequential frames to l2cap_reassemble_sdu()
	 * until a gap is encountered.
	 */

	BT_DBG("chan %p", chan);

	while (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
		struct sk_buff *skb;
		BT_DBG("Searching for skb with txseq %d (queue len %d)",
		       chan->buffer_seq, skb_queue_len(&chan->srej_q));

		skb = l2cap_ertm_seq_in_queue(&chan->srej_q, chan->buffer_seq);

		if (!skb)
			break;

		skb_unlink(skb, &chan->srej_q);
		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);
		err = l2cap_reassemble_sdu(chan, skb, &bt_cb(skb)->control);
		if (err)
			break;
	}

	if (skb_queue_empty(&chan->srej_q)) {
		chan->rx_state = L2CAP_RX_STATE_RECV;
		l2cap_send_ack(chan);
	}

	return err;
}

static void l2cap_handle_srej(struct l2cap_chan *chan,
			      struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (skb == NULL) {
		BT_DBG("Seq %d not available for retransmission",
		       control->reqseq);
		return;
	}

	if (chan->max_tx != 0 && bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	if (control->poll) {
		l2cap_pass_to_tx(chan, control);

		set_bit(CONN_SEND_FBIT, &chan->conn_state);
		l2cap_retransmit(chan, control);
		l2cap_ertm_send(chan);

		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
			set_bit(CONN_SREJ_ACT, &chan->conn_state);
			chan->srej_save_reqseq = control->reqseq;
		}
	} else {
		l2cap_pass_to_tx_fbit(chan, control);

		if (control->final) {
			if (chan->srej_save_reqseq != control->reqseq ||
			    !test_and_clear_bit(CONN_SREJ_ACT,
						&chan->conn_state))
				l2cap_retransmit(chan, control);
		} else {
			l2cap_retransmit(chan, control);
			if (chan->tx_state == L2CAP_TX_STATE_WAIT_F) {
				set_bit(CONN_SREJ_ACT, &chan->conn_state);
				chan->srej_save_reqseq = control->reqseq;
			}
		}
	}
}

static void l2cap_handle_rej(struct l2cap_chan *chan,
			     struct l2cap_ctrl *control)
{
	struct sk_buff *skb;

	BT_DBG("chan %p, control %p", chan, control);

	if (control->reqseq == chan->next_tx_seq) {
		BT_DBG("Invalid reqseq %d, disconnecting", control->reqseq);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	skb = l2cap_ertm_seq_in_queue(&chan->tx_q, control->reqseq);

	if (chan->max_tx && skb &&
	    bt_cb(skb)->control.retries >= chan->max_tx) {
		BT_DBG("Retry limit exceeded (%d)", chan->max_tx);
		l2cap_send_disconn_req(chan, ECONNRESET);
		return;
	}

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	l2cap_pass_to_tx(chan, control);

	if (control->final) {
		if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state))
			l2cap_retransmit_all(chan, control);
	} else {
		l2cap_retransmit_all(chan, control);
		l2cap_ertm_send(chan);
		if (chan->tx_state == L2CAP_TX_STATE_WAIT_F)
			set_bit(CONN_REJ_ACT, &chan->conn_state);
	}
}

static u8 l2cap_classify_txseq(struct l2cap_chan *chan, u16 txseq)
{
	BT_DBG("chan %p, txseq %d", chan, txseq);

	BT_DBG("last_acked_seq %d, expected_tx_seq %d", chan->last_acked_seq,
	       chan->expected_tx_seq);

	if (chan->rx_state == L2CAP_RX_STATE_SREJ_SENT) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			/* See notes below regarding "double poll" and
			 * invalid packets.
			 */
			if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
				BT_DBG("Invalid/Ignore - after SREJ");
				return L2CAP_TXSEQ_INVALID_IGNORE;
			} else {
				BT_DBG("Invalid - in window after SREJ sent");
				return L2CAP_TXSEQ_INVALID;
			}
		}

		if (chan->srej_list.head == txseq) {
			BT_DBG("Expected SREJ");
			return L2CAP_TXSEQ_EXPECTED_SREJ;
		}

		if (l2cap_ertm_seq_in_queue(&chan->srej_q, txseq)) {
			BT_DBG("Duplicate SREJ - txseq already stored");
			return L2CAP_TXSEQ_DUPLICATE_SREJ;
		}

		if (l2cap_seq_list_contains(&chan->srej_list, txseq)) {
			BT_DBG("Unexpected SREJ - not requested");
			return L2CAP_TXSEQ_UNEXPECTED_SREJ;
		}
	}

	if (chan->expected_tx_seq == txseq) {
		if (__seq_offset(chan, txseq, chan->last_acked_seq) >=
		    chan->tx_win) {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		} else {
			BT_DBG("Expected");
			return L2CAP_TXSEQ_EXPECTED;
		}
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) <
	    __seq_offset(chan, chan->expected_tx_seq, chan->last_acked_seq)) {
		BT_DBG("Duplicate - expected_tx_seq later than txseq");
		return L2CAP_TXSEQ_DUPLICATE;
	}

	if (__seq_offset(chan, txseq, chan->last_acked_seq) >= chan->tx_win) {
		/* A source of invalid packets is a "double poll" condition,
		 * where delays cause us to send multiple poll packets.  If
		 * the remote stack receives and processes both polls,
		 * sequence numbers can wrap around in such a way that a
		 * resent frame has a sequence number that looks like new data
		 * with a sequence gap.  This would trigger an erroneous SREJ
		 * request.
		 *
		 * Fortunately, this is impossible with a tx window that's
		 * less than half of the maximum sequence number, which allows
		 * invalid frames to be safely ignored.
		 *
		 * With tx window sizes greater than half of the tx window
		 * maximum, the frame is invalid and cannot be ignored.  This
		 * causes a disconnect.
		 */

		if (chan->tx_win <= ((chan->tx_win_max + 1) >> 1)) {
			BT_DBG("Invalid/Ignore - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID_IGNORE;
		} else {
			BT_DBG("Invalid - txseq outside tx window");
			return L2CAP_TXSEQ_INVALID;
		}
	} else {
		BT_DBG("Unexpected - txseq indicates missing frames");
		return L2CAP_TXSEQ_UNEXPECTED;
	}
}

static int l2cap_rx_state_recv(struct l2cap_chan *chan,
			       struct l2cap_ctrl *control,
			       struct sk_buff *skb, u8 event)
{
	int err = 0;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, control->txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			l2cap_pass_to_tx(chan, control);

			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding expected seq %d",
				       control->txseq);
				break;
			}

			chan->expected_tx_seq = __next_seq(chan,
							   control->txseq);

			chan->buffer_seq = chan->expected_tx_seq;
			skb_in_use = 1;

			err = l2cap_reassemble_sdu(chan, skb, control);
			if (err)
				break;

			if (control->final) {
				if (!test_and_clear_bit(CONN_REJ_ACT,
							&chan->conn_state)) {
					control->final = 0;
					l2cap_retransmit_all(chan, control);
					l2cap_ertm_send(chan);
				}
			}

			if (!test_bit(CONN_LOCAL_BUSY, &chan->conn_state))
				l2cap_send_ack(chan);
			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			l2cap_pass_to_tx(chan, control);

			/* Can't issue SREJ frames in the local busy state.
			 * Drop this frame, it will be seen as missing
			 * when local busy is exited.
			 */
			if (test_bit(CONN_LOCAL_BUSY, &chan->conn_state)) {
				BT_DBG("Busy, discarding unexpected seq %d",
				       control->txseq);
				break;
			}

			/* There was a gap in the sequence, so an SREJ
			 * must be sent for each missing frame.  The
			 * current frame is stored for later use.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			clear_bit(CONN_SREJ_ACT, &chan->conn_state);
			l2cap_seq_list_clear(&chan->srej_list);
			l2cap_send_srej(chan, control->txseq);

			chan->rx_state = L2CAP_RX_STATE_SREJ_SENT;
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT, &chan->conn_state) &&
			    !__chan_is_moving(chan)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			l2cap_send_i_or_rr_or_rnr(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_ertm_send(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control && control->poll) {
			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_rr_or_rnr(chan, 0);
		}
		__clear_retrans_timer(chan);
		l2cap_seq_list_clear(&chan->retrans_list);
		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	default:
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_rx_state_srej_sent(struct l2cap_chan *chan,
				    struct l2cap_ctrl *control,
				    struct sk_buff *skb, u8 event)
{
	int err = 0;
	u16 txseq = control->txseq;
	bool skb_in_use = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	switch (event) {
	case L2CAP_EV_RECV_IFRAME:
		switch (l2cap_classify_txseq(chan, txseq)) {
		case L2CAP_TXSEQ_EXPECTED:
			/* Keep frame for reassembly later */
			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			chan->expected_tx_seq = __next_seq(chan, txseq);
			break;
		case L2CAP_TXSEQ_EXPECTED_SREJ:
			l2cap_seq_list_pop(&chan->srej_list);

			l2cap_pass_to_tx(chan, control);
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			err = l2cap_rx_queued_iframes(chan);
			if (err)
				break;

			break;
		case L2CAP_TXSEQ_UNEXPECTED:
			/* Got a frame that can't be reassembled yet.
			 * Save it for later, and send SREJs to cover
			 * the missing frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_UNEXPECTED_SREJ:
			/* This frame was requested with an SREJ, but
			 * some expected retransmitted frames are
			 * missing.  Request retransmission of missing
			 * SREJ'd frames.
			 */
			skb_queue_tail(&chan->srej_q, skb);
			skb_in_use = 1;
			BT_DBG("Queued %p (queue len %d)", skb,
			       skb_queue_len(&chan->srej_q));

			l2cap_pass_to_tx(chan, control);
			l2cap_send_srej_list(chan, control->txseq);
			break;
		case L2CAP_TXSEQ_DUPLICATE_SREJ:
			/* We've already queued this frame.  Drop this copy. */
			l2cap_pass_to_tx(chan, control);
			break;
		case L2CAP_TXSEQ_DUPLICATE:
			/* Expecting a later sequence number, so this frame
			 * was already received.  Ignore it completely.
			 */
			break;
		case L2CAP_TXSEQ_INVALID_IGNORE:
			break;
		case L2CAP_TXSEQ_INVALID:
		default:
			l2cap_send_disconn_req(chan, ECONNRESET);
			break;
		}
		break;
	case L2CAP_EV_RECV_RR:
		l2cap_pass_to_tx(chan, control);
		if (control->final) {
			clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

			if (!test_and_clear_bit(CONN_REJ_ACT,
						&chan->conn_state)) {
				control->final = 0;
				l2cap_retransmit_all(chan, control);
			}

			l2cap_ertm_send(chan);
		} else if (control->poll) {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames) {
				__set_retrans_timer(chan);
			}

			set_bit(CONN_SEND_FBIT, &chan->conn_state);
			l2cap_send_srej_tail(chan);
		} else {
			if (test_and_clear_bit(CONN_REMOTE_BUSY,
					       &chan->conn_state) &&
			    chan->unacked_frames)
				__set_retrans_timer(chan);

			l2cap_send_ack(chan);
		}
		break;
	case L2CAP_EV_RECV_RNR:
		set_bit(CONN_REMOTE_BUSY, &chan->conn_state);
		l2cap_pass_to_tx(chan, control);
		if (control->poll) {
			l2cap_send_srej_tail(chan);
		} else {
			struct l2cap_ctrl rr_control;
			memset(&rr_control, 0, sizeof(rr_control));
			rr_control.sframe = 1;
			rr_control.super = L2CAP_SUPER_RR;
			rr_control.reqseq = chan->buffer_seq;
			l2cap_send_sframe(chan, &rr_control);
		}

		break;
	case L2CAP_EV_RECV_REJ:
		l2cap_handle_rej(chan, control);
		break;
	case L2CAP_EV_RECV_SREJ:
		l2cap_handle_srej(chan, control);
		break;
	}

	if (skb && !skb_in_use) {
		BT_DBG("Freeing %p", skb);
		kfree_skb(skb);
	}

	return err;
}

static int l2cap_finish_move(struct l2cap_chan *chan)
{
	BT_DBG("chan %p", chan);

	chan->rx_state = L2CAP_RX_STATE_RECV;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	return l2cap_resegment(chan);
}

static int l2cap_rx_state_wait_p(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	BT_DBG("chan %p, control %p, skb %p, event %d", chan, control, skb,
	       event);

	if (!control->poll)
		return -EPROTO;

	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	err = l2cap_finish_move(chan);
	if (err)
		return err;

	set_bit(CONN_SEND_FBIT, &chan->conn_state);
	l2cap_send_i_or_rr_or_rnr(chan);

	if (event == L2CAP_EV_RECV_IFRAME)
		return -EPROTO;

	return l2cap_rx_state_recv(chan, control, NULL, event);
}

static int l2cap_rx_state_wait_f(struct l2cap_chan *chan,
				 struct l2cap_ctrl *control,
				 struct sk_buff *skb, u8 event)
{
	int err;

	if (!control->final)
		return -EPROTO;

	clear_bit(CONN_REMOTE_BUSY, &chan->conn_state);

	chan->rx_state = L2CAP_RX_STATE_RECV;
	l2cap_process_reqseq(chan, control->reqseq);

	if (!skb_queue_empty(&chan->tx_q))
		chan->tx_send_head = skb_peek(&chan->tx_q);
	else
		chan->tx_send_head = NULL;

	/* Rewind next_tx_seq to the point expected
	 * by the receiver.
	 */
	chan->next_tx_seq = control->reqseq;
	chan->unacked_frames = 0;

	if (chan->hs_hcon)
		chan->conn->mtu = chan->hs_hcon->hdev->block_mtu;
	else
		chan->conn->mtu = chan->conn->hcon->hdev->acl_mtu;

	err = l2cap_resegment(chan);

	if (!err)
		err = l2cap_rx_state_recv(chan, control, skb, event);

	return err;
}

static bool __valid_reqseq(struct l2cap_chan *chan, u16 reqseq)
{
	/* Make sure reqseq is for a packet that has been sent but not acked */
	u16 unacked;

	unacked = __seq_offset(chan, chan->next_tx_seq, chan->expected_ack_seq);
	return __seq_offset(chan, chan->next_tx_seq, reqseq) <= unacked;
}

static int l2cap_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
		    struct sk_buff *skb, u8 event)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, event %d, state %d", chan,
	       control, skb, event, chan->rx_state);

	if (__valid_reqseq(chan, control->reqseq)) {
		switch (chan->rx_state) {
		case L2CAP_RX_STATE_RECV:
			err = l2cap_rx_state_recv(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_SREJ_SENT:
			err = l2cap_rx_state_srej_sent(chan, control, skb,
						       event);
			break;
		case L2CAP_RX_STATE_WAIT_P:
			err = l2cap_rx_state_wait_p(chan, control, skb, event);
			break;
		case L2CAP_RX_STATE_WAIT_F:
			err = l2cap_rx_state_wait_f(chan, control, skb, event);
			break;
		default:
			/* shut it down */
			break;
		}
	} else {
		BT_DBG("Invalid reqseq %d (next_tx_seq %d, expected_ack_seq %d",
		       control->reqseq, chan->next_tx_seq,
		       chan->expected_ack_seq);
		l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return err;
}

static int l2cap_stream_rx(struct l2cap_chan *chan, struct l2cap_ctrl *control,
			   struct sk_buff *skb)
{
	int err = 0;

	BT_DBG("chan %p, control %p, skb %p, state %d", chan, control, skb,
	       chan->rx_state);

	if (l2cap_classify_txseq(chan, control->txseq) ==
	    L2CAP_TXSEQ_EXPECTED) {
		l2cap_pass_to_tx(chan, control);

		BT_DBG("buffer_seq %d->%d", chan->buffer_seq,
		       __next_seq(chan, chan->buffer_seq));

		chan->buffer_seq = __next_seq(chan, chan->buffer_seq);

		l2cap_reassemble_sdu(chan, skb, control);
	} else {
		if (chan->sdu) {
			kfree_skb(chan->sdu);
			chan->sdu = NULL;
		}
		chan->sdu_last_frag = NULL;
		chan->sdu_len = 0;

		if (skb) {
			BT_DBG("Freeing %p", skb);
			kfree_skb(skb);
		}
	}

	chan->last_acked_seq = control->txseq;
	chan->expected_tx_seq = __next_seq(chan, control->txseq);

	return err;
}

static int l2cap_data_rcv(struct l2cap_chan *chan, struct sk_buff *skb)
{
	struct l2cap_ctrl *control = &bt_cb(skb)->control;
	u16 len;
	u8 event;

	__unpack_control(chan, skb);

	len = skb->len;

	/*
	 * We can just drop the corrupted I-frame here.
	 * Receiver will miss it and start proper recovery
	 * procedures and ask for retransmission.
	 */
	if (l2cap_check_fcs(chan, skb))
		goto drop;

	if (!control->sframe && control->sar == L2CAP_SAR_START)
		len -= L2CAP_SDULEN_SIZE;

	if (chan->fcs == L2CAP_FCS_CRC16)
		len -= L2CAP_FCS_SIZE;

	if (len > chan->mps) {
		l2cap_send_disconn_req(chan, ECONNRESET);
		goto drop;
	}

	if (!control->sframe) {
		int err;

		BT_DBG("iframe sar %d, reqseq %d, final %d, txseq %d",
		       control->sar, control->reqseq, control->final,
		       control->txseq);

		/* Validate F-bit - F=0 always valid, F=1 only
		 * valid in TX WAIT_F
		 */
		if (control->final && chan->tx_state != L2CAP_TX_STATE_WAIT_F)
			goto drop;

		if (chan->mode != L2CAP_MODE_STREAMING) {
			event = L2CAP_EV_RECV_IFRAME;
			err = l2cap_rx(chan, control, skb, event);
		} else {
			err = l2cap_stream_rx(chan, control, skb);
		}

		if (err)
			l2cap_send_disconn_req(chan, ECONNRESET);
	} else {
		const u8 rx_func_to_event[4] = {
			L2CAP_EV_RECV_RR, L2CAP_EV_RECV_REJ,
			L2CAP_EV_RECV_RNR, L2CAP_EV_RECV_SREJ
		};

		/* Only I-frames are expected in streaming mode */
		if (chan->mode == L2CAP_MODE_STREAMING)
			goto drop;

		BT_DBG("sframe reqseq %d, final %d, poll %d, super %d",
		       control->reqseq, control->final, control->poll,
		       control->super);

		if (len != 0) {
			BT_ERR("Trailing bytes: %d in sframe", len);
			l2cap_send_disconn_req(chan, ECONNRESET);
			goto drop;
		}

		/* Validate F and P bits */
		if (control->final && (control->poll ||
				       chan->tx_state != L2CAP_TX_STATE_WAIT_F))
			goto drop;

		event = rx_func_to_event[control->super];
		if (l2cap_rx(chan, control, skb, event))
			l2cap_send_disconn_req(chan, ECONNRESET);
	}

	return 0;

drop:
	kfree_skb(skb);
	return 0;
}

static void l2cap_data_channel(struct l2cap_conn *conn, u16 cid,
			       struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_get_chan_by_scid(conn, cid);
	if (!chan) {
		if (cid == L2CAP_CID_A2MP) {
			chan = a2mp_channel_create(conn, skb);
			if (!chan) {
				kfree_skb(skb);
				return;
			}

			l2cap_chan_lock(chan);
		} else {
			BT_DBG("unknown cid 0x%4.4x", cid);
			/* Drop packet and return */
			kfree_skb(skb);
			return;
		}
	}

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_CONNECTED)
		goto drop;

	switch (chan->mode) {
	case L2CAP_MODE_BASIC:
		/* If socket recv buffers overflows we drop data here
		 * which is *bad* because L2CAP has to be reliable.
		 * But we don't have any other choice. L2CAP doesn't
		 * provide flow control mechanism. */

		if (chan->imtu < skb->len)
			goto drop;

		if (!chan->ops->recv(chan, skb))
			goto done;
		break;

	case L2CAP_MODE_ERTM:
	case L2CAP_MODE_STREAMING:
		l2cap_data_rcv(chan, skb);
		goto done;

	default:
		BT_DBG("chan %p: bad mode 0x%2.2x", chan, chan->mode);
		break;
	}

drop:
	kfree_skb(skb);

done:
	l2cap_chan_unlock(chan);
}

static void l2cap_conless_channel(struct l2cap_conn *conn, __le16 psm,
				  struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_psm(0, psm, conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_att_channel(struct l2cap_conn *conn,
			      struct sk_buff *skb)
{
	struct l2cap_chan *chan;

	chan = l2cap_global_chan_by_scid(0, L2CAP_CID_LE_DATA,
					 conn->src, conn->dst);
	if (!chan)
		goto drop;

	BT_DBG("chan %p, len %d", chan, skb->len);

	if (chan->state != BT_BOUND && chan->state != BT_CONNECTED)
		goto drop;

	if (chan->imtu < skb->len)
		goto drop;

	if (!chan->ops->recv(chan, skb))
		return;

drop:
	kfree_skb(skb);
}

static void l2cap_recv_frame(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct l2cap_hdr *lh = (void *) skb->data;
	u16 cid, len;
	__le16 psm;

	skb_pull(skb, L2CAP_HDR_SIZE);
	cid = __le16_to_cpu(lh->cid);
	len = __le16_to_cpu(lh->len);

	if (len != skb->len) {
		kfree_skb(skb);
		return;
	}

	BT_DBG("len %d, cid 0x%4.4x", len, cid);

	switch (cid) {
	case L2CAP_CID_LE_SIGNALING:
	case L2CAP_CID_SIGNALING:
		l2cap_sig_channel(conn, skb);
		break;

	case L2CAP_CID_CONN_LESS:
		psm = get_unaligned((__le16 *) skb->data);
		skb_pull(skb, L2CAP_PSMLEN_SIZE);
		l2cap_conless_channel(conn, psm, skb);
		break;

	case L2CAP_CID_LE_DATA:
		l2cap_att_channel(conn, skb);
		break;

	case L2CAP_CID_SMP:
		if (smp_sig_channel(conn, skb))
			l2cap_conn_del(conn->hcon, EACCES);
		break;

	default:
		l2cap_data_channel(conn, cid, skb);
		break;
	}
}

/* ---- L2CAP interface with lower layer (HCI) ---- */

int l2cap_connect_ind(struct hci_dev *hdev, bdaddr_t *bdaddr)
{
	int exact = 0, lm1 = 0, lm2 = 0;
	struct l2cap_chan *c;

	BT_DBG("hdev %s, bdaddr %pMR", hdev->name, bdaddr);

	/* Find listening sockets and check their link_mode */
	read_lock(&chan_list_lock);
	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		if (c->state != BT_LISTEN)
			continue;

		if (!bacmp(&bt_sk(sk)->src, &hdev->bdaddr)) {
			lm1 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm1 |= HCI_LM_MASTER;
			exact++;
		} else if (!bacmp(&bt_sk(sk)->src, BDADDR_ANY)) {
			lm2 |= HCI_LM_ACCEPT;
			if (test_bit(FLAG_ROLE_SWITCH, &c->flags))
				lm2 |= HCI_LM_MASTER;
		}
	}
	read_unlock(&chan_list_lock);

	return exact ? lm1 : lm2;
}

void l2cap_connect_cfm(struct hci_conn *hcon, u8 status)
{
	struct l2cap_conn *conn;

	BT_DBG("hcon %p bdaddr %pMR status %d", hcon, &hcon->dst, status);

	if (!status) {
		conn = l2cap_conn_add(hcon);
		if (conn)
			l2cap_conn_ready(conn);
	} else {
		l2cap_conn_del(hcon, bt_to_errno(status));
	}
}

int l2cap_disconn_ind(struct hci_conn *hcon)
{
	struct l2cap_conn *conn = hcon->l2cap_data;

	BT_DBG("hcon %p", hcon);

	if (!conn)
		return HCI_ERROR_REMOTE_USER_TERM;
	return conn->disc_reason;
}

void l2cap_disconn_cfm(struct hci_conn *hcon, u8 reason)
{
	BT_DBG("hcon %p reason %d", hcon, reason);

	l2cap_conn_del(hcon, bt_to_errno(reason));
}

static inline void l2cap_check_encryption(struct l2cap_chan *chan, u8 encrypt)
{
	if (chan->chan_type != L2CAP_CHAN_CONN_ORIENTED)
		return;

	if (encrypt == 0x00) {
		if (chan->sec_level == BT_SECURITY_MEDIUM) {
			__set_chan_timer(chan, L2CAP_ENC_TIMEOUT);
		} else if (chan->sec_level == BT_SECURITY_HIGH)
			l2cap_chan_close(chan, ECONNREFUSED);
	} else {
		if (chan->sec_level == BT_SECURITY_MEDIUM)
			__clear_chan_timer(chan);
	}
}

int l2cap_security_cfm(struct hci_conn *hcon, u8 status, u8 encrypt)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_chan *chan;

	if (!conn)
		return 0;

	BT_DBG("conn %p status 0x%2.2x encrypt %u", conn, status, encrypt);

	if (hcon->type == LE_LINK) {
		if (!status && encrypt)
			smp_distribute_keys(conn, 0);
		cancel_delayed_work(&conn->security_timer);
	}

	mutex_lock(&conn->chan_lock);

	list_for_each_entry(chan, &conn->chan_l, list) {
		l2cap_chan_lock(chan);

		BT_DBG("chan %p scid 0x%4.4x state %s", chan, chan->scid,
		       state_to_string(chan->state));

		if (chan->chan_type == L2CAP_CHAN_CONN_FIX_A2MP) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->scid == L2CAP_CID_LE_DATA) {
			if (!status && encrypt) {
				chan->sec_level = hcon->sec_level;
				l2cap_chan_ready(chan);
			}

			l2cap_chan_unlock(chan);
			continue;
		}

		if (!__l2cap_no_conn_pending(chan)) {
			l2cap_chan_unlock(chan);
			continue;
		}

		if (!status && (chan->state == BT_CONNECTED ||
				chan->state == BT_CONFIG)) {
			struct sock *sk = chan->sk;

			clear_bit(BT_SK_SUSPEND, &bt_sk(sk)->flags);
			sk->sk_state_change(sk);

			l2cap_check_encryption(chan, encrypt);
			l2cap_chan_unlock(chan);
			continue;
		}

		if (chan->state == BT_CONNECT) {
			if (!status) {
				l2cap_start_connection(chan);
			} else {
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
			}
		} else if (chan->state == BT_CONNECT2) {
			struct sock *sk = chan->sk;
			struct l2cap_conn_rsp rsp;
			__u16 res, stat;

			lock_sock(sk);

			if (!status) {
				if (test_bit(BT_SK_DEFER_SETUP,
					     &bt_sk(sk)->flags)) {
					res = L2CAP_CR_PEND;
					stat = L2CAP_CS_AUTHOR_PEND;
					chan->ops->defer(chan);
				} else {
					__l2cap_state_change(chan, BT_CONFIG);
					res = L2CAP_CR_SUCCESS;
					stat = L2CAP_CS_NO_INFO;
				}
			} else {
				__l2cap_state_change(chan, BT_DISCONN);
				__set_chan_timer(chan, L2CAP_DISC_TIMEOUT);
				res = L2CAP_CR_SEC_BLOCK;
				stat = L2CAP_CS_NO_INFO;
			}

			release_sock(sk);

			rsp.scid   = cpu_to_le16(chan->dcid);
			rsp.dcid   = cpu_to_le16(chan->scid);
			rsp.result = cpu_to_le16(res);
			rsp.status = cpu_to_le16(stat);
			l2cap_send_cmd(conn, chan->ident, L2CAP_CONN_RSP,
				       sizeof(rsp), &rsp);

			if (!test_bit(CONF_REQ_SENT, &chan->conf_state) &&
			    res == L2CAP_CR_SUCCESS) {
				char buf[128];
				set_bit(CONF_REQ_SENT, &chan->conf_state);
				l2cap_send_cmd(conn, l2cap_get_ident(conn),
					       L2CAP_CONF_REQ,
					       l2cap_build_conf_req(chan, buf),
					       buf);
				chan->num_conf_req++;
			}
		}

		l2cap_chan_unlock(chan);
	}

	mutex_unlock(&conn->chan_lock);

	return 0;
}

int l2cap_recv_acldata(struct hci_conn *hcon, struct sk_buff *skb, u16 flags)
{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct l2cap_hdr *hdr;
	int len;

	/* For AMP controller do not create l2cap conn */
	if (!conn && hcon->hdev->dev_type != HCI_BREDR)
		goto drop;

	if (!conn)
		conn = l2cap_conn_add(hcon);

	if (!conn)
		goto drop;

	BT_DBG("conn %p len %d flags 0x%x", conn, skb->len, flags);

	switch (flags) {
	case ACL_START:
	case ACL_START_NO_FLUSH:
	case ACL_COMPLETE:
		if (conn->rx_len) {
			BT_ERR("Unexpected start frame (len %d)", skb->len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
		}

		/* Start fragment always begin with Basic L2CAP header */
		if (skb->len < L2CAP_HDR_SIZE) {
			BT_ERR("Frame is too short (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		hdr = (struct l2cap_hdr *) skb->data;
		len = __le16_to_cpu(hdr->len) + L2CAP_HDR_SIZE;

		if (len == skb->len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, skb);
			return 0;
		}

		BT_DBG("Start: total len %d, frag len %d", len, skb->len);

		if (skb->len > len) {
			BT_ERR("Frame is too long (len %d, expected len %d)",
			       skb->len, len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		/* Allocate skb for the complete frame (with header) */
		conn->rx_skb = bt_skb_alloc(len, GFP_KERNEL);
		if (!conn->rx_skb)
			goto drop;

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len = len - skb->len;
		break;

	case ACL_CONT:
		BT_DBG("Cont: frag len %d (expecting %d)", skb->len, conn->rx_len);

		if (!conn->rx_len) {
			BT_ERR("Unexpected continuation frame (len %d)", skb->len);
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		if (skb->len > conn->rx_len) {
			BT_ERR("Fragment is too long (len %d, expected %d)",
			       skb->len, conn->rx_len);
			kfree_skb(conn->rx_skb);
			conn->rx_skb = NULL;
			conn->rx_len = 0;
			l2cap_conn_unreliable(conn, ECOMM);
			goto drop;
		}

		skb_copy_from_linear_data(skb, skb_put(conn->rx_skb, skb->len),
					  skb->len);
		conn->rx_len -= skb->len;

		if (!conn->rx_len) {
			/* Complete frame received */
			l2cap_recv_frame(conn, conn->rx_skb);
			conn->rx_skb = NULL;
		}
		break;
	}

drop:
	kfree_skb(skb);
	return 0;
}

static int l2cap_debugfs_show(struct seq_file *f, void *p)
{
	struct l2cap_chan *c;

	read_lock(&chan_list_lock);

	list_for_each_entry(c, &chan_list, global_l) {
		struct sock *sk = c->sk;

		seq_printf(f, "%pMR %pMR %d %d 0x%4.4x 0x%4.4x %d %d %d %d\n",
			   &bt_sk(sk)->src, &bt_sk(sk)->dst,
			   c->state, __le16_to_cpu(c->psm),
			   c->scid, c->dcid, c->imtu, c->omtu,
			   c->sec_level, c->mode);
	}

	read_unlock(&chan_list_lock);

	return 0;
}

static int l2cap_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, l2cap_debugfs_show, inode->i_private);
}

static const struct file_operations l2cap_debugfs_fops = {
	.open		= l2cap_debugfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *l2cap_debugfs;

int __init l2cap_init(void)
{
	int err;

	err = l2cap_init_sockets();
	if (err < 0)
		return err;

	if (bt_debugfs) {
		l2cap_debugfs = debugfs_create_file("l2cap", 0444, bt_debugfs,
						    NULL, &l2cap_debugfs_fops);
		if (!l2cap_debugfs)
			BT_ERR("Failed to create L2CAP debug file");
	}

	return 0;
}

void l2cap_exit(void)
{
	debugfs_remove(l2cap_debugfs);
	l2cap_cleanup_sockets();
}

module_param(disable_ertm, bool, 0644);
MODULE_PARM_DESC(disable_ertm, "Disable enhanced retransmission mode");
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		l2cap_command_rej(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_REQ:
		err = l2cap_connect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONN_RSP:
	case L2CAP_CREATE_CHAN_RSP:
		err = l2cap_connect_create_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_REQ:
		err = l2cap_config_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CONF_RSP:
		err = l2cap_config_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_REQ:
		err = l2cap_disconnect_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_DISCONN_RSP:
		err = l2cap_disconnect_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_ECHO_REQ:
		l2cap_send_cmd(conn, cmd->ident, L2CAP_ECHO_RSP, cmd_len, data);
		break;

	case L2CAP_ECHO_RSP:
		break;

	case L2CAP_INFO_REQ:
		err = l2cap_information_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_INFO_RSP:
		err = l2cap_information_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_CREATE_CHAN_REQ:
		err = l2cap_create_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_REQ:
		err = l2cap_move_channel_req(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_RSP:
		err = l2cap_move_channel_rsp(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM:
		err = l2cap_move_channel_confirm(conn, cmd, cmd_len, data);
		break;

	case L2CAP_MOVE_CHAN_CFM_RSP:
		err = l2cap_move_channel_confirm_rsp(conn, cmd, cmd_len, data);
		break;

	default:
		BT_ERR("Unknown BR/EDR signaling command 0x%2.2x", cmd->code);
		err = -EINVAL;
		break;
	}

	return err;
}

static inline int l2cap_le_sig_cmd(struct l2cap_conn *conn,
				   struct l2cap_cmd_hdr *cmd, u8 *data)
{
	switch (cmd->code) {
	case L2CAP_COMMAND_REJ:
		return 0;

	case L2CAP_CONN_PARAM_UPDATE_REQ:
		return l2cap_conn_param_update_req(conn, cmd, data);

	case L2CAP_CONN_PARAM_UPDATE_RSP:
		return 0;

	default:
		BT_ERR("Unknown LE signaling command 0x%2.2x", cmd->code);
		return -EINVAL;
	}
}

static inline void l2cap_sig_channel(struct l2cap_conn *conn,
				     struct sk_buff *skb)
{
	u8 *data = skb->data;
	int len = skb->len;
	struct l2cap_cmd_hdr cmd;
	int err;

	l2cap_raw_recv(conn, skb);

	while (len >= L2CAP_CMD_HDR_SIZE) {
		u16 cmd_len;
		memcpy(&cmd, data, L2CAP_CMD_HDR_SIZE);
		data += L2CAP_CMD_HDR_SIZE;
		len  -= L2CAP_CMD_HDR_SIZE;

		cmd_len = le16_to_cpu(cmd.len);

		BT_DBG("code 0x%2.2x len %d id 0x%2.2x", cmd.code, cmd_len,
		       cmd.ident);

		if (cmd_len > len || !cmd.ident) {
			BT_DBG("corrupted command");
			break;
		}