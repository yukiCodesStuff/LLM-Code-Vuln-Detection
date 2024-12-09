
#define DRV_DESCRIPTION	"The new Intel(R) wireless AGN driver for Linux"
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_LICENSE("GPL");

static const struct iwl_op_mode_ops iwl_mvm_ops;
static const struct iwl_op_mode_ops iwl_mvm_ops_mq;
		       iwl_mvm_probe_resp_data_notif,
		       RX_HANDLER_ASYNC_LOCKED,
		       struct iwl_probe_resp_data_notif),
	RX_HANDLER_GRP(MAC_CONF_GROUP, CHANNEL_SWITCH_NOA_NOTIF,
		       iwl_mvm_channel_switch_noa_notif,
		       RX_HANDLER_SYNC, struct iwl_channel_switch_noa_notif),
	RX_HANDLER_GRP(DATA_PATH_GROUP, MONITOR_NOTIF,
		       iwl_mvm_rx_monitor_notif, RX_HANDLER_ASYNC_LOCKED,
		       struct iwl_datapath_monitor_notif),

	HCMD_NAME(CHANNEL_SWITCH_TIME_EVENT_CMD),
	HCMD_NAME(SESSION_PROTECTION_CMD),
	HCMD_NAME(SESSION_PROTECTION_NOTIF),
	HCMD_NAME(CHANNEL_SWITCH_NOA_NOTIF),
};

/* Please keep this array *SORTED* by hex value.
 * Access is done through binary search
	HCMD_NAME(CMD_DTS_MEASUREMENT_TRIGGER_WIDE),
	HCMD_NAME(CTDP_CONFIG_CMD),
	HCMD_NAME(TEMP_REPORTING_THRESHOLDS_CMD),
	HCMD_NAME(GEO_TX_POWER_LIMIT),
	HCMD_NAME(CT_KILL_NOTIFICATION),
	HCMD_NAME(DTS_MEASUREMENT_NOTIF_WIDE),
};

	return 0;
}

static struct iwl_op_mode *
iwl_op_mode_mvm_start(struct iwl_trans *trans, const struct iwl_cfg *cfg,
		      const struct iwl_fw *fw, struct dentry *dbgfs_dir)
{
	mvm->hw = hw;

	iwl_fw_runtime_init(&mvm->fwrt, trans, fw, &iwl_mvm_fwrt_ops, mvm,
			    dbgfs_dir);

	iwl_mvm_get_acpi_tables(mvm);

	mvm->init_status = 0;
	mvm->cmd_ver.range_resp =
		iwl_fw_lookup_notif_ver(mvm->fw, LOCATION_GROUP,
					TOF_RANGE_RESPONSE_NOTIF, 5);
	/* we only support up to version 8 */
	if (WARN_ON_ONCE(mvm->cmd_ver.range_resp > 8))
		goto out_free;

	/*
	 * Populate the state variables that the transport layer needs