{
	struct l2cap_conn *conn = hcon->l2cap_data;
	struct smp_chan *smp = conn->smp_chan;
	__u8 authreq;

	BT_DBG("conn %p hcon %p level 0x%2.2x", conn, hcon, sec_level);

	if (!test_bit(HCI_LE_ENABLED, &hcon->hdev->dev_flags))
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
{
	__u8 code = skb->data[0];
	__u8 reason;
	int err = 0;

	if (!test_bit(HCI_LE_ENABLED, &conn->hcon->hdev->dev_flags)) {
		err = -ENOTSUPP;
		reason = SMP_PAIRING_NOTSUPP;
		goto done;
	}