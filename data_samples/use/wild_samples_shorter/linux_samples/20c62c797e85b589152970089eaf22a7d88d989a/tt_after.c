
	lockdep_assert_held(&mvm->mutex);

	status = 0;
	ret = iwl_mvm_send_cmd_pdu_status(mvm, WIDE_ID(PHY_OPS_GROUP,
						       CTDP_CONFIG_CMD),
					  sizeof(cmd), &cmd, &status);
