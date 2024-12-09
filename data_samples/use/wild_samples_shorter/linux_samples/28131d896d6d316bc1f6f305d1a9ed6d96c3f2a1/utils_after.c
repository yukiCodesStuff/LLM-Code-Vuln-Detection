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
	return BIT(ffs(mask) - 1);
}

#define MAX_ANT_NUM 2
/*
 * Toggles between TX antennas to send the probe request on.
 * Receives the bitmask of valid TX antennas and the *index* used
 * for the last TX, and returns the next valid *index* to use.

	lockdep_assert_held(&mvm->mutex);

	if (iwlmvm_mod_params.power_scheme != IWL_POWER_SCHEME_CAM)
		return false;

	if (num_of_ant(iwl_mvm_get_valid_rx_ant(mvm)) == 1)
		return false;

	if (mvm->cfg->rx_with_siso_diversity)