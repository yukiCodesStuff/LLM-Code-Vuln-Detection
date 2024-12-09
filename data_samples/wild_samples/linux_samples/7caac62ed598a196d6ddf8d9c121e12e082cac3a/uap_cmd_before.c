{
	struct ieee_types_header *rate_ie;
	int var_offset = offsetof(struct ieee80211_mgmt, u.beacon.variable);
	const u8 *var_pos = params->beacon.head + var_offset;
	int len = params->beacon.head_len - var_offset;
	u8 rate_len = 0;

	rate_ie = (void *)cfg80211_find_ie(WLAN_EID_SUPP_RATES, var_pos, len);
	if (rate_ie) {
		memcpy(bss_cfg->rates, rate_ie + 1, rate_ie->len);
		rate_len = rate_ie->len;
	}
	if (vendor_ie) {
		wmm_ie = vendor_ie;
		memcpy(&bss_cfg->wmm_info, wmm_ie +
		       sizeof(struct ieee_types_header), *(wmm_ie + 1));
		priv->wmm_enabled = 1;
	} else {
		memset(&bss_cfg->wmm_info, 0, sizeof(bss_cfg->wmm_info));
		memcpy(&bss_cfg->wmm_info.oui, wmm_oui, sizeof(wmm_oui));
		bss_cfg->wmm_info.subtype = MWIFIEX_WMM_SUBTYPE;
		bss_cfg->wmm_info.version = MWIFIEX_WMM_VERSION;
		priv->wmm_enabled = 0;
	}

	bss_cfg->qos_info = 0x00;
	return;
}