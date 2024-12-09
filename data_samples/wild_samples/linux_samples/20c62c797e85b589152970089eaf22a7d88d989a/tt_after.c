	struct iwl_mvm_ctdp_cmd cmd = {
		.operation = cpu_to_le32(op),
		.budget = cpu_to_le32(iwl_mvm_cdev_budgets[state]),
		.window_size = 0,
	};
	int ret;
	u32 status;

	lockdep_assert_held(&mvm->mutex);

	status = 0;
	ret = iwl_mvm_send_cmd_pdu_status(mvm, WIDE_ID(PHY_OPS_GROUP,
						       CTDP_CONFIG_CMD),
					  sizeof(cmd), &cmd, &status);

	if (ret) {
		IWL_ERR(mvm, "cTDP command failed (err=%d)\n", ret);
		return ret;
	}