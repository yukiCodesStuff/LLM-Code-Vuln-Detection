		.bandwidths = BIT(NL80211_CHAN_WIDTH_20_NOHT) |
			      BIT(NL80211_CHAN_WIDTH_20) |
			      BIT(NL80211_CHAN_WIDTH_40) |
			      BIT(NL80211_CHAN_WIDTH_80) |
			      BIT(NL80211_CHAN_WIDTH_160),
		.preambles = BIT(NL80211_PREAMBLE_LEGACY) |
			     BIT(NL80211_PREAMBLE_HT) |
			     BIT(NL80211_PREAMBLE_VHT) |
			     BIT(NL80211_PREAMBLE_HE),
	}

	sband = mvm->hw->wiphy->bands[chanctx_conf->def.chan->band];
	own_he_cap = ieee80211_get_he_iftype_cap(sband,
						 ieee80211_vif_type_p2p(vif));

	sta = rcu_dereference(mvm->fw_id_to_mac_id[sta_ctxt_cmd.sta_id]);
	if (IS_ERR_OR_NULL(sta)) {
		rcu_read_unlock();
		IWL_ERR(mvm, "Failed to config FW to work HE!\n");
}

static void iwl_mvm_protect_assoc(struct iwl_mvm *mvm,
				  struct ieee80211_vif *vif,
				  u32 duration_override)
{
	u32 duration = IWL_MVM_TE_SESSION_PROTECTION_MAX_TIME_MS;
	u32 min_duration = IWL_MVM_TE_SESSION_PROTECTION_MIN_TIME_MS;

	if (duration_override > duration)
		duration = duration_override;

	/* Try really hard to protect the session and hear a beacon
	 * The new session protection command allows us to protect the
	 * session for a much longer time since the firmware will internally
	 * create two events: a 300TU one with a very high priority that
	 * won't be fragmented which should be enough for 99% of the cases,
	 * and another one (which we configure here to be 900TU long) which
	 * will have a slightly lower priority, but more importantly, can be
	 * fragmented so that it'll allow other activities to run.
	 */
	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_SESSION_PROT_CMD))
		iwl_mvm_schedule_session_protection(mvm, vif, 900,
						    min_duration, false);
	else
		iwl_mvm_protect_session(mvm, vif, duration,
					min_duration, 500, false);
}

