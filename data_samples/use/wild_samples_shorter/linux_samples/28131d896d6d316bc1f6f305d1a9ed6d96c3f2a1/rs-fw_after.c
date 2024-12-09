		fw_chains |= IWL_TLC_MNG_CHAIN_A_MSK;
	if (chains & ANT_B)
		fw_chains |= IWL_TLC_MNG_CHAIN_B_MSK;

	return fw_chains;
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