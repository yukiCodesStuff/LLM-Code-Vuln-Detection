{
	struct nlattr *rp = attrs[XFRMA_REPLAY_VAL];
	struct nlattr *re = update_esn ? attrs[XFRMA_REPLAY_ESN_VAL] : NULL;
	struct nlattr *lt = attrs[XFRMA_LTIME_VAL];
	struct nlattr *et = attrs[XFRMA_ETIMER_THRESH];
	struct nlattr *rt = attrs[XFRMA_REPLAY_THRESH];
	struct nlattr *mt = attrs[XFRMA_MTIMER_THRESH];

	if (re && x->replay_esn && x->preplay_esn) {
		struct xfrm_replay_state_esn *replay_esn;
		replay_esn = nla_data(re);
		memcpy(x->replay_esn, replay_esn,
		       xfrm_replay_state_esn_len(replay_esn));
		memcpy(x->preplay_esn, replay_esn,
		       xfrm_replay_state_esn_len(replay_esn));
	}