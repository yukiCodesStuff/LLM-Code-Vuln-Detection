	hw->vif_data_size = sizeof(struct mt7921_vif);

	wiphy->iface_combinations = if_comb;
	wiphy->flags &= ~WIPHY_FLAG_IBSS_RSN;
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	wiphy->n_iface_combinations = ARRAY_SIZE(if_comb);
	wiphy->max_scan_ie_len = MT76_CONNAC_SCAN_IE_LEN;
	wiphy->max_scan_ssids = 4;