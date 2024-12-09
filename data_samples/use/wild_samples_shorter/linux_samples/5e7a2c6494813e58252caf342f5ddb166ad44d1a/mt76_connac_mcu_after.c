	phy->phy_type = mt76_connac_get_phy_mode_v2(mphy, vif, band, sta);
	phy->basic_rate = cpu_to_le16((u16)vif->bss_conf.basic_rates);
	phy->rcpi = rcpi;
	phy->ampdu = FIELD_PREP(IEEE80211_HT_AMPDU_PARM_FACTOR,
				sta->ht_cap.ampdu_factor) |
		     FIELD_PREP(IEEE80211_HT_AMPDU_PARM_DENSITY,
				sta->ht_cap.ampdu_density);

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_RA, sizeof(*ra_info));
	ra_info = (struct sta_rec_ra_info *)tlv;
	ra_info->legacy = cpu_to_le16((u16)sta->supp_rates[band]);