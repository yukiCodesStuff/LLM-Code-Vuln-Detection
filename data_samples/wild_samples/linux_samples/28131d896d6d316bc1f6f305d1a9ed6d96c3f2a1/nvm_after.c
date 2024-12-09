	if (iwl_mvm_is_vif_assoc(mvm) && notif->source_id == MCC_SOURCE_WIFI) {
		IWL_DEBUG_LAR(mvm, "Ignore mcc update while associated\n");
		return;
	}

	if (WARN_ON_ONCE(!iwl_mvm_is_lar_supported(mvm)))
		return;

	mcc[0] = le16_to_cpu(notif->mcc) >> 8;
	mcc[1] = le16_to_cpu(notif->mcc) & 0xff;
	mcc[2] = '\0';
	src = notif->source_id;

	IWL_DEBUG_LAR(mvm,
		      "RX: received chub update mcc cmd (mcc '%s' src %d)\n",
		      mcc, src);
	regd = iwl_mvm_get_regdomain(mvm->hw->wiphy, mcc, src, NULL);
	if (IS_ERR_OR_NULL(regd))
		return;

	wgds_tbl_idx = iwl_mvm_get_sar_geo_profile(mvm);
	if (wgds_tbl_idx < 1)
		IWL_DEBUG_INFO(mvm,
			       "SAR WGDS is disabled or error received (%d)\n",
			       wgds_tbl_idx);
	else
		IWL_DEBUG_INFO(mvm, "SAR WGDS: geo profile %d is configured\n",
			       wgds_tbl_idx);

	regulatory_set_wiphy_regd(mvm->hw->wiphy, regd);
	kfree(regd);
}