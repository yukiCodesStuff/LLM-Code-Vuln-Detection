
	rate_ie = (void *)cfg80211_find_ie(WLAN_EID_SUPP_RATES, var_pos, len);
	if (rate_ie) {
		if (rate_ie->len > MWIFIEX_SUPPORTED_RATES)
			return;
		memcpy(bss_cfg->rates, rate_ie + 1, rate_ie->len);
		rate_len = rate_ie->len;
	}

	rate_ie = (void *)cfg80211_find_ie(WLAN_EID_EXT_SUPP_RATES,
					   params->beacon.tail,
					   params->beacon.tail_len);
	if (rate_ie) {
		if (rate_ie->len > MWIFIEX_SUPPORTED_RATES - rate_len)
			return;
		memcpy(bss_cfg->rates + rate_len, rate_ie + 1, rate_ie->len);
	}

	return;
}

					    params->beacon.tail_len);
	if (vendor_ie) {
		wmm_ie = vendor_ie;
		if (*(wmm_ie + 1) > sizeof(struct mwifiex_types_wmm_info))
			return;
		memcpy(&bss_cfg->wmm_info, wmm_ie +
		       sizeof(struct ieee_types_header), *(wmm_ie + 1));
		priv->wmm_enabled = 1;
	} else {