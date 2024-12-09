	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);
	set_bit(IWL_MVM_STATUS_D3_RECONFIG, &mvm->status);
	/*
	 * When switching images we return 1, which causes mac80211
	 * to do a reconfig with IEEE80211_RECONFIG_TYPE_RESTART.