static void iwl_mvm_bss_info_changed_station(struct iwl_mvm *mvm,
					     struct ieee80211_vif *vif,
					     struct ieee80211_bss_conf *bss_conf,
					     u32 changes)
				u32 dur = (11 * vif->bss_conf.beacon_int) / 10;
				iwl_mvm_protect_session(mvm, vif, dur, dur,
							5 * dur, false);
			} else if (!test_bit(IWL_MVM_STATUS_IN_HW_RESTART,
					     &mvm->status) &&
				   !vif->bss_conf.dtim_period) {
				/*
				 * If we're not restarting and still haven't
				 * heard a beacon (dtim period unknown) then
				 * make sure we still have enough minimum time
				 * remaining in the time event, since the auth
				 * might actually have taken quite a while
				 * (especially for SAE) and so the remaining
				 * time could be small without us having heard
				 * a beacon yet.
				 */
				iwl_mvm_protect_assoc(mvm, vif, 0);
			}

			iwl_mvm_sf_update(mvm, vif, false);
			iwl_mvm_power_vif_assoc(mvm, vif);
		if (iwl_mvm_phy_ctx_count(mvm) > 1)
			iwl_mvm_teardown_tdls_peers(mvm);

		if (sta->tdls) {
			iwl_mvm_tdls_check_trigger(mvm, vif, sta->addr,
						   NL80211_TDLS_ENABLE_LINK);
		} else {
			/* enable beacon filtering */
			WARN_ON(iwl_mvm_enable_beacon_filter(mvm, vif, 0));

			mvmvif->authorized = 1;

			/*
			 * Now that the station is authorized, i.e., keys were already
			 * installed, need to indicate to the FW that
			 * multicast data frames can be forwarded to the driver
			 */
			iwl_mvm_mac_ctxt_changed(mvm, vif, false, NULL);
		}

		iwl_mvm_rs_rate_init(mvm, sta, mvmvif->phy_ctxt->channel->band,
				     true);
	} else if (old_state == IEEE80211_STA_AUTHORIZED &&
		   new_state == IEEE80211_STA_ASSOC) {
		if (!sta->tdls) {
			/* Multicast data frames are no longer allowed */
			iwl_mvm_mac_ctxt_changed(mvm, vif, false, NULL);

			/*
			 * Set this after the above iwl_mvm_mac_ctxt_changed()
			 * to avoid sending high prio again for a little time.
			 */
			mvmvif->authorized = 0;

			/* disable beacon filtering */
			ret = iwl_mvm_disable_beacon_filter(mvm, vif, 0);
			WARN_ON(ret &&
				!test_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED,
					  &mvm->status));
		}
		ret = 0;
	} else if (old_state == IEEE80211_STA_ASSOC &&
		   new_state == IEEE80211_STA_AUTH) {
		if (vif->type == NL80211_IFTYPE_AP) {
			mvmvif->ap_assoc_sta_count--;
			iwl_mvm_mac_ctxt_changed(mvm, vif, false, NULL);
		} else if (vif->type == NL80211_IFTYPE_STATION && !sta->tdls) {
			/* remove session protection if still running */
			iwl_mvm_stop_session_protection(mvm, vif);
		}
		ret = 0;
	} else if (old_state == IEEE80211_STA_AUTH &&
		   new_state == IEEE80211_STA_NONE) {
				       struct ieee80211_prep_tx_info *info)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	mutex_lock(&mvm->mutex);
	iwl_mvm_protect_assoc(mvm, vif, info->duration);
	mutex_unlock(&mvm->mutex);
}

static void iwl_mvm_mac_mgd_complete_tx(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
					struct ieee80211_prep_tx_info *info)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	/* for successful cases (auth/assoc), don't cancel session protection */
	if (info->success)
		return;

	mutex_lock(&mvm->mutex);
	iwl_mvm_stop_session_protection(mvm, vif);
	mutex_unlock(&mvm->mutex);
}

