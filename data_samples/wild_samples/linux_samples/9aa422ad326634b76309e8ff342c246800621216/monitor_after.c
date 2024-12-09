{
	struct tipc_monitor *mon = tipc_monitor(net, bearer_id);
	struct tipc_mon_domain *arrv_dom = data;
	struct tipc_mon_domain dom_bef;
	struct tipc_mon_domain *dom;
	struct tipc_peer *peer;
	u16 new_member_cnt = mon_le16_to_cpu(arrv_dom->member_cnt);
	int new_dlen = dom_rec_len(arrv_dom, new_member_cnt);
	u16 new_gen = mon_le16_to_cpu(arrv_dom->gen);
	u16 acked_gen = mon_le16_to_cpu(arrv_dom->ack_gen);
	u16 arrv_dlen = mon_le16_to_cpu(arrv_dom->len);
	bool probing = state->probing;
	int i, applied_bef;

	state->probing = false;

	/* Sanity check received domain record */
	if (new_member_cnt > MAX_MON_DOMAIN)
		return;
	if (dlen < dom_rec_len(arrv_dom, 0))
		return;
	if (dlen != dom_rec_len(arrv_dom, new_member_cnt))
		return;
	if (dlen < new_dlen || arrv_dlen != new_dlen)
		return;

	/* Synch generation numbers with peer if link just came up */
	if (!state->synched) {
		state->peer_gen = new_gen - 1;
		state->acked_gen = acked_gen;
		state->synched = true;
	}