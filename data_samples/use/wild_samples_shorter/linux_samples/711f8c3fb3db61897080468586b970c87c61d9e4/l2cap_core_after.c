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

	/* Check if we have socket listening on psm */
	pchan = l2cap_global_chan_by_psm(BT_LISTEN, psm, &conn->hcon->src,
					 &conn->hcon->dst, LE_LINK);
	if (!pchan) {

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

	BT_DBG("psm 0x%2.2x mtu %u mps %u", __le16_to_cpu(psm), mtu, mps);

	memset(&pdu, 0, sizeof(pdu));
