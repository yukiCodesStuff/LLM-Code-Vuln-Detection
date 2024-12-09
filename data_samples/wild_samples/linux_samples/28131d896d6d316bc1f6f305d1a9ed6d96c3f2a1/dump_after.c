	if (((val & ~0xf) == 0xa5a5a5a0) || ((val & ~0xf) == 0x5a5a5a50)) {
		int err;

		IWL_ERR(trans, "HW error, resetting before reading\n");

		/* reset the device */
		iwl_trans_sw_reset(trans);

		err = iwl_finish_nic_init(trans);
		if (err)
			return;
	}

	iwl_trans_read_mem_bytes(trans, base, &table, sizeof(table));

	if (table.valid)
		fwrt->dump.lmac_err_id[lmac_num] = table.error_id;

	if (ERROR_START_OFFSET <= table.valid * ERROR_ELEM_SIZE) {
		IWL_ERR(trans, "Start IWL Error Log Dump:\n");
		IWL_ERR(trans, "Transport status: 0x%08lX, valid: %d\n",
			fwrt->trans->status, table.valid);
	}

	/* Do not change this output - scripts rely on it */

	IWL_ERR(fwrt, "Loaded firmware version: %s\n", fwrt->fw->fw_version);

	IWL_ERR(fwrt, "0x%08X | %-28s\n", table.error_id,
		iwl_fw_lookup_assert_desc(table.error_id));
	IWL_ERR(fwrt, "0x%08X | trm_hw_status0\n", table.trm_hw_status0);
	IWL_ERR(fwrt, "0x%08X | trm_hw_status1\n", table.trm_hw_status1);
	IWL_ERR(fwrt, "0x%08X | branchlink2\n", table.blink2);
	IWL_ERR(fwrt, "0x%08X | interruptlink1\n", table.ilink1);
	IWL_ERR(fwrt, "0x%08X | interruptlink2\n", table.ilink2);
	IWL_ERR(fwrt, "0x%08X | data1\n", table.data1);
	IWL_ERR(fwrt, "0x%08X | data2\n", table.data2);
	IWL_ERR(fwrt, "0x%08X | data3\n", table.data3);
	IWL_ERR(fwrt, "0x%08X | beacon time\n", table.bcon_time);
	IWL_ERR(fwrt, "0x%08X | tsf low\n", table.tsf_low);
	IWL_ERR(fwrt, "0x%08X | tsf hi\n", table.tsf_hi);
	IWL_ERR(fwrt, "0x%08X | time gp1\n", table.gp1);
	IWL_ERR(fwrt, "0x%08X | time gp2\n", table.gp2);
	IWL_ERR(fwrt, "0x%08X | uCode revision type\n", table.fw_rev_type);
	IWL_ERR(fwrt, "0x%08X | uCode version major\n", table.major);
	IWL_ERR(fwrt, "0x%08X | uCode version minor\n", table.minor);
	IWL_ERR(fwrt, "0x%08X | hw version\n", table.hw_ver);
	IWL_ERR(fwrt, "0x%08X | board version\n", table.brd_ver);
	IWL_ERR(fwrt, "0x%08X | hcmd\n", table.hcmd);
	IWL_ERR(fwrt, "0x%08X | isr0\n", table.isr0);
	IWL_ERR(fwrt, "0x%08X | isr1\n", table.isr1);
	IWL_ERR(fwrt, "0x%08X | isr2\n", table.isr2);
	IWL_ERR(fwrt, "0x%08X | isr3\n", table.isr3);
	IWL_ERR(fwrt, "0x%08X | isr4\n", table.isr4);
	IWL_ERR(fwrt, "0x%08X | last cmd Id\n", table.last_cmd_id);
	IWL_ERR(fwrt, "0x%08X | wait_event\n", table.wait_event);
	IWL_ERR(fwrt, "0x%08X | l2p_control\n", table.l2p_control);
	IWL_ERR(fwrt, "0x%08X | l2p_duration\n", table.l2p_duration);
	IWL_ERR(fwrt, "0x%08X | l2p_mhvalid\n", table.l2p_mhvalid);
	IWL_ERR(fwrt, "0x%08X | l2p_addr_match\n", table.l2p_addr_match);
	IWL_ERR(fwrt, "0x%08X | lmpm_pmg_sel\n", table.lmpm_pmg_sel);
	IWL_ERR(fwrt, "0x%08X | timestamp\n", table.u_timestamp);
	IWL_ERR(fwrt, "0x%08X | flow_handler\n", table.flow_handler);
}

/*
 * TCM error struct.
 * Note: This structure is read from the device with IO accesses,
 * and the reading already does the endian conversion. As it is
 * read with u32-sized accesses, any members with a different size
 * need to be ordered correctly though!
 */
