{
	struct smp_cmd_pairing rsp, *req = (void *) skb->data;
	struct smp_chan *smp;
	u8 key_size;
	u8 auth = SMP_AUTH_NONE;
	int ret;

	BT_DBG("conn %p", conn);

	if (conn->hcon->link_mode & HCI_LM_MASTER)
		return SMP_CMD_NOTSUPP;

	if (!test_and_set_bit(HCI_CONN_LE_SMP_PEND, &conn->hcon->flags))
		smp = smp_chan_create(conn);

	smp = conn->smp_chan;

	smp->preq[0] = SMP_CMD_PAIRING_REQ;
	memcpy(&smp->preq[1], req, sizeof(*req));
	skb_pull(skb, sizeof(*req));

	/* We didn't start the pairing, so match remote */
	if (req->auth_req & SMP_AUTH_BONDING)
		auth = req->auth_req;

	conn->hcon->pending_sec_level = authreq_to_seclevel(auth);

	build_pairing_cmd(conn, req, &rsp, auth);

	key_size = min(req->max_key_size, rsp.max_key_size);
	if (check_enc_key_size(conn, key_size))
		return SMP_ENC_KEY_SIZE;

	ret = smp_rand(smp->prnd);
	if (ret)
		return SMP_UNSPECIFIED;

	smp->prsp[0] = SMP_CMD_PAIRING_RSP;
	memcpy(&smp->prsp[1], &rsp, sizeof(rsp));

	smp_send_cmd(conn, SMP_CMD_PAIRING_RSP, sizeof(rsp), &rsp);

	/* Request setup of TK */
	ret = tk_request(conn, 0, auth, rsp.io_capability, req->io_capability);
	if (ret)
		return SMP_UNSPECIFIED;

	return 0;
}

static u8 smp_cmd_pairing_rsp(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_cmd_pairing *req, *rsp = (void *) skb->data;
	struct smp_chan *smp = conn->smp_chan;
	struct hci_dev *hdev = conn->hcon->hdev;
	u8 key_size, auth = SMP_AUTH_NONE;
	int ret;

	BT_DBG("conn %p", conn);

	if (!(conn->hcon->link_mode & HCI_LM_MASTER))
		return SMP_CMD_NOTSUPP;

	skb_pull(skb, sizeof(*rsp));

	req = (void *) &smp->preq[1];

	key_size = min(req->max_key_size, rsp->max_key_size);
	if (check_enc_key_size(conn, key_size))
		return SMP_ENC_KEY_SIZE;

	ret = smp_rand(smp->prnd);
	if (ret)
		return SMP_UNSPECIFIED;

	smp->prsp[0] = SMP_CMD_PAIRING_RSP;
	memcpy(&smp->prsp[1], rsp, sizeof(*rsp));

	if ((req->auth_req & SMP_AUTH_BONDING) &&
			(rsp->auth_req & SMP_AUTH_BONDING))
		auth = SMP_AUTH_BONDING;

	auth |= (req->auth_req | rsp->auth_req) & SMP_AUTH_MITM;

	ret = tk_request(conn, 0, auth, req->io_capability, rsp->io_capability);
	if (ret)
		return SMP_UNSPECIFIED;

	set_bit(SMP_FLAG_CFM_PENDING, &smp->smp_flags);

	/* Can't compose response until we have been confirmed */
	if (!test_bit(SMP_FLAG_TK_VALID, &smp->smp_flags))
		return 0;

	queue_work(hdev->workqueue, &smp->confirm);

	return 0;
}

static u8 smp_cmd_pairing_confirm(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_chan *smp = conn->smp_chan;
	struct hci_dev *hdev = conn->hcon->hdev;

	BT_DBG("conn %p %s", conn, conn->hcon->out ? "master" : "slave");

	memcpy(smp->pcnf, skb->data, sizeof(smp->pcnf));
	skb_pull(skb, sizeof(smp->pcnf));

	if (conn->hcon->out) {
		u8 random[16];

		swap128(smp->prnd, random);
		smp_send_cmd(conn, SMP_CMD_PAIRING_RANDOM, sizeof(random),
								random);
	} else if (test_bit(SMP_FLAG_TK_VALID, &smp->smp_flags)) {
		queue_work(hdev->workqueue, &smp->confirm);
	} else {
		set_bit(SMP_FLAG_CFM_PENDING, &smp->smp_flags);
	}

	return 0;
}

static u8 smp_cmd_pairing_random(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_chan *smp = conn->smp_chan;
	struct hci_dev *hdev = conn->hcon->hdev;

	BT_DBG("conn %p", conn);

	swap128(skb->data, smp->rrnd);
	skb_pull(skb, sizeof(smp->rrnd));

	queue_work(hdev->workqueue, &smp->random);

	return 0;
}

