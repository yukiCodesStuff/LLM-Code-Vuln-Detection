	if (nla_len(rp) < ulen || xfrm_replay_state_esn_len(replay_esn) != ulen)
		return -EINVAL;

	if (up->replay_window > up->bmp_len * sizeof(__u32) * 8)
		return -EINVAL;

	return 0;
}

static int xfrm_alloc_replay_state_esn(struct xfrm_replay_state_esn **replay_esn,