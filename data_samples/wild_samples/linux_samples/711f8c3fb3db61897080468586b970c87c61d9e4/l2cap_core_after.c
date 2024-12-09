{
	struct l2cap_le_conn_req *req = (struct l2cap_le_conn_req *) data;
	struct l2cap_le_conn_rsp rsp;
	struct l2cap_chan *chan, *pchan;
	u16 dcid, scid, credits, mtu, mps;
	__le16 psm;
	u8 result;

	if (cmd_len != sizeof(*req))
		return -EPROTO;

	scid = __le16_to_cpu(req->scid);
	mtu  = __le16_to_cpu(req->mtu);
	mps  = __le16_to_cpu(req->mps);
	psm  = req->psm;
	dcid = 0;
	credits = 0;

	if (mtu < 23 || mps < 23)
		return -EPROTO;

	BT_DBG("psm 0x%2.2x scid 0x%4.4x mtu %u mps %u", __le16_to_cpu(psm),
	       scid, mtu, mps);

	/* BLUETOOTH CORE SPECIFICATION Version 5.3 | Vol 3, Part A
	 * page 1059:
	 *
	 * Valid range: 0x0001-0x00ff
	 *
	 * Table 4.15: L2CAP_LE_CREDIT_BASED_CONNECTION_REQ SPSM ranges
	 */
	if (!psm || __le16_to_cpu(psm) > L2CAP_PSM_LE_DYN_END) {
		result = L2CAP_CR_LE_BAD_PSM;
		chan = NULL;
		goto response;
	}
	if (mtu < L2CAP_ECRED_MIN_MTU || mps < L2CAP_ECRED_MIN_MPS) {
		result = L2CAP_CR_LE_UNACCEPT_PARAMS;
		goto response;
	}

	psm  = req->psm;

	/* BLUETOOTH CORE SPECIFICATION Version 5.3 | Vol 3, Part A
	 * page 1059:
	 *
	 * Valid range: 0x0001-0x00ff
	 *
	 * Table 4.15: L2CAP_LE_CREDIT_BASED_CONNECTION_REQ SPSM ranges
	 */
	if (!psm || __le16_to_cpu(psm) > L2CAP_PSM_LE_DYN_END) {
		result = L2CAP_CR_LE_BAD_PSM;
		goto response;
	}