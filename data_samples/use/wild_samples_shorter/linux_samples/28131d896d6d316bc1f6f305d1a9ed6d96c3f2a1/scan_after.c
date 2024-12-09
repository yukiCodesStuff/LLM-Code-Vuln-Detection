	tx_ant = BIT(mvm->scan_last_antenna_idx) << RATE_MCS_ANT_POS;

	if (band == NL80211_BAND_2GHZ && !no_cck)
		return cpu_to_le32(IWL_RATE_1M_PLCP | RATE_MCS_CCK_MSK_V1 |
				   tx_ant);
	else
		return cpu_to_le32(IWL_RATE_6M_PLCP | tx_ant);
}
{
	u16 flags = 0;

	/*
	 * If no direct SSIDs are provided perform a passive scan. Otherwise,
	 * if there is a single SSID which is not the broadcast SSID, assume
	 * that the scan is intended for roaming purposes and thus enable Rx on
	 * all chains to improve chances of hearing the beacons/probe responses.
	 */
	if (params->n_ssids == 0)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_FORCE_PASSIVE;
	else if (params->n_ssids == 1 && params->ssids[0].ssid_len)
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_USE_ALL_RX_CHAINS;

	if (iwl_mvm_is_scan_fragmented(params->type))
		flags |= IWL_UMAC_SCAN_GEN_FLAGS_V2_FRAGMENTED_LMAC1;
