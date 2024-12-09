
	if (!test_and_set_bit(HCI_CONN_LE_SMP_PEND, &conn->hcon->flags))
		smp = smp_chan_create(conn);
	else
		smp = conn->smp_chan;

	if (!smp)
		return SMP_UNSPECIFIED;

	smp->preq[0] = SMP_CMD_PAIRING_REQ;
	memcpy(&smp->preq[1], req, sizeof(*req));
	skb_pull(skb, sizeof(*req));