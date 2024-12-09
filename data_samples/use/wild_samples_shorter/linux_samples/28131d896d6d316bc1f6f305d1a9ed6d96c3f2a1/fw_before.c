	if (ret) {
		struct iwl_trans *trans = mvm->trans;

		if (trans->trans_cfg->device_family >=
					IWL_DEVICE_FAMILY_22000) {
			IWL_ERR(mvm,
				"SecBoot CPU1 Status: 0x%x, CPU2 Status: 0x%x\n",
				iwl_read_umac_prph(trans, UMAG_SB_CPU_1_STATUS),
				iwl_read_umac_prph(trans,
						   UMAG_SB_CPU_2_STATUS));
			IWL_ERR(mvm, "UMAC PC: 0x%x\n",
				iwl_read_umac_prph(trans,
						   UREG_UMAC_CURRENT_PC));
			IWL_ERR(mvm, "LMAC PC: 0x%x\n",
				IWL_ERR(mvm, "LMAC2 PC: 0x%x\n",
					iwl_read_umac_prph(trans,
						UREG_LMAC2_CURRENT_PC));
		} else if (trans->trans_cfg->device_family >=
			   IWL_DEVICE_FAMILY_8000) {
			IWL_ERR(mvm,
				"SecBoot CPU1 Status: 0x%x, CPU2 Status: 0x%x\n",
				iwl_read_prph(trans, SB_CPU_1_STATUS),
				iwl_read_prph(trans, SB_CPU_2_STATUS));
		}

		if (ret == -ETIMEDOUT)
			iwl_fw_dbg_error_collect(&mvm->fwrt,
	int ret;
	struct iwl_host_cmd cmd;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, PHY_OPS_GROUP,
					   GEO_TX_POWER_LIMIT,
					   IWL_FW_CMD_VER_UNKNOWN);

	/* the ops field is at the same spot for all versions, so set in v1 */
	geo_tx_cmd.v1.ops =
		cpu_to_le32(IWL_PER_CHAIN_OFFSET_GET_CURRENT_TABLE);

	if (cmd_ver == 3)
		len = sizeof(geo_tx_cmd.v3);
	else if (fw_has_api(&mvm->fwrt.fw->ucode_capa,
			    IWL_UCODE_TLV_API_SAR_TABLE_VER))
		len = sizeof(geo_tx_cmd.v2);
		return -EOPNOTSUPP;

	cmd = (struct iwl_host_cmd){
		.id =  WIDE_ID(PHY_OPS_GROUP, GEO_TX_POWER_LIMIT),
		.len = { len, },
		.flags = CMD_WANT_SKB,
		.data = { &geo_tx_cmd },
	};
	resp = (void *)cmd.resp_pkt->data;
	ret = le32_to_cpu(resp->profile_idx);

	if (WARN_ON(ret > ACPI_NUM_GEO_PROFILES))
		ret = -EIO;

	iwl_free_resp(&cmd);
	return ret;
	union iwl_geo_tx_power_profiles_cmd cmd;
	u16 len;
	u32 n_bands;
	int ret;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, PHY_OPS_GROUP,
					   GEO_TX_POWER_LIMIT,
					   IWL_FW_CMD_VER_UNKNOWN);

	BUILD_BUG_ON(offsetof(struct iwl_geo_tx_power_profiles_cmd_v1, ops) !=
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v2, ops) ||
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v2, ops) !=
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v3, ops));
	/* the ops field is at the same spot for all versions, so set in v1 */
	cmd.v1.ops = cpu_to_le32(IWL_PER_CHAIN_OFFSET_SET_TABLES);

	if (cmd_ver == 3) {
		len = sizeof(cmd.v3);
		n_bands = ARRAY_SIZE(cmd.v3.table[0]);
	} else if (fw_has_api(&mvm->fwrt.fw->ucode_capa,
			      IWL_UCODE_TLV_API_SAR_TABLE_VER)) {
		len = sizeof(cmd.v2);
		n_bands = ARRAY_SIZE(cmd.v2.table[0]);
	} else {
		len = sizeof(cmd.v1);
		n_bands = ARRAY_SIZE(cmd.v1.table[0]);
	}

	BUILD_BUG_ON(offsetof(struct iwl_geo_tx_power_profiles_cmd_v1, table) !=
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v2, table) ||
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v2, table) !=
		     offsetof(struct iwl_geo_tx_power_profiles_cmd_v3, table));
	/* the table is at the same position for all versions, so set use v1 */
	ret = iwl_sar_geo_init(&mvm->fwrt, &cmd.v1.table[0][0], n_bands);

	/*
	 * It is a valid scenario to not support SAR, or miss wgds table,
	 * but in that case there is no need to send the command.
	 * Set the revision on versions that contain it.
	 * This must be done after calling iwl_sar_geo_init().
	 */
	if (cmd_ver == 3)
		cmd.v3.table_revision = cpu_to_le32(mvm->fwrt.geo_rev);
	else if (fw_has_api(&mvm->fwrt.fw->ucode_capa,
			    IWL_UCODE_TLV_API_SAR_TABLE_VER))
		cmd.v2.table_revision = cpu_to_le32(mvm->fwrt.geo_rev);

	return iwl_mvm_send_cmd_pdu(mvm,
				    WIDE_ID(PHY_OPS_GROUP, GEO_TX_POWER_LIMIT),
				    0, len, &cmd);
}

static int iwl_mvm_get_ppag_table(struct iwl_mvm *mvm)
static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	u8 value;
	int ret = iwl_acpi_get_dsm_u8((&mvm->fwrt)->dev, 0, DSM_RFI_FUNC_ENABLE,
				      &iwl_rfi_guid, &value);

	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm, "Failed to get DSM RFI, ret=%d\n", ret);
{
	int ret;
	u32 value;
	struct iwl_lari_config_change_cmd_v4 cmd = {};

	cmd.config_bitmap = iwl_acpi_get_lari_config_bitmap(&mvm->fwrt);

	ret = iwl_acpi_get_dsm_u32((&mvm->fwrt)->dev, 0, DSM_FUNC_11AX_ENABLEMENT,
				   &iwl_guid, &value);
	if (!ret)
		cmd.oem_11ax_allow_bitmap = cpu_to_le32(value);
	/* apply more config masks here */

	ret = iwl_acpi_get_dsm_u32((&mvm->fwrt)->dev, 0,
				   DSM_FUNC_ENABLE_UNII4_CHAN,
				   &iwl_guid, &value);
	if (!ret)
		cmd.oem_unii4_allow_bitmap = cpu_to_le32(value);

	if (cmd.config_bitmap ||
	    cmd.oem_11ax_allow_bitmap ||
	    cmd.oem_unii4_allow_bitmap) {
		size_t cmd_size;
		u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw,
						   REGULATORY_AND_NVM_GROUP,
						   LARI_CONFIG_CHANGE, 1);
		if (cmd_ver == 4)
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v4);
		else if (cmd_ver == 3)
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v3);
		else if (cmd_ver == 2)
				le32_to_cpu(cmd.config_bitmap),
				le32_to_cpu(cmd.oem_11ax_allow_bitmap));
		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, oem_unii4_allow_bitmap=0x%x, cmd_ver=%d\n",
				le32_to_cpu(cmd.oem_unii4_allow_bitmap),
				cmd_ver);
		ret = iwl_mvm_send_cmd_pdu(mvm,
					   WIDE_ID(REGULATORY_AND_NVM_GROUP,
						   LARI_CONFIG_CHANGE),
					   0, cmd_size, &cmd);