	struct tipc_msg *hdr = buf_msg(skb);
	struct tipc_gap_ack_blks *ga = NULL;
	bool reply = msg_probe(hdr), retransmitted = false;
	u32 dlen = msg_data_sz(hdr), glen = 0;
	u16 peers_snd_nxt =  msg_next_sent(hdr);
	u16 peers_tol = msg_link_tolerance(hdr);
	u16 peers_prio = msg_linkprio(hdr);
	u16 gap = msg_seq_gap(hdr);
	void *data;

	trace_tipc_proto_rcv(skb, false, l->name);

	if (dlen > U16_MAX)
		goto exit;

	if (tipc_link_is_blocked(l) || !xmitq)
		goto exit;

	if (tipc_own_addr(l->net) > msg_prevnode(hdr))

		/* Receive Gap ACK blocks from peer if any */
		glen = tipc_get_gap_ack_blks(&ga, l, hdr, true);
		if(glen > dlen)
			break;
		tipc_mon_rcv(l->net, data + glen, dlen - glen, l->addr,
			     &l->mon_state, l->bearer_id);

		/* Send NACK if peer has sent pkts we haven't received yet */