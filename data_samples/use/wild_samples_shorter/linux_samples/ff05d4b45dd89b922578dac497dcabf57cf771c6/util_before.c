	const struct element *non_inherit = NULL;
	u8 *nontransmitted_profile;
	int nontransmitted_profile_len = 0;

	elems = kzalloc(sizeof(*elems), GFP_ATOMIC);
	if (!elems)
		return NULL;
	elems->ie_start = params->start;
	elems->total_len = params->len;

	nontransmitted_profile = kmalloc(params->len, GFP_ATOMIC);
	if (nontransmitted_profile) {
		nontransmitted_profile_len =
			ieee802_11_find_bssid_profile(params->start, params->len,
						      elems, params->bss,
						      nontransmitted_profile);
		non_inherit =
			cfg80211_find_ext_elem(WLAN_EID_EXT_NON_INHERITANCE,
					       nontransmitted_profile,
					       nontransmitted_profile_len);
	}

	elems->crc = _ieee802_11_parse_elems_full(params, elems, non_inherit);

	/* Override with nontransmitted profile, if found */
	    offsetofend(struct ieee80211_bssid_index, dtim_count))
		elems->dtim_count = elems->bssid_index->dtim_count;

	kfree(nontransmitted_profile);

	return elems;
}

void ieee80211_regulatory_limit_wmm_params(struct ieee80211_sub_if_data *sdata,