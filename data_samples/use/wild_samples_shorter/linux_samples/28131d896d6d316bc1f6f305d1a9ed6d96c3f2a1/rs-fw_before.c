		fw_chains |= IWL_TLC_MNG_CHAIN_A_MSK;
	if (chains & ANT_B)
		fw_chains |= IWL_TLC_MNG_CHAIN_B_MSK;
	if (chains & ANT_C)
		WARN(false,
		     "tlc offload doesn't support antenna C. chains: 0x%x\n",
		     chains);

	return fw_chains;
}


	if (flags & IWL_TLC_NOTIF_FLAG_RATE) {
		char pretty_rate[100];
		lq_sta->last_rate_n_flags = le32_to_cpu(notif->rate);
		rs_pretty_print_rate(pretty_rate, sizeof(pretty_rate),
				     lq_sta->last_rate_n_flags);
		IWL_DEBUG_RATE(mvm, "new rate: %s\n", pretty_rate);
	}