{
	struct xfrm_replay_state_esn *up;
	int ulen;

	if (!replay_esn || !rp)
		return 0;

	up = nla_data(rp);
	ulen = xfrm_replay_state_esn_len(up);

	if (nla_len(rp) < ulen || xfrm_replay_state_esn_len(replay_esn) != ulen)
		return -EINVAL;

	if (up->replay_window > up->bmp_len * sizeof(__u32) * 8)
		return -EINVAL;

	return 0;
}

static int xfrm_alloc_replay_state_esn(struct xfrm_replay_state_esn **replay_esn,
				       struct xfrm_replay_state_esn **preplay_esn,
				       struct nlattr *rta)
{
	struct xfrm_replay_state_esn *p, *pp, *up;
	int klen, ulen;

	if (!rta)
		return 0;

	up = nla_data(rta);
	klen = xfrm_replay_state_esn_len(up);
	ulen = nla_len(rta) >= klen ? klen : sizeof(*up);

	p = kzalloc(klen, GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	pp = kzalloc(klen, GFP_KERNEL);
	if (!pp) {
		kfree(p);
		return -ENOMEM;
	}