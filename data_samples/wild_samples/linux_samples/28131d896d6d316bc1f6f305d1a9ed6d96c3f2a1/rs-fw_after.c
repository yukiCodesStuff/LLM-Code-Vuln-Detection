{
	u8 fw_chains = 0;

	if (chains & ANT_A)
		fw_chains |= IWL_TLC_MNG_CHAIN_A_MSK;
	if (chains & ANT_B)
		fw_chains |= IWL_TLC_MNG_CHAIN_B_MSK;

	return fw_chains;
}

static u8 rs_fw_sgi_cw_support(struct ieee80211_sta *sta)
{
	struct ieee80211_sta_ht_cap *ht_cap = &sta->ht_cap;
	struct ieee80211_sta_vht_cap *vht_cap = &sta->vht_cap;
	struct ieee80211_sta_he_cap *he_cap = &sta->he_cap;
	u8 supp = 0;

	if (he_cap->has_he)
		return 0;

	if (ht_cap->cap & IEEE80211_HT_CAP_SGI_20)
		supp |= BIT(IWL_TLC_MNG_CH_WIDTH_20MHZ);
	if (ht_cap->cap & IEEE80211_HT_CAP_SGI_40)
		supp |= BIT(IWL_TLC_MNG_CH_WIDTH_40MHZ);
	if (vht_cap->cap & IEEE80211_VHT_CAP_SHORT_GI_80)
		supp |= BIT(IWL_TLC_MNG_CH_WIDTH_80MHZ);
	if (vht_cap->cap & IEEE80211_VHT_CAP_SHORT_GI_160)
		supp |= BIT(IWL_TLC_MNG_CH_WIDTH_160MHZ);

	return supp;
}

static u16 rs_fw_get_config_flags(struct iwl_mvm *mvm,
				  struct ieee80211_sta *sta,
				  struct ieee80211_supported_band *sband)
{
	struct ieee80211_sta_ht_cap *ht_cap = &sta->ht_cap;
	struct ieee80211_sta_vht_cap *vht_cap = &sta->vht_cap;
	struct ieee80211_sta_he_cap *he_cap = &sta->he_cap;
	bool vht_ena = vht_cap->vht_supported;
	u16 flags = 0;

	/* get STBC flags */
	if (mvm->cfg->ht_params->stbc &&
	    (num_of_ant(iwl_mvm_get_valid_tx_ant(mvm)) > 1)) {
		if (he_cap->has_he && he_cap->he_cap_elem.phy_cap_info[2] &
				      IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ)
			flags |= IWL_TLC_MNG_CFG_FLAGS_STBC_MSK;
		else if (vht_cap->cap & IEEE80211_VHT_CAP_RXSTBC_MASK)
			flags |= IWL_TLC_MNG_CFG_FLAGS_STBC_MSK;
		else if (ht_cap->cap & IEEE80211_HT_CAP_RX_STBC)
			flags |= IWL_TLC_MNG_CFG_FLAGS_STBC_MSK;
	}
	if (flags & IWL_TLC_NOTIF_FLAG_RATE) {
		char pretty_rate[100];

	if (iwl_fw_lookup_notif_ver(mvm->fw, DATA_PATH_GROUP,
				    TLC_MNG_UPDATE_NOTIF, 0) < 3) {
		rs_pretty_print_rate_v1(pretty_rate, sizeof(pretty_rate),
					le32_to_cpu(notif->rate));
		IWL_DEBUG_RATE(mvm,
			       "Got rate in old format. Rate: %s. Converting.\n",
			       pretty_rate);
		lq_sta->last_rate_n_flags =
			iwl_new_rate_from_v1(le32_to_cpu(notif->rate));
	} else {
		lq_sta->last_rate_n_flags = le32_to_cpu(notif->rate);
	}
		rs_pretty_print_rate(pretty_rate, sizeof(pretty_rate),
				     lq_sta->last_rate_n_flags);
		IWL_DEBUG_RATE(mvm, "new rate: %s\n", pretty_rate);
	}

	if (flags & IWL_TLC_NOTIF_FLAG_AMSDU && !mvmsta->orig_amsdu_len) {
		u16 size = le32_to_cpu(notif->amsdu_size);
		int i;

		if (sta->max_amsdu_len < size) {
			/*
			 * In debug sta->max_amsdu_len < size
			 * so also check with orig_amsdu_len which holds the
			 * original data before debugfs changed the value
			 */
			WARN_ON(mvmsta->orig_amsdu_len < size);
			goto out;
		}

		mvmsta->amsdu_enabled = le32_to_cpu(notif->amsdu_enabled);
		mvmsta->max_amsdu_len = size;
		sta->max_rc_amsdu_len = mvmsta->max_amsdu_len;

		for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
			if (mvmsta->amsdu_enabled & BIT(i))
				sta->max_tid_amsdu_len[i] =
					iwl_mvm_max_amsdu_size(mvm, sta, i);
			else
				/*
				 * Not so elegant, but this will effectively
				 * prevent AMSDU on this TID
				 */
				sta->max_tid_amsdu_len[i] = 1;
		}

		IWL_DEBUG_RATE(mvm,
			       "AMSDU update. AMSDU size: %d, AMSDU selected size: %d, AMSDU TID bitmap 0x%X\n",
			       le32_to_cpu(notif->amsdu_size), size,
			       mvmsta->amsdu_enabled);
	}
out:
	rcu_read_unlock();
}