static int iwl_mvm_mac_sched_scan_start(struct ieee80211_hw *hw,
	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_CS_MODIFY))
		return;

	IWL_DEBUG_MAC80211(mvm, "Modify CSA on mac %d count = %d (old %d) mode = %d\n",
			   mvmvif->id, chsw->count, mvmvif->csa_count, chsw->block_tx);

	if (chsw->count >= mvmvif->csa_count && chsw->block_tx) {
		if (mvmvif->csa_misbehave) {
			/* Second time, give up on this AP*/
			iwl_mvm_abort_channel_switch(hw, vif);
	if (mvmvif->csa_failed)
		goto out_unlock;

	WARN_ON(iwl_mvm_send_cmd_pdu(mvm,
				     WIDE_ID(MAC_CONF_GROUP,
					     CHANNEL_SWITCH_TIME_EVENT_CMD),
				     0, sizeof(cmd), &cmd));

static void iwl_mvm_set_sta_rate(u32 rate_n_flags, struct rate_info *rinfo)
{
	u32 format = rate_n_flags & RATE_MCS_MOD_TYPE_MSK;

	switch (rate_n_flags & RATE_MCS_CHAN_WIDTH_MSK) {
	case RATE_MCS_CHAN_WIDTH_20:
		rinfo->bw = RATE_INFO_BW_20;
		break;
		break;
	}

	if (format == RATE_MCS_CCK_MSK ||
	    format == RATE_MCS_LEGACY_OFDM_MSK) {
		int rate = u32_get_bits(rate_n_flags, RATE_LEGACY_RATE_MSK);

		/* add the offset needed to get to the legacy ofdm indices */
		if (format == RATE_MCS_LEGACY_OFDM_MSK)
			rate += IWL_FIRST_OFDM_RATE;

		switch (rate) {
		case IWL_RATE_1M_INDEX:
			rinfo->legacy = 10;
			break;
		case IWL_RATE_2M_INDEX:
			rinfo->legacy = 20;
			break;
		case IWL_RATE_5M_INDEX:
			rinfo->legacy = 55;
			break;
		case IWL_RATE_11M_INDEX:
			rinfo->legacy = 110;
			break;
		case IWL_RATE_6M_INDEX:
			rinfo->legacy = 60;
			break;
		case IWL_RATE_9M_INDEX:
			rinfo->legacy = 90;
			break;
		case IWL_RATE_12M_INDEX:
			rinfo->legacy = 120;
			break;
		case IWL_RATE_18M_INDEX:
			rinfo->legacy = 180;
			break;
		case IWL_RATE_24M_INDEX:
			rinfo->legacy = 240;
			break;
		case IWL_RATE_36M_INDEX:
			rinfo->legacy = 360;
			break;
		case IWL_RATE_48M_INDEX:
			rinfo->legacy = 480;
			break;
		case IWL_RATE_54M_INDEX:
			rinfo->legacy = 540;
		}
		return;
	}

	rinfo->nss = u32_get_bits(rate_n_flags,
				  RATE_MCS_NSS_MSK) + 1;
	rinfo->mcs = format == RATE_MCS_HT_MSK ?
		RATE_HT_MCS_INDEX(rate_n_flags) :
		u32_get_bits(rate_n_flags, RATE_MCS_CODE_MSK);

	if (format == RATE_MCS_HE_MSK) {
		u32 gi_ltf = u32_get_bits(rate_n_flags,
					  RATE_MCS_HE_GI_LTF_MSK);

		rinfo->flags |= RATE_INFO_FLAGS_HE_MCS;

		if (rate_n_flags & RATE_MCS_HE_106T_MSK) {
			rinfo->bw = RATE_INFO_BW_HE_RU;
			rinfo->he_ru_alloc = NL80211_RATE_INFO_HE_RU_ALLOC_106;
				rinfo->he_gi = NL80211_RATE_INFO_HE_GI_0_8;
			else if (gi_ltf == 2)
				rinfo->he_gi = NL80211_RATE_INFO_HE_GI_1_6;
			else if (gi_ltf == 3)
				rinfo->he_gi = NL80211_RATE_INFO_HE_GI_3_2;
			else
				rinfo->he_gi = NL80211_RATE_INFO_HE_GI_0_8;
			break;
		case RATE_MCS_HE_TYPE_MU:
			if (gi_ltf == 0 || gi_ltf == 1)
				rinfo->he_gi = NL80211_RATE_INFO_HE_GI_0_8;

		if (rate_n_flags & RATE_HE_DUAL_CARRIER_MODE_MSK)
			rinfo->he_dcm = 1;
		return;
	}

	if (rate_n_flags & RATE_MCS_SGI_MSK)
		rinfo->flags |= RATE_INFO_FLAGS_SHORT_GI;

	if (format == RATE_MCS_HT_MSK) {
		rinfo->flags |= RATE_INFO_FLAGS_MCS;

	} else if (format == RATE_MCS_VHT_MSK) {
		rinfo->flags |= RATE_INFO_FLAGS_VHT_MCS;
	}

}

static void iwl_mvm_mac_sta_statistics(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif,
	.sta_rc_update = iwl_mvm_sta_rc_update,
	.conf_tx = iwl_mvm_mac_conf_tx,
	.mgd_prepare_tx = iwl_mvm_mac_mgd_prepare_tx,
	.mgd_complete_tx = iwl_mvm_mac_mgd_complete_tx,
	.mgd_protect_tdls_discover = iwl_mvm_mac_mgd_protect_tdls_discover,
	.flush = iwl_mvm_mac_flush,
	.sched_scan_start = iwl_mvm_mac_sched_scan_start,
	.sched_scan_stop = iwl_mvm_mac_sched_scan_stop,