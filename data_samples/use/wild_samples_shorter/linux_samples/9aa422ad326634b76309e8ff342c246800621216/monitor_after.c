	state->probing = false;

	/* Sanity check received domain record */
	if (new_member_cnt > MAX_MON_DOMAIN)
		return;
	if (dlen < dom_rec_len(arrv_dom, 0))
		return;
	if (dlen != dom_rec_len(arrv_dom, new_member_cnt))
		return;