static u8 smp_ltk_encrypt(struct l2cap_conn *conn, u8 sec_level)
{
	struct smp_ltk *key;
	struct hci_conn *hcon = conn->hcon;

	key = hci_find_ltk_by_addr(hcon->hdev, conn->dst, hcon->dst_type);
	if (!key)
		return 0;

	if (sec_level > BT_SECURITY_MEDIUM && !key->authenticated)
		return 0;

	if (test_and_set_bit(HCI_CONN_ENCRYPT_PEND, &hcon->flags))
		return 1;

	hci_le_start_enc(hcon, key->ediv, key->rand, key->val);
	hcon->enc_key_size = key->enc_size;

	return 1;

}
static u8 smp_cmd_security_req(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_cmd_security_req *rp = (void *) skb->data;
	struct smp_cmd_pairing cp;
	struct hci_conn *hcon = conn->hcon;
	struct smp_chan *smp;

	BT_DBG("conn %p", conn);

	hcon->pending_sec_level = authreq_to_seclevel(rp->auth_req);

	if (smp_ltk_encrypt(conn, hcon->pending_sec_level))
		return 0;

	if (test_and_set_bit(HCI_CONN_LE_SMP_PEND, &hcon->flags))
		return 0;

	smp = smp_chan_create(conn);

	skb_pull(skb, sizeof(*rp));

	memset(&cp, 0, sizeof(cp));
	build_pairing_cmd(conn, &cp, NULL, rp->auth_req);

	smp->preq[0] = SMP_CMD_PAIRING_REQ;
	memcpy(&smp->preq[1], &cp, sizeof(cp));

	smp_send_cmd(conn, SMP_CMD_PAIRING_REQ, sizeof(cp), &cp);

	return 0;
}

int smp_conn_security(struct l2cap_conn *conn, __u8 sec_level)
{
	struct hci_conn *hcon = conn->hcon;
	struct smp_chan *smp = conn->smp_chan;
	__u8 authreq;

	BT_DBG("conn %p hcon %p level 0x%2.2x", conn, hcon, sec_level);

	if (!lmp_host_le_capable(hcon->hdev))
		return 1;

	if (sec_level == BT_SECURITY_LOW)
		return 1;

	if (hcon->sec_level >= sec_level)
		return 1;

	if (hcon->link_mode & HCI_LM_MASTER)
		if (smp_ltk_encrypt(conn, sec_level))
			goto done;

	if (test_and_set_bit(HCI_CONN_LE_SMP_PEND, &hcon->flags))
		return 0;

	smp = smp_chan_create(conn);
	if (!smp)
		return 1;

	authreq = seclevel_to_authreq(sec_level);

	if (hcon->link_mode & HCI_LM_MASTER) {
		struct smp_cmd_pairing cp;

		build_pairing_cmd(conn, &cp, NULL, authreq);
		smp->preq[0] = SMP_CMD_PAIRING_REQ;
		memcpy(&smp->preq[1], &cp, sizeof(cp));

		smp_send_cmd(conn, SMP_CMD_PAIRING_REQ, sizeof(cp), &cp);
	} else {
		struct smp_cmd_security_req cp;
		cp.auth_req = authreq;
		smp_send_cmd(conn, SMP_CMD_SECURITY_REQ, sizeof(cp), &cp);
	}

done:
	hcon->pending_sec_level = sec_level;

	return 0;
}

static int smp_cmd_encrypt_info(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_cmd_encrypt_info *rp = (void *) skb->data;
	struct smp_chan *smp = conn->smp_chan;

	skb_pull(skb, sizeof(*rp));

	memcpy(smp->tk, rp->ltk, sizeof(smp->tk));

	return 0;
}

static int smp_cmd_master_ident(struct l2cap_conn *conn, struct sk_buff *skb)
{
	struct smp_cmd_master_ident *rp = (void *) skb->data;
	struct smp_chan *smp = conn->smp_chan;
	struct hci_dev *hdev = conn->hcon->hdev;
	struct hci_conn *hcon = conn->hcon;
	u8 authenticated;

	skb_pull(skb, sizeof(*rp));

	hci_dev_lock(hdev);
	authenticated = (conn->hcon->sec_level == BT_SECURITY_HIGH);
	hci_add_ltk(conn->hcon->hdev, conn->dst, hcon->dst_type,
		    HCI_SMP_LTK, 1, authenticated, smp->tk, smp->enc_key_size,
		    rp->ediv, rp->rand);
	smp_distribute_keys(conn, 1);
	hci_dev_unlock(hdev);

	return 0;
}

