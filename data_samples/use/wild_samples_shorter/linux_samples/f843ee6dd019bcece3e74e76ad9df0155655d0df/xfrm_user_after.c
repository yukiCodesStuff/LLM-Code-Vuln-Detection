	up = nla_data(rp);
	ulen = xfrm_replay_state_esn_len(up);

	/* Check the overall length and the internal bitmap length to avoid
	 * potential overflow. */
	if (nla_len(rp) < ulen ||
	    xfrm_replay_state_esn_len(replay_esn) != ulen ||
	    replay_esn->bmp_len != up->bmp_len)
		return -EINVAL;

	if (up->replay_window > up->bmp_len * sizeof(__u32) * 8)
		return -EINVAL;