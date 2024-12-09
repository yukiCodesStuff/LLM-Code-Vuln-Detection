	int rate_idx = -1;
	u8 rate_plcp;
	u32 rate_flags = 0;
	bool is_cck;
	struct iwl_mvm_sta *mvmsta = iwl_mvm_sta_from_mac80211(sta);

	/* info->control is only relevant for non HW rate control */
	if (!ieee80211_hw_check(mvm->hw, HAS_RATE_CONTROL)) {
	BUILD_BUG_ON(IWL_FIRST_CCK_RATE != 0);

	/* Get PLCP rate for tx_cmd->rate_n_flags */
	rate_plcp = iwl_mvm_mac80211_idx_to_hwrate(mvm->fw, rate_idx);
	is_cck = (rate_idx >= IWL_FIRST_CCK_RATE) && (rate_idx <= IWL_LAST_CCK_RATE);

	/* Set CCK or OFDM flag */
	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP, TX_CMD, 0) > 8) {
		if (!is_cck)
			rate_flags |= RATE_MCS_LEGACY_OFDM_MSK;
		else
			rate_flags |= RATE_MCS_CCK_MSK;
	} else if (is_cck) {
		rate_flags |= RATE_MCS_CCK_MSK_V1;
	}

	return (u32)rate_plcp | rate_flags;
}

}
#endif /* CONFIG_IWLWIFI_DEBUG */

static int iwl_mvm_get_hwrate_chan_width(u32 chan_width)
{
	switch (chan_width) {
	case RATE_MCS_CHAN_WIDTH_20:
		return 0;
	case RATE_MCS_CHAN_WIDTH_40:
		return IEEE80211_TX_RC_40_MHZ_WIDTH;
	case RATE_MCS_CHAN_WIDTH_80:
		return IEEE80211_TX_RC_80_MHZ_WIDTH;
	case RATE_MCS_CHAN_WIDTH_160:
		return IEEE80211_TX_RC_160_MHZ_WIDTH;
	default:
		return 0;
	}
}

void iwl_mvm_hwrate_to_tx_rate(u32 rate_n_flags,
			       enum nl80211_band band,
			       struct ieee80211_tx_rate *r)
{
	u32 format = rate_n_flags & RATE_MCS_MOD_TYPE_MSK;
	u32 rate = format == RATE_MCS_HT_MSK ?
		RATE_HT_MCS_INDEX(rate_n_flags) :
		rate_n_flags & RATE_MCS_CODE_MSK;

	r->flags |=
		iwl_mvm_get_hwrate_chan_width(rate_n_flags &
					      RATE_MCS_CHAN_WIDTH_MSK);

	if (rate_n_flags & RATE_MCS_SGI_MSK)
		r->flags |= IEEE80211_TX_RC_SHORT_GI;
	if (format ==  RATE_MCS_HT_MSK) {
		r->flags |= IEEE80211_TX_RC_MCS;
		r->idx = rate;
	} else if (format ==  RATE_MCS_VHT_MSK) {
		ieee80211_rate_set_vht(r, rate,
				       ((rate_n_flags & RATE_MCS_NSS_MSK) >>
					RATE_MCS_NSS_POS) + 1);
		r->flags |= IEEE80211_TX_RC_VHT_MCS;
	} else if (format == RATE_MCS_HE_MSK) {
		/* mac80211 cannot do this without ieee80211_tx_status_ext()
		 * but it only matters for radiotap */
		r->idx = 0;
	} else {
		r->idx = iwl_mvm_legacy_hw_idx_to_mac80211_idx(rate_n_flags,
							       band);
	}
}

void iwl_mvm_hwrate_to_tx_rate_v1(u32 rate_n_flags,
				  enum nl80211_band band,
				  struct ieee80211_tx_rate *r)
{
	if (rate_n_flags & RATE_HT_MCS_GF_MSK)
		r->flags |= IEEE80211_TX_RC_GREEN_FIELD;

	r->flags |=
		iwl_mvm_get_hwrate_chan_width(rate_n_flags &
					      RATE_MCS_CHAN_WIDTH_MSK_V1);

	if (rate_n_flags & RATE_MCS_SGI_MSK_V1)
		r->flags |= IEEE80211_TX_RC_SHORT_GI;
	if (rate_n_flags & RATE_MCS_HT_MSK_V1) {
		r->flags |= IEEE80211_TX_RC_MCS;
		r->idx = rate_n_flags & RATE_HT_MCS_INDEX_MSK_V1;
	} else if (rate_n_flags & RATE_MCS_VHT_MSK_V1) {
		ieee80211_rate_set_vht(
			r, rate_n_flags & RATE_VHT_MCS_RATE_CODE_MSK,
			((rate_n_flags & RATE_VHT_MCS_NSS_MSK) >>
						RATE_VHT_MCS_NSS_POS) + 1);
/*
 * translate ucode response to mac80211 tx status control values
 */
static void iwl_mvm_hwrate_to_tx_status(const struct iwl_fw *fw,
					u32 rate_n_flags,
					struct ieee80211_tx_info *info)
{
	struct ieee80211_tx_rate *r = &info->status.rates[0];

	if (iwl_fw_lookup_notif_ver(fw, LONG_GROUP,
				    TX_CMD, 0) > 6)
		rate_n_flags = iwl_new_rate_from_v1(rate_n_flags);

	info->status.antenna =
		((rate_n_flags & RATE_MCS_ANT_AB_MSK) >> RATE_MCS_ANT_POS);
	iwl_mvm_hwrate_to_tx_rate(rate_n_flags,
				  info->band, r);
}

static void iwl_mvm_tx_status_check_trigger(struct iwl_mvm *mvm,
					    u32 status, __le16 frame_control)
			/* the FW should have stopped the queue and not
			 * return this status
			 */
			IWL_ERR_LIMIT(mvm,
				      "FW reported TX filtered, status=0x%x, FC=0x%x\n",
				      status, le16_to_cpu(hdr->frame_control));
			info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
			break;
		default:
			break;
		iwl_mvm_tx_status_check_trigger(mvm, status, hdr->frame_control);

		info->status.rates[0].count = tx_resp->failure_frame + 1;

		iwl_mvm_hwrate_to_tx_status(mvm->fw,
					    le32_to_cpu(tx_resp->initial_rate),
					    info);

		/* Don't assign the converted initial_rate, because driver
		 * TLC uses this and doesn't support the new FW rate
		 */
		info->status.status_driver_data[1] =
			(void *)(uintptr_t)le32_to_cpu(tx_resp->initial_rate);

		/* Single frame failure in an AMPDU queue => send BAR */
			info->flags |= IEEE80211_TX_STAT_AMPDU;
			memcpy(&info->status, &tx_info->status,
			       sizeof(tx_info->status));
			iwl_mvm_hwrate_to_tx_status(mvm->fw, rate, info);
		}
	}

	spin_unlock_bh(&mvmsta->lock);
			goto out;

		tx_info->band = chanctx_conf->def.chan->band;
		iwl_mvm_hwrate_to_tx_status(mvm->fw, rate, tx_info);

		if (!iwl_mvm_has_tlc_offload(mvm)) {
			IWL_DEBUG_TX_REPLY(mvm,
					   "No reclaim. Update rs directly\n");