int smp_sig_channel(struct l2cap_conn *conn, struct sk_buff *skb)
{
	__u8 code = skb->data[0];
	__u8 reason;
	int err = 0;

	if (!lmp_host_le_capable(conn->hcon->hdev)) {
		err = -ENOTSUPP;
		reason = SMP_PAIRING_NOTSUPP;
		goto done;
	}

	skb_pull(skb, sizeof(code));

	switch (code) {
	case SMP_CMD_PAIRING_REQ:
		reason = smp_cmd_pairing_req(conn, skb);
		break;

	case SMP_CMD_PAIRING_FAIL:
		smp_failure(conn, skb->data[0], 0);
		reason = 0;
		err = -EPERM;
		break;

	case SMP_CMD_PAIRING_RSP:
		reason = smp_cmd_pairing_rsp(conn, skb);
		break;

	case SMP_CMD_SECURITY_REQ:
		reason = smp_cmd_security_req(conn, skb);
		break;

	case SMP_CMD_PAIRING_CONFIRM:
		reason = smp_cmd_pairing_confirm(conn, skb);
		break;

	case SMP_CMD_PAIRING_RANDOM:
		reason = smp_cmd_pairing_random(conn, skb);
		break;

	case SMP_CMD_ENCRYPT_INFO:
		reason = smp_cmd_encrypt_info(conn, skb);
		break;

	case SMP_CMD_MASTER_IDENT:
		reason = smp_cmd_master_ident(conn, skb);
		break;

	case SMP_CMD_IDENT_INFO:
	case SMP_CMD_IDENT_ADDR_INFO:
	case SMP_CMD_SIGN_INFO:
		/* Just ignored */
		reason = 0;
		break;

	default:
		BT_DBG("Unknown command code 0x%2.2x", code);

		reason = SMP_CMD_NOTSUPP;
		err = -EOPNOTSUPP;
		goto done;
	}

done:
	if (reason)
		smp_failure(conn, reason, 1);

	kfree_skb(skb);
	return err;
}

int smp_distribute_keys(struct l2cap_conn *conn, __u8 force)
{
	struct smp_cmd_pairing *req, *rsp;
	struct smp_chan *smp = conn->smp_chan;
	__u8 *keydist;

	BT_DBG("conn %p force %d", conn, force);

	if (!test_bit(HCI_CONN_LE_SMP_PEND, &conn->hcon->flags))
		return 0;

	rsp = (void *) &smp->prsp[1];

	/* The responder sends its keys first */
	if (!force && conn->hcon->out && (rsp->resp_key_dist & 0x07))
		return 0;

	req = (void *) &smp->preq[1];

	if (conn->hcon->out) {
		keydist = &rsp->init_key_dist;
		*keydist &= req->init_key_dist;
	} else {
		keydist = &rsp->resp_key_dist;
		*keydist &= req->resp_key_dist;
	}


	BT_DBG("keydist 0x%x", *keydist);

	if (*keydist & SMP_DIST_ENC_KEY) {
		struct smp_cmd_encrypt_info enc;
		struct smp_cmd_master_ident ident;
		struct hci_conn *hcon = conn->hcon;
		u8 authenticated;
		__le16 ediv;

		get_random_bytes(enc.ltk, sizeof(enc.ltk));
		get_random_bytes(&ediv, sizeof(ediv));
		get_random_bytes(ident.rand, sizeof(ident.rand));

		smp_send_cmd(conn, SMP_CMD_ENCRYPT_INFO, sizeof(enc), &enc);

		authenticated = hcon->sec_level == BT_SECURITY_HIGH;
		hci_add_ltk(conn->hcon->hdev, conn->dst, hcon->dst_type,
			    HCI_SMP_LTK_SLAVE, 1, authenticated,
			    enc.ltk, smp->enc_key_size, ediv, ident.rand);

		ident.ediv = ediv;

		smp_send_cmd(conn, SMP_CMD_MASTER_IDENT, sizeof(ident), &ident);

		*keydist &= ~SMP_DIST_ENC_KEY;
	}

	if (*keydist & SMP_DIST_ID_KEY) {
		struct smp_cmd_ident_addr_info addrinfo;
		struct smp_cmd_ident_info idinfo;

		/* Send a dummy key */
		get_random_bytes(idinfo.irk, sizeof(idinfo.irk));

		smp_send_cmd(conn, SMP_CMD_IDENT_INFO, sizeof(idinfo), &idinfo);

		/* Just public address */
		memset(&addrinfo, 0, sizeof(addrinfo));
		bacpy(&addrinfo.bdaddr, conn->src);

		smp_send_cmd(conn, SMP_CMD_IDENT_ADDR_INFO, sizeof(addrinfo),
								&addrinfo);

		*keydist &= ~SMP_DIST_ID_KEY;
	}

	if (*keydist & SMP_DIST_SIGN) {
		struct smp_cmd_sign_info sign;

		/* Send a dummy key */
		get_random_bytes(sign.csrk, sizeof(sign.csrk));

		smp_send_cmd(conn, SMP_CMD_SIGN_INFO, sizeof(sign), &sign);

		*keydist &= ~SMP_DIST_SIGN;
	}

	if (conn->hcon->out || force) {
		clear_bit(HCI_CONN_LE_SMP_PEND, &conn->hcon->flags);
		cancel_delayed_work_sync(&conn->security_timer);
		smp_chan_destroy(conn);
	}

	return 0;
}