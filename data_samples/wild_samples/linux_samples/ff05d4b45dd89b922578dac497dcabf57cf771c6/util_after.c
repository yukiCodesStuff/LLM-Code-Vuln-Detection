{
	struct ieee802_11_elems *elems;
	const struct element *non_inherit = NULL;
	u8 *nontransmitted_profile;
	int nontransmitted_profile_len = 0;
	size_t scratch_len = params->len;

	elems = kzalloc(sizeof(*elems) + scratch_len, GFP_ATOMIC);
	if (!elems)
		return NULL;
	elems->ie_start = params->start;
	elems->total_len = params->len;
	elems->scratch_len = scratch_len;
	elems->scratch_pos = elems->scratch;

	nontransmitted_profile = elems->scratch_pos;
	nontransmitted_profile_len =
		ieee802_11_find_bssid_profile(params->start, params->len,
					      elems, params->bss,
					      nontransmitted_profile);
	elems->scratch_pos += nontransmitted_profile_len;
	elems->scratch_len -= nontransmitted_profile_len;
	non_inherit = cfg80211_find_ext_elem(WLAN_EID_EXT_NON_INHERITANCE,
					     nontransmitted_profile,
					     nontransmitted_profile_len);

	elems->crc = _ieee802_11_parse_elems_full(params, elems, non_inherit);

	/* Override with nontransmitted profile, if found */
	if (nontransmitted_profile_len) {
		struct ieee80211_elems_parse_params sub = {
			.start = nontransmitted_profile,
			.len = nontransmitted_profile_len,
			.action = params->action,
			.link_id = params->link_id,
		};

		_ieee802_11_parse_elems_full(&sub, elems, NULL);
	}

	if (elems->tim && !elems->parse_error) {
		const struct ieee80211_tim_ie *tim_ie = elems->tim;

		elems->dtim_period = tim_ie->dtim_period;
		elems->dtim_count = tim_ie->dtim_count;
	}

	/* Override DTIM period and count if needed */
	if (elems->bssid_index &&
	    elems->bssid_index_len >=
	    offsetofend(struct ieee80211_bssid_index, dtim_period))
		elems->dtim_period = elems->bssid_index->dtim_period;

	if (elems->bssid_index &&
	    elems->bssid_index_len >=
	    offsetofend(struct ieee80211_bssid_index, dtim_count))
		elems->dtim_count = elems->bssid_index->dtim_count;

	return elems;
}
	if (elems->tim && !elems->parse_error) {
		const struct ieee80211_tim_ie *tim_ie = elems->tim;

		elems->dtim_period = tim_ie->dtim_period;
		elems->dtim_count = tim_ie->dtim_count;
	}

	/* Override DTIM period and count if needed */
	if (elems->bssid_index &&
	    elems->bssid_index_len >=
	    offsetofend(struct ieee80211_bssid_index, dtim_period))
		elems->dtim_period = elems->bssid_index->dtim_period;

	if (elems->bssid_index &&
	    elems->bssid_index_len >=
	    offsetofend(struct ieee80211_bssid_index, dtim_count))
		elems->dtim_count = elems->bssid_index->dtim_count;

	return elems;
}

void ieee80211_regulatory_limit_wmm_params(struct ieee80211_sub_if_data *sdata,
					   struct ieee80211_tx_queue_params
					   *qparam, int ac)
{
	struct ieee80211_chanctx_conf *chanctx_conf;
	const struct ieee80211_reg_rule *rrule;
	const struct ieee80211_wmm_ac *wmm_ac;
	u16 center_freq = 0;

	if (sdata->vif.type != NL80211_IFTYPE_AP &&
	    sdata->vif.type != NL80211_IFTYPE_STATION)
		return;

	rcu_read_lock();
	chanctx_conf = rcu_dereference(sdata->vif.bss_conf.chanctx_conf);
	if (chanctx_conf)
		center_freq = chanctx_conf->def.chan->center_freq;

	if (!center_freq) {
		rcu_read_unlock();
		return;
	}