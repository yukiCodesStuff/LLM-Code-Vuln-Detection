{
	u32 tx_ant;

	iwl_mvm_toggle_tx_ant(mvm, &mvm->scan_last_antenna_idx);
	tx_ant = BIT(mvm->scan_last_antenna_idx) << RATE_MCS_ANT_POS;

	if (band == NL80211_BAND_2GHZ && !no_cck)
		return cpu_to_le32(IWL_RATE_1M_PLCP | RATE_MCS_CCK_MSK |
				   tx_ant);
	else
		return cpu_to_le32(IWL_RATE_6M_PLCP | tx_ant);
}

static void iwl_mvm_scan_condition_iterator(void *data, u8 *mac,
					    struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	int *global_cnt = data;

	if (vif->type != NL80211_IFTYPE_P2P_DEVICE && mvmvif->phy_ctxt &&
	    mvmvif->phy_ctxt->id < NUM_PHY_CTX)
		*global_cnt += 1;
}

static enum iwl_mvm_traffic_load iwl_mvm_get_traffic_load(struct iwl_mvm *mvm)
{
	return mvm->tcm.result.global_load;
}

static enum iwl_mvm_traffic_load
iwl_mvm_get_traffic_load_band(struct iwl_mvm *mvm, enum nl80211_band band)
{
	return mvm->tcm.result.band_load[band];
}

struct iwl_is_dcm_with_go_iterator_data {
	struct ieee80211_vif *current_vif;
	bool is_dcm_with_p2p_go;
};

static void iwl_mvm_is_dcm_with_go_iterator(void *_data, u8 *mac,
					    struct ieee80211_vif *vif)
{
	struct iwl_is_dcm_with_go_iterator_data *data = _data;
	struct iwl_mvm_vif *other_mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_vif *curr_mvmvif =
		iwl_mvm_vif_from_mac80211(data->current_vif);

	/* exclude the given vif */
	if (vif == data->current_vif)
		return;

	if (vif->type == NL80211_IFTYPE_AP && vif->p2p &&
	    other_mvmvif->phy_ctxt && curr_mvmvif->phy_ctxt &&
	    other_mvmvif->phy_ctxt->id != curr_mvmvif->phy_ctxt->id)
		data->is_dcm_with_p2p_go = true;
}
{
	u16 flags = 0;

	if (params->n_ssids == 0)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_FORCE_PASSIVE;

	if (iwl_mvm_is_scan_fragmented(params->type))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_FRAGMENTED_LMAC1;

	if (iwl_mvm_is_scan_fragmented(params->hb_type))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_FRAGMENTED_LMAC2;

	if (params->pass_all)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_PASS_ALL;
	else
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_MATCH;

	if (!iwl_mvm_is_regular_scan(params))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_PERIODIC;

	if (params->iter_notif ||
	    mvm->sched_scan_pass_all == SCHED_SCAN_PASS_ALL_ENABLED)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_NTFY_ITER_COMPLETE;

	if (IWL_MVM_ADWELL_ENABLE)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_ADAPTIVE_DWELL;

	if (type == IWL_MVM_SCAN_SCHED || type == IWL_MVM_SCAN_NETDETECT)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_PREEMPTIVE;

	if ((type == IWL_MVM_SCAN_SCHED || type == IWL_MVM_SCAN_NETDETECT) &&
	    params->flags & NL80211_SCAN_FLAG_COLOCATED_6GHZ)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_TRIGGER_UHB_SCAN;

	if (params->enable_6ghz_passive)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_6GHZ_PASSIVE_SCAN;

	return flags;
}

static u16 iwl_mvm_scan_umac_flags(struct iwl_mvm *mvm,
				   struct iwl_mvm_scan_params *params,
				   struct ieee80211_vif *vif)
{
	u16 flags = 0;

	if (params->n_ssids == 0)
		flags = IWL_UMAC_SCAN_GEN_FLAGS_PASSIVE;

	if (params->n_ssids == 1 && params->ssids[0].ssid_len != 0)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_PRE_CONNECT;

	if (iwl_mvm_is_scan_fragmented(params->type))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_FRAGMENTED;

	if (iwl_mvm_is_cdb_supported(mvm) &&
	    iwl_mvm_is_scan_fragmented(params->hb_type))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_LMAC2_FRAGMENTED;

	if (iwl_mvm_rrm_scan_needed(mvm) &&
	    fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_WFA_TPC_REP_IE_SUPPORT))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_RRM_ENABLED;

	if (params->pass_all)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_PASS_ALL;
	else
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_MATCH;

	if (!iwl_mvm_is_regular_scan(params))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_PERIODIC;

	if (params->iter_notif)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_ITER_COMPLETE;

#ifdef CONFIG_IWLWIFI_DEBUGFS
	if (mvm->scan_iter_notif_enabled)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_ITER_COMPLETE;
#endif

	if (mvm->sched_scan_pass_all == SCHED_SCAN_PASS_ALL_ENABLED)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_ITER_COMPLETE;

	if (iwl_mvm_is_adaptive_dwell_supported(mvm) && IWL_MVM_ADWELL_ENABLE)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_ADAPTIVE_DWELL;

	/*
	 * Extended dwell is relevant only for low band to start with, as it is
	 * being used for social channles only (1, 6, 11), so we can check
	 * only scan type on low band also for CDB.
	 */
	if (iwl_mvm_is_regular_scan(params) &&
	    vif->type != NL80211_IFTYPE_P2P_DEVICE &&
	    !iwl_mvm_is_scan_fragmented(params->type) &&
	    !iwl_mvm_is_adaptive_dwell_supported(mvm) &&
	    !iwl_mvm_is_oce_supported(mvm))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_EXTENDED_DWELL;

	if (iwl_mvm_is_oce_supported(mvm)) {
		if ((params->flags &
		     NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE))
			flags |= IWL_UMAC_SCAN_GEN_FLAGS_PROB_REQ_HIGH_TX_RATE;
		/* Since IWL_UMAC_SCAN_GEN_FLAGS_EXTENDED_DWELL and
		 * NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION shares
		 * the same bit, we need to make sure that we use this bit here
		 * only when IWL_UMAC_SCAN_GEN_FLAGS_EXTENDED_DWELL cannot be
		 * used. */
		if ((params->flags &
		     NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION) &&
		     !WARN_ON_ONCE(!iwl_mvm_is_adaptive_dwell_supported(mvm)))
			flags |= IWL_UMAC_SCAN_GEN_FLAGS_PROB_REQ_DEFER_SUPP;
		if ((params->flags & NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME))
			flags |= IWL_UMAC_SCAN_GEN_FLAGS_MAX_CHNL_TIME;
	}