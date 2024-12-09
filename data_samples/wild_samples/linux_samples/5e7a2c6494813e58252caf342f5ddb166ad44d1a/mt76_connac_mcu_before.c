		len = is_mt7921(dev) ? sizeof(*vht) : sizeof(*vht) - 4;
		tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_VHT, len);
		vht = (struct sta_rec_vht *)tlv;
		vht->vht_cap = cpu_to_le32(sta->vht_cap.cap);
		vht->vht_rx_mcs_map = sta->vht_cap.vht_mcs.rx_mcs_map;
		vht->vht_tx_mcs_map = sta->vht_cap.vht_mcs.tx_mcs_map;
	}

	/* starec uapsd */
	mt76_connac_mcu_sta_uapsd(skb, vif, sta);

	if (!is_mt7921(dev))
		return;

	if (sta->ht_cap.ht_supported)
		mt76_connac_mcu_sta_amsdu_tlv(skb, sta, vif);

	/* starec he */
	if (sta->he_cap.has_he)
		mt76_connac_mcu_sta_he_tlv(skb, sta);

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_PHY, sizeof(*phy));
	phy = (struct sta_rec_phy *)tlv;
	phy->phy_type = mt76_connac_get_phy_mode_v2(mphy, vif, band, sta);
	phy->basic_rate = cpu_to_le16((u16)vif->bss_conf.basic_rates);
	phy->rcpi = rcpi;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_RA, sizeof(*ra_info));
	ra_info = (struct sta_rec_ra_info *)tlv;
	ra_info->legacy = cpu_to_le16((u16)sta->supp_rates[band]);

	if (sta->ht_cap.ht_supported)
		memcpy(ra_info->rx_mcs_bitmask, sta->ht_cap.mcs.rx_mask,
		       HT_MCS_MASK_NUM);

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_STATE, sizeof(*state));
	state = (struct sta_rec_state *)tlv;
	state->state = 2;

	if (sta->vht_cap.vht_supported) {
		state->vht_opmode = sta->bandwidth;
		state->vht_opmode |= (sta->rx_nss - 1) <<
			IEEE80211_OPMODE_NOTIF_RX_NSS_SHIFT;
	}