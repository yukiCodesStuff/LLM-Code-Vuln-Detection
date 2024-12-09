
	if (!test_and_set_bit(HCI_CONN_LE_SMP_PEND, &conn->hcon->flags))
		smp = smp_chan_create(conn);

	smp = conn->smp_chan;

	smp->preq[0] = SMP_CMD_PAIRING_REQ;
	memcpy(&smp->preq[1], req, sizeof(*req));
	skb_pull(skb, sizeof(*req));