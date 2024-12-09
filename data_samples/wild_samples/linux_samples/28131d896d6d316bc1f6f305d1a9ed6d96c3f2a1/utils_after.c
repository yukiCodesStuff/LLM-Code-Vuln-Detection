	struct iwl_host_cmd cmd = {
		.id = id,
		.len = { len, },
		.data = { data, },
	};

	return iwl_mvm_send_cmd_status(mvm, &cmd, status);
}

int iwl_mvm_legacy_hw_idx_to_mac80211_idx(u32 rate_n_flags,
					  enum nl80211_band band)
{
	int format = rate_n_flags & RATE_MCS_MOD_TYPE_MSK;
	int rate = rate_n_flags & RATE_LEGACY_RATE_MSK;
	bool is_LB = band == NL80211_BAND_2GHZ;

	if (format == RATE_MCS_LEGACY_OFDM_MSK)
		return is_LB ? rate + IWL_FIRST_OFDM_RATE :
			rate;

	/* CCK is not allowed in HB */
	return is_LB ? rate : -1;
}

int iwl_mvm_legacy_rate_to_mac80211_idx(u32 rate_n_flags,
					enum nl80211_band band)
{
	int rate = rate_n_flags & RATE_LEGACY_RATE_MSK_V1;
	int idx;
	int band_offset = 0;

	/* Legacy rate format, search for match in table */
	if (band != NL80211_BAND_2GHZ)
		band_offset = IWL_FIRST_OFDM_RATE;
	for (idx = band_offset; idx < IWL_RATE_COUNT_LEGACY; idx++)
		if (iwl_fw_rate_idx_to_plcp(idx) == rate)
			return idx - band_offset;

	return -1;
}

u8 iwl_mvm_mac80211_idx_to_hwrate(const struct iwl_fw *fw, int rate_idx)
{
	if (iwl_fw_lookup_cmd_ver(fw, LONG_GROUP,
				  TX_CMD, 0) > 8)
		/* In the new rate legacy rates are indexed:
		 * 0 - 3 for CCK and 0 - 7 for OFDM.
		 */
		return (rate_idx >= IWL_FIRST_OFDM_RATE ?
			rate_idx - IWL_FIRST_OFDM_RATE :
			rate_idx);

	return iwl_fw_rate_idx_to_plcp(rate_idx);
}

u8 iwl_mvm_mac80211_ac_to_ucode_ac(enum ieee80211_ac_numbers ac)
{
	static const u8 mac80211_ac_to_ucode_ac[] = {
		AC_VO,
		AC_VI,
		AC_BE,
		AC_BK
	};

	return mac80211_ac_to_ucode_ac[ac];
}
{
	BUILD_BUG_ON(ANT_A != BIT(0)); /* using ffs is wrong if not */
	if (WARN_ON_ONCE(!mask)) /* ffs will return 0 if mask is zeroed */
		return BIT(0);
	return BIT(ffs(mask) - 1);
}

#define MAX_ANT_NUM 2
/*
 * Toggles between TX antennas to send the probe request on.
 * Receives the bitmask of valid TX antennas and the *index* used
 * for the last TX, and returns the next valid *index* to use.
 * In order to set it in the tx_cmd, must do BIT(idx).
 */
u8 iwl_mvm_next_antenna(struct iwl_mvm *mvm, u8 valid, u8 last_idx)
{
	u8 ind = last_idx;
	int i;

	for (i = 0; i < MAX_ANT_NUM; i++) {
		ind = (ind + 1) % MAX_ANT_NUM;
		if (valid & BIT(ind))
			return ind;
	}
	struct iwl_mvm_diversity_iter_data data = {
		.ctxt = ctxt,
		.result = true,
	};

	lockdep_assert_held(&mvm->mutex);

	if (iwlmvm_mod_params.power_scheme != IWL_POWER_SCHEME_CAM)
		return false;

	if (num_of_ant(iwl_mvm_get_valid_rx_ant(mvm)) == 1)
		return false;

	if (mvm->cfg->rx_with_siso_diversity)
		return false;

	ieee80211_iterate_active_interfaces_atomic(
			mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_diversity_iter, &data);

	return data.result;
}

void iwl_mvm_send_low_latency_cmd(struct iwl_mvm *mvm,
				  bool low_latency, u16 mac_id)
{
	struct iwl_mac_low_latency_cmd cmd = {
		.mac_id = cpu_to_le32(mac_id)
	};

	if (!fw_has_capa(&mvm->fw->ucode_capa,
			 IWL_UCODE_TLV_CAPA_DYNAMIC_QUOTA))
		return;

	if (low_latency) {
		/* currently we don't care about the direction */
		cmd.low_latency_rx = 1;
		cmd.low_latency_tx = 1;
	}

	if (iwl_mvm_send_cmd_pdu(mvm, iwl_cmd_id(LOW_LATENCY_CMD,
						 MAC_CONF_GROUP, 0),
				 0, sizeof(cmd), &cmd))
		IWL_ERR(mvm, "Failed to send low latency command\n");
}