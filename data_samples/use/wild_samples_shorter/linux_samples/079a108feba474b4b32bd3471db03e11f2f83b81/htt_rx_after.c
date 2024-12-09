	fw_desc = &rx->fw_desc;
	rx_desc_len = fw_desc->len;

	if (fw_desc->u.bits.discard) {
		ath10k_dbg(ar, ATH10K_DBG_HTT, "htt discard mpdu\n");
		goto err;
	}

	/* I have not yet seen any case where num_mpdu_ranges > 1.
	 * qcacld does not seem handle that case either, so we introduce the
	 * same limitiation here as well.
	 */