struct iwl_tcm_error_event_table {
	u32 valid;
	u32 error_id;
	u32 blink2;
	u32 ilink1;
	u32 ilink2;
	u32 data1, data2, data3;
	u32 logpc;
	u32 frame_pointer;
	u32 stack_pointer;
	u32 msgid;
	u32 isr;
	u32 hw_status[5];
	u32 sw_status[1];
	u32 reserved[4];
} __packed; /* TCM_LOG_ERROR_TABLE_API_S_VER_1 */

static void iwl_fwrt_dump_tcm_error_log(struct iwl_fw_runtime *fwrt)
{
	struct iwl_trans *trans = fwrt->trans;
	struct iwl_tcm_error_event_table table = {};
	u32 base = fwrt->trans->dbg.tcm_error_event_table;
	int i;

	if (!base ||
	    !(fwrt->trans->dbg.error_event_table_tlv_status &
	      IWL_ERROR_EVENT_TABLE_TCM))
		return;

	iwl_trans_read_mem_bytes(trans, base, &table, sizeof(table));

	IWL_ERR(fwrt, "TCM status:\n");
	IWL_ERR(fwrt, "0x%08X | error ID\n", table.error_id);
	IWL_ERR(fwrt, "0x%08X | tcm branchlink2\n", table.blink2);
	IWL_ERR(fwrt, "0x%08X | tcm interruptlink1\n", table.ilink1);
	IWL_ERR(fwrt, "0x%08X | tcm interruptlink2\n", table.ilink2);
	IWL_ERR(fwrt, "0x%08X | tcm data1\n", table.data1);
	IWL_ERR(fwrt, "0x%08X | tcm data2\n", table.data2);
	IWL_ERR(fwrt, "0x%08X | tcm data3\n", table.data3);
	IWL_ERR(fwrt, "0x%08X | tcm log PC\n", table.logpc);
	IWL_ERR(fwrt, "0x%08X | tcm frame pointer\n", table.frame_pointer);
	IWL_ERR(fwrt, "0x%08X | tcm stack pointer\n", table.stack_pointer);
	IWL_ERR(fwrt, "0x%08X | tcm msg ID\n", table.msgid);
	IWL_ERR(fwrt, "0x%08X | tcm ISR status\n", table.isr);
	for (i = 0; i < ARRAY_SIZE(table.hw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm HW status[%d]\n",
			table.hw_status[i], i);
	for (i = 0; i < ARRAY_SIZE(table.sw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm SW status[%d]\n",
			table.sw_status[i], i);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
		u32 scratch = iwl_read32(trans, CSR_FUNC_SCRATCH);

		IWL_ERR(fwrt, "Function Scratch status:\n");
		IWL_ERR(fwrt, "0x%08X | Func Scratch\n", scratch);
	}
{
	struct iwl_trans *trans = fwrt->trans;
	struct iwl_tcm_error_event_table table = {};
	u32 base = fwrt->trans->dbg.tcm_error_event_table;
	int i;

	if (!base ||
	    !(fwrt->trans->dbg.error_event_table_tlv_status &
	      IWL_ERROR_EVENT_TABLE_TCM))
		return;

	iwl_trans_read_mem_bytes(trans, base, &table, sizeof(table));

	IWL_ERR(fwrt, "TCM status:\n");
	IWL_ERR(fwrt, "0x%08X | error ID\n", table.error_id);
	IWL_ERR(fwrt, "0x%08X | tcm branchlink2\n", table.blink2);
	IWL_ERR(fwrt, "0x%08X | tcm interruptlink1\n", table.ilink1);
	IWL_ERR(fwrt, "0x%08X | tcm interruptlink2\n", table.ilink2);
	IWL_ERR(fwrt, "0x%08X | tcm data1\n", table.data1);
	IWL_ERR(fwrt, "0x%08X | tcm data2\n", table.data2);
	IWL_ERR(fwrt, "0x%08X | tcm data3\n", table.data3);
	IWL_ERR(fwrt, "0x%08X | tcm log PC\n", table.logpc);
	IWL_ERR(fwrt, "0x%08X | tcm frame pointer\n", table.frame_pointer);
	IWL_ERR(fwrt, "0x%08X | tcm stack pointer\n", table.stack_pointer);
	IWL_ERR(fwrt, "0x%08X | tcm msg ID\n", table.msgid);
	IWL_ERR(fwrt, "0x%08X | tcm ISR status\n", table.isr);
	for (i = 0; i < ARRAY_SIZE(table.hw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm HW status[%d]\n",
			table.hw_status[i], i);
	for (i = 0; i < ARRAY_SIZE(table.sw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm SW status[%d]\n",
			table.sw_status[i], i);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
		u32 scratch = iwl_read32(trans, CSR_FUNC_SCRATCH);

		IWL_ERR(fwrt, "Function Scratch status:\n");
		IWL_ERR(fwrt, "0x%08X | Func Scratch\n", scratch);
	}