	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status);
	set_bit(IWL_MVM_STATUS_D3_RECONFIG, &mvm->status);
	/*
	 * When switching images we return 1, which causes mac80211
	 * to do a reconfig with IEEE80211_RECONFIG_TYPE_RESTART.
	 * This type of reconfig calls iwl_mvm_restart_complete(),
	 * where we unref the IWL_MVM_REF_UCODE_DOWN, so we need
	 * to take the reference here.
	 */
	iwl_mvm_ref(mvm, IWL_MVM_REF_UCODE_DOWN);

	return 1;
}

static int iwl_mvm_resume_d3(struct iwl_mvm *mvm)
{
	iwl_trans_resume(mvm->trans);

	return __iwl_mvm_resume(mvm, false);
}

static int iwl_mvm_resume_d0i3(struct iwl_mvm *mvm)
{
	bool exit_now;
	enum iwl_d3_status d3_status;
	struct iwl_trans *trans = mvm->trans;

	iwl_trans_d3_resume(trans, &d3_status, false, false);

	/*
	 * make sure to clear D0I3_DEFER_WAKEUP before
	 * calling iwl_trans_resume(), which might wait
	 * for d0i3 exit completion.
	 */
	mutex_lock(&mvm->d0i3_suspend_mutex);
	__clear_bit(D0I3_DEFER_WAKEUP, &mvm->d0i3_suspend_flags);
	exit_now = __test_and_clear_bit(D0I3_PENDING_WAKEUP,
					&mvm->d0i3_suspend_flags);
	mutex_unlock(&mvm->d0i3_suspend_mutex);
	if (exit_now) {
		IWL_DEBUG_RPM(mvm, "Run deferred d0i3 exit\n");
		_iwl_mvm_exit_d0i3(mvm);
	}