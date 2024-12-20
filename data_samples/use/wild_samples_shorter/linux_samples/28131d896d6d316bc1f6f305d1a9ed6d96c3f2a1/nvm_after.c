		return;

	wgds_tbl_idx = iwl_mvm_get_sar_geo_profile(mvm);
	if (wgds_tbl_idx < 1)
		IWL_DEBUG_INFO(mvm,
			       "SAR WGDS is disabled or error received (%d)\n",
			       wgds_tbl_idx);
	else
		IWL_DEBUG_INFO(mvm, "SAR WGDS: geo profile %d is configured\n",
			       wgds_tbl_idx);