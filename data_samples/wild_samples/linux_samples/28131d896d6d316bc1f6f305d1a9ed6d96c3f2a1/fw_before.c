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
				iwl_read_umac_prph(trans,
						   UREG_LMAC1_CURRENT_PC));
			if (iwl_mvm_is_cdb_supported(mvm))
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
						 FW_DBG_TRIGGER_ALIVE_TIMEOUT);

		iwl_fw_set_current_image(&mvm->fwrt, old_type);
		return ret;
	}
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
				iwl_read_umac_prph(trans,
						   UREG_LMAC1_CURRENT_PC));
			if (iwl_mvm_is_cdb_supported(mvm))
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
{
	union iwl_geo_tx_power_profiles_cmd geo_tx_cmd;
	struct iwl_geo_tx_power_profiles_resp *resp;
	u16 len;
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
	else
		len = sizeof(geo_tx_cmd.v1);

	if (!iwl_sar_geo_support(&mvm->fwrt))
		return -EOPNOTSUPP;

	cmd = (struct iwl_host_cmd){
		.id =  WIDE_ID(PHY_OPS_GROUP, GEO_TX_POWER_LIMIT),
		.len = { len, },
		.flags = CMD_WANT_SKB,
		.data = { &geo_tx_cmd },
	};

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "Failed to get geographic profile info %d\n", ret);
		return ret;
	}

	resp = (void *)cmd.resp_pkt->data;
	ret = le32_to_cpu(resp->profile_idx);

	if (WARN_ON(ret > ACPI_NUM_GEO_PROFILES))
		ret = -EIO;

	iwl_free_resp(&cmd);
	return ret;
}
{
	union iwl_geo_tx_power_profiles_cmd geo_tx_cmd;
	struct iwl_geo_tx_power_profiles_resp *resp;
	u16 len;
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
	else
		len = sizeof(geo_tx_cmd.v1);

	if (!iwl_sar_geo_support(&mvm->fwrt))
		return -EOPNOTSUPP;

	cmd = (struct iwl_host_cmd){
		.id =  WIDE_ID(PHY_OPS_GROUP, GEO_TX_POWER_LIMIT),
		.len = { len, },
		.flags = CMD_WANT_SKB,
		.data = { &geo_tx_cmd },
	};

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "Failed to get geographic profile info %d\n", ret);
		return ret;
	}

	resp = (void *)cmd.resp_pkt->data;
	ret = le32_to_cpu(resp->profile_idx);

	if (WARN_ON(ret > ACPI_NUM_GEO_PROFILES))
		ret = -EIO;

	iwl_free_resp(&cmd);
	return ret;
}
	if (ret) {
		IWL_ERR(mvm, "Failed to get geographic profile info %d\n", ret);
		return ret;
	}

	resp = (void *)cmd.resp_pkt->data;
	ret = le32_to_cpu(resp->profile_idx);

	if (WARN_ON(ret > ACPI_NUM_GEO_PROFILES))
		ret = -EIO;

	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_sar_geo_init(struct iwl_mvm *mvm)
{
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
	 */
	if (ret)
		return 0;

	/*
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
{
	union acpi_object *wifi_pkg, *data, *flags;
	int i, j, ret, tbl_rev, num_sub_bands;
	int idx = 2;
	s8 *gain;

	/*
	 * The 'flags' field is the same in v1 and in v2 so we can just
	 * use v1 to access it.
	 */
	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);

	data = iwl_acpi_get_object(mvm->dev, ACPI_PPAG_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	/* try to read ppag table rev 2 or 1 (both have the same data size) */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V2, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev == 1 || tbl_rev == 2) {
			num_sub_bands = IWL_NUM_SUB_BANDS_V2;
			gain = mvm->fwrt.ppag_table.v2.gain[0];
			mvm->fwrt.ppag_ver = tbl_rev;
			IWL_DEBUG_RADIO(mvm,
					"Reading PPAG table v2 (tbl_rev=%d)\n",
					tbl_rev);
			goto read_table;
		} else {
			ret = -EINVAL;
			goto out_free;
		}
	}

	/* try to read ppag table revision 0 */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V1, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev != 0) {
			ret = -EINVAL;
			goto out_free;
		}
		num_sub_bands = IWL_NUM_SUB_BANDS_V1;
		gain = mvm->fwrt.ppag_table.v1.gain[0];
		mvm->fwrt.ppag_ver = 0;
		IWL_DEBUG_RADIO(mvm, "Reading PPAG table v1 (tbl_rev=0)\n");
		goto read_table;
	}
	ret = PTR_ERR(wifi_pkg);
	goto out_free;

read_table:
	flags = &wifi_pkg->package.elements[1];

	if (flags->type != ACPI_TYPE_INTEGER) {
		ret = -EINVAL;
		goto out_free;
	}

	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(flags->integer.value &
						    IWL_PPAG_MASK);

	if (!mvm->fwrt.ppag_table.v1.flags) {
		ret = 0;
		goto out_free;
	}

	/*
	 * read, verify gain values and save them into the PPAG table.
	 * first sub-band (j=0) corresponds to Low-Band (2.4GHz), and the
	 * following sub-bands to High-Band (5GHz).
	 */
	for (i = 0; i < IWL_NUM_CHAIN_LIMITS; i++) {
		for (j = 0; j < num_sub_bands; j++) {
			union acpi_object *ent;

			ent = &wifi_pkg->package.elements[idx++];
			if (ent->type != ACPI_TYPE_INTEGER) {
				ret = -EINVAL;
				goto out_free;
			}

			gain[i * num_sub_bands + j] = ent->integer.value;

			if ((j == 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_LB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_LB)) ||
			    (j != 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_HB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_HB))) {
				mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);
				ret = -EINVAL;
				goto out_free;
			}
		}
	}

	ret = 0;
out_free:
	kfree(data);
	return ret;
}

int iwl_mvm_ppag_send_cmd(struct iwl_mvm *mvm)
{
	u8 cmd_ver;
	int i, j, ret, num_sub_bands, cmd_size;
	s8 *gain;

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_SET_PPAG)) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG capability not supported by FW, command not sent.\n");
		return 0;
	}
	if (!mvm->fwrt.ppag_table.v1.flags) {
		IWL_DEBUG_RADIO(mvm, "PPAG not enabled, command not sent.\n");
		return 0;
	}

	cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, PHY_OPS_GROUP,
					PER_PLATFORM_ANT_GAIN_CMD,
					IWL_FW_CMD_VER_UNKNOWN);
	if (cmd_ver == 1) {
		num_sub_bands = IWL_NUM_SUB_BANDS_V1;
		gain = mvm->fwrt.ppag_table.v1.gain[0];
		cmd_size = sizeof(mvm->fwrt.ppag_table.v1);
		if (mvm->fwrt.ppag_ver == 1 || mvm->fwrt.ppag_ver == 2) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table rev is %d but FW supports v1, sending truncated table\n",
					mvm->fwrt.ppag_ver);
			mvm->fwrt.ppag_table.v1.flags &=
				cpu_to_le32(IWL_PPAG_ETSI_MASK);
		}
	} else if (cmd_ver == 2 || cmd_ver == 3) {
		num_sub_bands = IWL_NUM_SUB_BANDS_V2;
		gain = mvm->fwrt.ppag_table.v2.gain[0];
		cmd_size = sizeof(mvm->fwrt.ppag_table.v2);
		if (mvm->fwrt.ppag_ver == 0) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table is v1 but FW supports v2, sending padded table\n");
		} else if (cmd_ver == 2 && mvm->fwrt.ppag_ver == 2) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table is v3 but FW supports v2, sending partial bitmap.\n");
			mvm->fwrt.ppag_table.v1.flags &=
				cpu_to_le32(IWL_PPAG_ETSI_MASK);
		}
	} else {
		IWL_DEBUG_RADIO(mvm, "Unsupported PPAG command version\n");
		return 0;
	}

	for (i = 0; i < IWL_NUM_CHAIN_LIMITS; i++) {
		for (j = 0; j < num_sub_bands; j++) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table: chain[%d] band[%d]: gain = %d\n",
					i, j, gain[i * num_sub_bands + j]);
		}
	}
	IWL_DEBUG_RADIO(mvm, "Sending PER_PLATFORM_ANT_GAIN_CMD\n");
	ret = iwl_mvm_send_cmd_pdu(mvm, WIDE_ID(PHY_OPS_GROUP,
						PER_PLATFORM_ANT_GAIN_CMD),
				   0, cmd_size, &mvm->fwrt.ppag_table);
	if (ret < 0)
		IWL_ERR(mvm, "failed to send PER_PLATFORM_ANT_GAIN_CMD (%d)\n",
			ret);

	return ret;
}

static const struct dmi_system_id dmi_ppag_approved_list[] = {
	{ .ident = "HP",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "HP"),
		},
	},
	{ .ident = "SAMSUNG",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "SAMSUNG ELECTRONICS CO., LTD"),
		},
	},
	{ .ident = "MSFT",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
		},
	},
	{ .ident = "ASUS",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTek COMPUTER INC."),
		},
	},
	{}
};

static int iwl_mvm_ppag_init(struct iwl_mvm *mvm)
{
	/* no need to read the table, done in INIT stage */
	if (!dmi_check_system(dmi_ppag_approved_list)) {
		IWL_DEBUG_RADIO(mvm,
				"System vendor '%s' is not in the approved list, disabling PPAG.\n",
				dmi_get_system_info(DMI_SYS_VENDOR));
		mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);
		return 0;
	}

	return iwl_mvm_ppag_send_cmd(mvm);
}

static void iwl_mvm_tas_init(struct iwl_mvm *mvm)
{
	int ret;
	struct iwl_tas_config_cmd cmd = {};
	int list_size;

	BUILD_BUG_ON(ARRAY_SIZE(cmd.block_list_array) <
		     APCI_WTAS_BLACK_LIST_MAX);

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_TAS_CFG)) {
		IWL_DEBUG_RADIO(mvm, "TAS not enabled in FW\n");
		return;
	}

	ret = iwl_acpi_get_tas(&mvm->fwrt, cmd.block_list_array, &list_size);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"TAS table invalid or unavailable. (%d)\n",
				ret);
		return;
	}

	if (list_size < 0)
		return;

	/* list size if TAS enabled can only be non-negative */
	cmd.block_list_size = cpu_to_le32((u32)list_size);

	ret = iwl_mvm_send_cmd_pdu(mvm, WIDE_ID(REGULATORY_AND_NVM_GROUP,
						TAS_CONFIG),
				   0, sizeof(cmd), &cmd);
	if (ret < 0)
		IWL_DEBUG_RADIO(mvm, "failed to send TAS_CONFIG (%d)\n", ret);
}

static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	u8 value;
	int ret = iwl_acpi_get_dsm_u8((&mvm->fwrt)->dev, 0, DSM_RFI_FUNC_ENABLE,
				      &iwl_rfi_guid, &value);

	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm, "Failed to get DSM RFI, ret=%d\n", ret);

	} else if (value >= DSM_VALUE_RFI_MAX) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI got invalid value, ret=%d\n",
				value);

	} else if (value == DSM_VALUE_RFI_ENABLE) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI is evaluated to enable\n");
		return DSM_VALUE_RFI_ENABLE;
	}

	IWL_DEBUG_RADIO(mvm, "DSM RFI is disabled\n");

	/* default behaviour is disabled */
	return DSM_VALUE_RFI_DISABLE;
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
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
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v2);
		else
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v1);

		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, config_bitmap=0x%x, oem_11ax_allow_bitmap=0x%x\n",
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
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"Failed to send LARI_CONFIG_CHANGE (%d)\n",
					ret);
	}
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
	int ret;

	/* read PPAG table */
	ret = iwl_mvm_get_ppag_table(mvm);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG BIOS table invalid or unavailable. (%d)\n",
				ret);
	}

	/* read SAR tables */
	ret = iwl_sar_get_wrds_table(&mvm->fwrt);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"WRDS SAR BIOS table invalid or unavailable. (%d)\n",
				ret);
		/*
		 * If not available, don't fail and don't bother with EWRD and
		 * WGDS */

		if (!iwl_sar_get_wgds_table(&mvm->fwrt)) {
			/*
			 * If basic SAR is not available, we check for WGDS,
			 * which should *not* be available either.  If it is
			 * available, issue an error, because we can't use SAR
			 * Geo without basic SAR.
			 */
			IWL_ERR(mvm, "BIOS contains WGDS but no WRDS\n");
		}

	} else {
		ret = iwl_sar_get_ewrd_table(&mvm->fwrt);
		/* if EWRD is not available, we can still use
		* WRDS, so don't fail */
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"EWRD SAR BIOS table invalid or unavailable. (%d)\n",
					ret);

		/* read geo SAR table */
		if (iwl_sar_geo_support(&mvm->fwrt)) {
			ret = iwl_sar_get_wgds_table(&mvm->fwrt);
			if (ret < 0)
				IWL_DEBUG_RADIO(mvm,
						"Geo SAR BIOS table invalid or unavailable. (%d)\n",
						ret);
				/* we don't fail if the table is not available */
		}
	}
}
#else /* CONFIG_ACPI */

inline int iwl_mvm_sar_select_profile(struct iwl_mvm *mvm,
				      int prof_a, int prof_b)
{
	return 1;
}

inline int iwl_mvm_get_sar_geo_profile(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_sar_geo_init(struct iwl_mvm *mvm)
{
	return 0;
}

int iwl_mvm_ppag_send_cmd(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_ppag_init(struct iwl_mvm *mvm)
{
	return 0;
}

static void iwl_mvm_tas_init(struct iwl_mvm *mvm)
{
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
{
}

static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	return DSM_VALUE_RFI_DISABLE;
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
}
#endif /* CONFIG_ACPI */

void iwl_mvm_send_recovery_cmd(struct iwl_mvm *mvm, u32 flags)
{
	u32 error_log_size = mvm->fw->ucode_capa.error_log_size;
	int ret;
	u32 resp;

	struct iwl_fw_error_recovery_cmd recovery_cmd = {
		.flags = cpu_to_le32(flags),
		.buf_size = 0,
	};
	struct iwl_host_cmd host_cmd = {
		.id = WIDE_ID(SYSTEM_GROUP, FW_ERROR_RECOVERY_CMD),
		.flags = CMD_WANT_SKB,
		.data = {&recovery_cmd, },
		.len = {sizeof(recovery_cmd), },
	};

	/* no error log was defined in TLV */
	if (!error_log_size)
		return;

	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		/* no buf was allocated while HW reset */
		if (!mvm->error_recovery_buf)
			return;

		host_cmd.data[1] = mvm->error_recovery_buf;
		host_cmd.len[1] =  error_log_size;
		host_cmd.dataflags[1] = IWL_HCMD_DFL_NOCOPY;
		recovery_cmd.buf_size = cpu_to_le32(error_log_size);
	}

	ret = iwl_mvm_send_cmd(mvm, &host_cmd);
	kfree(mvm->error_recovery_buf);
	mvm->error_recovery_buf = NULL;

	if (ret) {
		IWL_ERR(mvm, "Failed to send recovery cmd %d\n", ret);
		return;
	}

	/* skb respond is only relevant in ERROR_RECOVERY_UPDATE_DB */
	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		resp = le32_to_cpu(*(__le32 *)host_cmd.resp_pkt->data);
		if (resp)
			IWL_ERR(mvm,
				"Failed to send recovery cmd blob was invalid %d\n",
				resp);
	}
}

static int iwl_mvm_sar_init(struct iwl_mvm *mvm)
{
	return iwl_mvm_sar_select_profile(mvm, 1, 1);
}

static int iwl_mvm_load_rt_fw(struct iwl_mvm *mvm)
{
	int ret;

	if (iwl_mvm_has_unified_ucode(mvm))
		return iwl_run_unified_mvm_ucode(mvm);

	WARN_ON(!mvm->nvm_data);
	ret = iwl_run_init_mvm_ucode(mvm);

	if (ret) {
		IWL_ERR(mvm, "Failed to run INIT ucode: %d\n", ret);

		if (iwlmvm_mod_params.init_dbg)
			return 0;
		return ret;
	}

	iwl_fw_dbg_stop_sync(&mvm->fwrt);
	iwl_trans_stop_device(mvm->trans);
	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = false;
	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_REGULAR);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = true;

	iwl_dbg_tlv_time_point(&mvm->fwrt, IWL_FW_INI_TIME_POINT_AFTER_ALIVE,
			       NULL);

	return iwl_init_paging(&mvm->fwrt, mvm->fwrt.cur_fw_img);
}

int iwl_mvm_up(struct iwl_mvm *mvm)
{
	int ret, i;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	struct ieee80211_supported_band *sband = NULL;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_rt_fw(mvm);
	if (ret) {
		IWL_ERR(mvm, "Failed to start RT ucode: %d\n", ret);
		if (ret != -ERFKILL)
			iwl_fw_dbg_error_collect(&mvm->fwrt,
						 FW_DBG_TRIGGER_DRIVER);
		goto error;
	}

	iwl_get_shared_mem_conf(&mvm->fwrt);

	ret = iwl_mvm_sf_update(mvm, NULL, false);
	if (ret)
		IWL_ERR(mvm, "Failed to initialize Smart Fifo\n");

	if (!iwl_trans_dbg_ini_valid(mvm->trans)) {
		mvm->fwrt.dump.conf = FW_DBG_INVALID;
		/* if we have a destination, assume EARLY START */
		if (mvm->fw->dbg.dest_tlv)
			mvm->fwrt.dump.conf = FW_DBG_START_FROM_ALIVE;
		iwl_fw_start_dbg_conf(&mvm->fwrt, FW_DBG_START_FROM_ALIVE);
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	if (!iwl_mvm_has_unified_ucode(mvm)) {
		/* Send phy db control command and then phy db calibration */
		ret = iwl_send_phy_db_data(mvm->phy_db);
		if (ret)
			goto error;
	}

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_send_bt_init_conf(mvm);
	if (ret)
		goto error;

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_SOC_LATENCY_SUPPORT)) {
		ret = iwl_set_soc_latency(&mvm->fwrt);
		if (ret)
			goto error;
	}

	/* Init RSS configuration */
	ret = iwl_configure_rxq(&mvm->fwrt);
	if (ret)
		goto error;

	if (iwl_mvm_has_new_rx_api(mvm)) {
		ret = iwl_send_rss_cfg_cmd(mvm);
		if (ret) {
			IWL_ERR(mvm, "Failed to configure RSS queues: %d\n",
				ret);
			goto error;
		}
	}

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	mvm->tdls_cs.peer.sta_id = IWL_MVM_INVALID_STA;

	/* reset quota debouncing buffer - 0xff will yield invalid data */
	memset(&mvm->last_quota_cmd, 0xff, sizeof(mvm->last_quota_cmd));

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_DQA_SUPPORT)) {
		ret = iwl_mvm_send_dqa_cmd(mvm);
		if (ret)
			goto error;
	}

	/*
	 * Add auxiliary station for scanning.
	 * Newer versions of this command implies that the fw uses
	 * internal aux station for all aux activities that don't
	 * requires a dedicated data queue.
	 */
	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		 /*
		  * In old version the aux station uses mac id like other
		  * station and not lmac id
		  */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	/* Add all the PHY contexts */
	i = 0;
	while (!sband && i < NUM_NL80211_BANDS)
		sband = mvm->hw->wiphy->bands[i++];

	if (WARN_ON_ONCE(!sband))
		goto error;

	chan = &sband->channels[0];

	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);
	for (i = 0; i < NUM_PHY_CTX; i++) {
		/*
		 * The channel used here isn't relevant as it's
		 * going to be overwritten in the other flows.
		 * For now use the first channel we have.
		 */
		ret = iwl_mvm_phy_ctxt_add(mvm, &mvm->phy_ctxts[i],
					   &chandef, 1, 1);
		if (ret)
			goto error;
	}

	if (iwl_mvm_is_tt_in_fw(mvm)) {
		/* in order to give the responsibility of ct-kill and
		 * TX backoff to FW we need to send empty temperature reporting
		 * cmd during init time
		 */
		iwl_mvm_send_temp_report_ths_cmd(mvm);
	} else {
		/* Initialize tx backoffs to the minimal possible */
		iwl_mvm_tt_tx_backoff(mvm, 0);
	}

#ifdef CONFIG_THERMAL
	/* TODO: read the budget from BIOS / Platform NVM */

	/*
	 * In case there is no budget from BIOS / Platform NVM the default
	 * budget should be 2000mW (cooling state 0).
	 */
	if (iwl_mvm_is_ctdp_supported(mvm)) {
		ret = iwl_mvm_ctdp_command(mvm, CTDP_CMD_OPERATION_START,
					   mvm->cooling_dev.cur_state);
		if (ret)
			goto error;
	}
#endif

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_SET_LTR_GEN2))
		WARN_ON(iwl_mvm_config_ltr(mvm));

	ret = iwl_mvm_power_update_device(mvm);
	if (ret)
		goto error;

	iwl_mvm_lari_cfg(mvm);
	/*
	 * RTNL is not taken during Ct-kill, but we don't need to scan/Tx
	 * anyway, so don't init MCC.
	 */
	if (!test_bit(IWL_MVM_STATUS_HW_CTKILL, &mvm->status)) {
		ret = iwl_mvm_init_mcc(mvm);
		if (ret)
			goto error;
	}

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_UMAC_SCAN)) {
		mvm->scan_type = IWL_SCAN_TYPE_NOT_SET;
		mvm->hb_scan_type = IWL_SCAN_TYPE_NOT_SET;
		ret = iwl_mvm_config_scan(mvm);
		if (ret)
			goto error;
	}

	if (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status))
		iwl_mvm_send_recovery_cmd(mvm, ERROR_RECOVERY_UPDATE_DB);

	if (iwl_acpi_get_eckv(mvm->dev, &mvm->ext_clock_valid))
		IWL_DEBUG_INFO(mvm, "ECKV table doesn't exist in BIOS\n");

	ret = iwl_mvm_ppag_init(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_sar_init(mvm);
	if (ret == 0)
		ret = iwl_mvm_sar_geo_init(mvm);
	else if (ret < 0)
		goto error;

	iwl_mvm_tas_init(mvm);
	iwl_mvm_leds_sync(mvm);

	iwl_mvm_ftm_initiator_smooth_config(mvm);

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_RFIM_SUPPORT)) {
		if (iwl_mvm_eval_dsm_rfi(mvm) == DSM_VALUE_RFI_ENABLE)
			iwl_rfi_send_config_cmd(mvm, NULL);
	}

	IWL_DEBUG_INFO(mvm, "RT uCode started.\n");
	return 0;
 error:
	if (!iwlmvm_mod_params.init_dbg || !ret)
		iwl_mvm_stop_device(mvm);
	return ret;
}

int iwl_mvm_load_d3_fw(struct iwl_mvm *mvm)
{
	int ret, i;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_WOWLAN);
	if (ret) {
		IWL_ERR(mvm, "Failed to start WoWLAN firmware: %d\n", ret);
		goto error;
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	/* Send phy db control command and then phy db calibration*/
	ret = iwl_send_phy_db_data(mvm->phy_db);
	if (ret)
		goto error;

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		/*
		 * Add auxiliary station for scanning.
		 * Newer versions of this command implies that the fw uses
		 * internal aux station for all aux activities that don't
		 * requires a dedicated data queue.
		 * In old version the aux station uses mac id like other
		 * station and not lmac id
		 */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	return 0;
 error:
	iwl_mvm_stop_device(mvm);
	return ret;
}

void iwl_mvm_rx_card_state_notif(struct iwl_mvm *mvm,
				 struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_card_state_notif *card_state_notif = (void *)pkt->data;
	u32 flags = le32_to_cpu(card_state_notif->flags);

	IWL_DEBUG_RF_KILL(mvm, "Card state received: HW:%s SW:%s CT:%s\n",
			  (flags & HW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & SW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & CT_KILL_CARD_DISABLED) ?
			  "Reached" : "Not reached");
}

void iwl_mvm_rx_mfuart_notif(struct iwl_mvm *mvm,
			     struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_mfuart_load_notif *mfuart_notif = (void *)pkt->data;

	IWL_DEBUG_INFO(mvm,
		       "MFUART: installed ver: 0x%08x, external ver: 0x%08x, status: 0x%08x, duration: 0x%08x\n",
		       le32_to_cpu(mfuart_notif->installed_ver),
		       le32_to_cpu(mfuart_notif->external_ver),
		       le32_to_cpu(mfuart_notif->status),
		       le32_to_cpu(mfuart_notif->duration));

	if (iwl_rx_packet_payload_len(pkt) == sizeof(*mfuart_notif))
		IWL_DEBUG_INFO(mvm,
			       "MFUART: image size: 0x%08x\n",
			       le32_to_cpu(mfuart_notif->image_size));
}
{
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
	 */
	if (ret)
		return 0;

	/*
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
{
	union acpi_object *wifi_pkg, *data, *flags;
	int i, j, ret, tbl_rev, num_sub_bands;
	int idx = 2;
	s8 *gain;

	/*
	 * The 'flags' field is the same in v1 and in v2 so we can just
	 * use v1 to access it.
	 */
	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);

	data = iwl_acpi_get_object(mvm->dev, ACPI_PPAG_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	/* try to read ppag table rev 2 or 1 (both have the same data size) */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V2, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev == 1 || tbl_rev == 2) {
			num_sub_bands = IWL_NUM_SUB_BANDS_V2;
			gain = mvm->fwrt.ppag_table.v2.gain[0];
			mvm->fwrt.ppag_ver = tbl_rev;
			IWL_DEBUG_RADIO(mvm,
					"Reading PPAG table v2 (tbl_rev=%d)\n",
					tbl_rev);
			goto read_table;
		} else {
			ret = -EINVAL;
			goto out_free;
		}
	}

	/* try to read ppag table revision 0 */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V1, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev != 0) {
			ret = -EINVAL;
			goto out_free;
		}
		num_sub_bands = IWL_NUM_SUB_BANDS_V1;
		gain = mvm->fwrt.ppag_table.v1.gain[0];
		mvm->fwrt.ppag_ver = 0;
		IWL_DEBUG_RADIO(mvm, "Reading PPAG table v1 (tbl_rev=0)\n");
		goto read_table;
	}
	ret = PTR_ERR(wifi_pkg);
	goto out_free;

read_table:
	flags = &wifi_pkg->package.elements[1];

	if (flags->type != ACPI_TYPE_INTEGER) {
		ret = -EINVAL;
		goto out_free;
	}

	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(flags->integer.value &
						    IWL_PPAG_MASK);

	if (!mvm->fwrt.ppag_table.v1.flags) {
		ret = 0;
		goto out_free;
	}

	/*
	 * read, verify gain values and save them into the PPAG table.
	 * first sub-band (j=0) corresponds to Low-Band (2.4GHz), and the
	 * following sub-bands to High-Band (5GHz).
	 */
	for (i = 0; i < IWL_NUM_CHAIN_LIMITS; i++) {
		for (j = 0; j < num_sub_bands; j++) {
			union acpi_object *ent;

			ent = &wifi_pkg->package.elements[idx++];
			if (ent->type != ACPI_TYPE_INTEGER) {
				ret = -EINVAL;
				goto out_free;
			}

			gain[i * num_sub_bands + j] = ent->integer.value;

			if ((j == 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_LB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_LB)) ||
			    (j != 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_HB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_HB))) {
				mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);
				ret = -EINVAL;
				goto out_free;
			}
		}
	}

	ret = 0;
out_free:
	kfree(data);
	return ret;
}

int iwl_mvm_ppag_send_cmd(struct iwl_mvm *mvm)
{
	u8 cmd_ver;
	int i, j, ret, num_sub_bands, cmd_size;
	s8 *gain;

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_SET_PPAG)) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG capability not supported by FW, command not sent.\n");
		return 0;
	}
	if (!mvm->fwrt.ppag_table.v1.flags) {
		IWL_DEBUG_RADIO(mvm, "PPAG not enabled, command not sent.\n");
		return 0;
	}

	cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, PHY_OPS_GROUP,
					PER_PLATFORM_ANT_GAIN_CMD,
					IWL_FW_CMD_VER_UNKNOWN);
	if (cmd_ver == 1) {
		num_sub_bands = IWL_NUM_SUB_BANDS_V1;
		gain = mvm->fwrt.ppag_table.v1.gain[0];
		cmd_size = sizeof(mvm->fwrt.ppag_table.v1);
		if (mvm->fwrt.ppag_ver == 1 || mvm->fwrt.ppag_ver == 2) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table rev is %d but FW supports v1, sending truncated table\n",
					mvm->fwrt.ppag_ver);
			mvm->fwrt.ppag_table.v1.flags &=
				cpu_to_le32(IWL_PPAG_ETSI_MASK);
		}
	} else if (cmd_ver == 2 || cmd_ver == 3) {
		num_sub_bands = IWL_NUM_SUB_BANDS_V2;
		gain = mvm->fwrt.ppag_table.v2.gain[0];
		cmd_size = sizeof(mvm->fwrt.ppag_table.v2);
		if (mvm->fwrt.ppag_ver == 0) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table is v1 but FW supports v2, sending padded table\n");
		} else if (cmd_ver == 2 && mvm->fwrt.ppag_ver == 2) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table is v3 but FW supports v2, sending partial bitmap.\n");
			mvm->fwrt.ppag_table.v1.flags &=
				cpu_to_le32(IWL_PPAG_ETSI_MASK);
		}
	} else {
		IWL_DEBUG_RADIO(mvm, "Unsupported PPAG command version\n");
		return 0;
	}

	for (i = 0; i < IWL_NUM_CHAIN_LIMITS; i++) {
		for (j = 0; j < num_sub_bands; j++) {
			IWL_DEBUG_RADIO(mvm,
					"PPAG table: chain[%d] band[%d]: gain = %d\n",
					i, j, gain[i * num_sub_bands + j]);
		}
	}
	IWL_DEBUG_RADIO(mvm, "Sending PER_PLATFORM_ANT_GAIN_CMD\n");
	ret = iwl_mvm_send_cmd_pdu(mvm, WIDE_ID(PHY_OPS_GROUP,
						PER_PLATFORM_ANT_GAIN_CMD),
				   0, cmd_size, &mvm->fwrt.ppag_table);
	if (ret < 0)
		IWL_ERR(mvm, "failed to send PER_PLATFORM_ANT_GAIN_CMD (%d)\n",
			ret);

	return ret;
}

static const struct dmi_system_id dmi_ppag_approved_list[] = {
	{ .ident = "HP",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "HP"),
		},
	},
	{ .ident = "SAMSUNG",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "SAMSUNG ELECTRONICS CO., LTD"),
		},
	},
	{ .ident = "MSFT",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
		},
	},
	{ .ident = "ASUS",
	  .matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTek COMPUTER INC."),
		},
	},
	{}
};

static int iwl_mvm_ppag_init(struct iwl_mvm *mvm)
{
	/* no need to read the table, done in INIT stage */
	if (!dmi_check_system(dmi_ppag_approved_list)) {
		IWL_DEBUG_RADIO(mvm,
				"System vendor '%s' is not in the approved list, disabling PPAG.\n",
				dmi_get_system_info(DMI_SYS_VENDOR));
		mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);
		return 0;
	}

	return iwl_mvm_ppag_send_cmd(mvm);
}

static void iwl_mvm_tas_init(struct iwl_mvm *mvm)
{
	int ret;
	struct iwl_tas_config_cmd cmd = {};
	int list_size;

	BUILD_BUG_ON(ARRAY_SIZE(cmd.block_list_array) <
		     APCI_WTAS_BLACK_LIST_MAX);

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_TAS_CFG)) {
		IWL_DEBUG_RADIO(mvm, "TAS not enabled in FW\n");
		return;
	}

	ret = iwl_acpi_get_tas(&mvm->fwrt, cmd.block_list_array, &list_size);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"TAS table invalid or unavailable. (%d)\n",
				ret);
		return;
	}

	if (list_size < 0)
		return;

	/* list size if TAS enabled can only be non-negative */
	cmd.block_list_size = cpu_to_le32((u32)list_size);

	ret = iwl_mvm_send_cmd_pdu(mvm, WIDE_ID(REGULATORY_AND_NVM_GROUP,
						TAS_CONFIG),
				   0, sizeof(cmd), &cmd);
	if (ret < 0)
		IWL_DEBUG_RADIO(mvm, "failed to send TAS_CONFIG (%d)\n", ret);
}

static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	u8 value;
	int ret = iwl_acpi_get_dsm_u8((&mvm->fwrt)->dev, 0, DSM_RFI_FUNC_ENABLE,
				      &iwl_rfi_guid, &value);

	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm, "Failed to get DSM RFI, ret=%d\n", ret);

	} else if (value >= DSM_VALUE_RFI_MAX) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI got invalid value, ret=%d\n",
				value);

	} else if (value == DSM_VALUE_RFI_ENABLE) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI is evaluated to enable\n");
		return DSM_VALUE_RFI_ENABLE;
	}

	IWL_DEBUG_RADIO(mvm, "DSM RFI is disabled\n");

	/* default behaviour is disabled */
	return DSM_VALUE_RFI_DISABLE;
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
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
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v2);
		else
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v1);

		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, config_bitmap=0x%x, oem_11ax_allow_bitmap=0x%x\n",
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
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"Failed to send LARI_CONFIG_CHANGE (%d)\n",
					ret);
	}
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
	int ret;

	/* read PPAG table */
	ret = iwl_mvm_get_ppag_table(mvm);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG BIOS table invalid or unavailable. (%d)\n",
				ret);
	}

	/* read SAR tables */
	ret = iwl_sar_get_wrds_table(&mvm->fwrt);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"WRDS SAR BIOS table invalid or unavailable. (%d)\n",
				ret);
		/*
		 * If not available, don't fail and don't bother with EWRD and
		 * WGDS */

		if (!iwl_sar_get_wgds_table(&mvm->fwrt)) {
			/*
			 * If basic SAR is not available, we check for WGDS,
			 * which should *not* be available either.  If it is
			 * available, issue an error, because we can't use SAR
			 * Geo without basic SAR.
			 */
			IWL_ERR(mvm, "BIOS contains WGDS but no WRDS\n");
		}

	} else {
		ret = iwl_sar_get_ewrd_table(&mvm->fwrt);
		/* if EWRD is not available, we can still use
		* WRDS, so don't fail */
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"EWRD SAR BIOS table invalid or unavailable. (%d)\n",
					ret);

		/* read geo SAR table */
		if (iwl_sar_geo_support(&mvm->fwrt)) {
			ret = iwl_sar_get_wgds_table(&mvm->fwrt);
			if (ret < 0)
				IWL_DEBUG_RADIO(mvm,
						"Geo SAR BIOS table invalid or unavailable. (%d)\n",
						ret);
				/* we don't fail if the table is not available */
		}
	}
}
#else /* CONFIG_ACPI */

inline int iwl_mvm_sar_select_profile(struct iwl_mvm *mvm,
				      int prof_a, int prof_b)
{
	return 1;
}

inline int iwl_mvm_get_sar_geo_profile(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_sar_geo_init(struct iwl_mvm *mvm)
{
	return 0;
}

int iwl_mvm_ppag_send_cmd(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_ppag_init(struct iwl_mvm *mvm)
{
	return 0;
}

static void iwl_mvm_tas_init(struct iwl_mvm *mvm)
{
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
{
}

static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	return DSM_VALUE_RFI_DISABLE;
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
}
#endif /* CONFIG_ACPI */

void iwl_mvm_send_recovery_cmd(struct iwl_mvm *mvm, u32 flags)
{
	u32 error_log_size = mvm->fw->ucode_capa.error_log_size;
	int ret;
	u32 resp;

	struct iwl_fw_error_recovery_cmd recovery_cmd = {
		.flags = cpu_to_le32(flags),
		.buf_size = 0,
	};
	struct iwl_host_cmd host_cmd = {
		.id = WIDE_ID(SYSTEM_GROUP, FW_ERROR_RECOVERY_CMD),
		.flags = CMD_WANT_SKB,
		.data = {&recovery_cmd, },
		.len = {sizeof(recovery_cmd), },
	};

	/* no error log was defined in TLV */
	if (!error_log_size)
		return;

	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		/* no buf was allocated while HW reset */
		if (!mvm->error_recovery_buf)
			return;

		host_cmd.data[1] = mvm->error_recovery_buf;
		host_cmd.len[1] =  error_log_size;
		host_cmd.dataflags[1] = IWL_HCMD_DFL_NOCOPY;
		recovery_cmd.buf_size = cpu_to_le32(error_log_size);
	}

	ret = iwl_mvm_send_cmd(mvm, &host_cmd);
	kfree(mvm->error_recovery_buf);
	mvm->error_recovery_buf = NULL;

	if (ret) {
		IWL_ERR(mvm, "Failed to send recovery cmd %d\n", ret);
		return;
	}

	/* skb respond is only relevant in ERROR_RECOVERY_UPDATE_DB */
	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		resp = le32_to_cpu(*(__le32 *)host_cmd.resp_pkt->data);
		if (resp)
			IWL_ERR(mvm,
				"Failed to send recovery cmd blob was invalid %d\n",
				resp);
	}
}

static int iwl_mvm_sar_init(struct iwl_mvm *mvm)
{
	return iwl_mvm_sar_select_profile(mvm, 1, 1);
}

static int iwl_mvm_load_rt_fw(struct iwl_mvm *mvm)
{
	int ret;

	if (iwl_mvm_has_unified_ucode(mvm))
		return iwl_run_unified_mvm_ucode(mvm);

	WARN_ON(!mvm->nvm_data);
	ret = iwl_run_init_mvm_ucode(mvm);

	if (ret) {
		IWL_ERR(mvm, "Failed to run INIT ucode: %d\n", ret);

		if (iwlmvm_mod_params.init_dbg)
			return 0;
		return ret;
	}

	iwl_fw_dbg_stop_sync(&mvm->fwrt);
	iwl_trans_stop_device(mvm->trans);
	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = false;
	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_REGULAR);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = true;

	iwl_dbg_tlv_time_point(&mvm->fwrt, IWL_FW_INI_TIME_POINT_AFTER_ALIVE,
			       NULL);

	return iwl_init_paging(&mvm->fwrt, mvm->fwrt.cur_fw_img);
}

int iwl_mvm_up(struct iwl_mvm *mvm)
{
	int ret, i;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	struct ieee80211_supported_band *sband = NULL;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_rt_fw(mvm);
	if (ret) {
		IWL_ERR(mvm, "Failed to start RT ucode: %d\n", ret);
		if (ret != -ERFKILL)
			iwl_fw_dbg_error_collect(&mvm->fwrt,
						 FW_DBG_TRIGGER_DRIVER);
		goto error;
	}

	iwl_get_shared_mem_conf(&mvm->fwrt);

	ret = iwl_mvm_sf_update(mvm, NULL, false);
	if (ret)
		IWL_ERR(mvm, "Failed to initialize Smart Fifo\n");

	if (!iwl_trans_dbg_ini_valid(mvm->trans)) {
		mvm->fwrt.dump.conf = FW_DBG_INVALID;
		/* if we have a destination, assume EARLY START */
		if (mvm->fw->dbg.dest_tlv)
			mvm->fwrt.dump.conf = FW_DBG_START_FROM_ALIVE;
		iwl_fw_start_dbg_conf(&mvm->fwrt, FW_DBG_START_FROM_ALIVE);
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	if (!iwl_mvm_has_unified_ucode(mvm)) {
		/* Send phy db control command and then phy db calibration */
		ret = iwl_send_phy_db_data(mvm->phy_db);
		if (ret)
			goto error;
	}

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_send_bt_init_conf(mvm);
	if (ret)
		goto error;

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_SOC_LATENCY_SUPPORT)) {
		ret = iwl_set_soc_latency(&mvm->fwrt);
		if (ret)
			goto error;
	}

	/* Init RSS configuration */
	ret = iwl_configure_rxq(&mvm->fwrt);
	if (ret)
		goto error;

	if (iwl_mvm_has_new_rx_api(mvm)) {
		ret = iwl_send_rss_cfg_cmd(mvm);
		if (ret) {
			IWL_ERR(mvm, "Failed to configure RSS queues: %d\n",
				ret);
			goto error;
		}
	}

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	mvm->tdls_cs.peer.sta_id = IWL_MVM_INVALID_STA;

	/* reset quota debouncing buffer - 0xff will yield invalid data */
	memset(&mvm->last_quota_cmd, 0xff, sizeof(mvm->last_quota_cmd));

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_DQA_SUPPORT)) {
		ret = iwl_mvm_send_dqa_cmd(mvm);
		if (ret)
			goto error;
	}

	/*
	 * Add auxiliary station for scanning.
	 * Newer versions of this command implies that the fw uses
	 * internal aux station for all aux activities that don't
	 * requires a dedicated data queue.
	 */
	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		 /*
		  * In old version the aux station uses mac id like other
		  * station and not lmac id
		  */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	/* Add all the PHY contexts */
	i = 0;
	while (!sband && i < NUM_NL80211_BANDS)
		sband = mvm->hw->wiphy->bands[i++];

	if (WARN_ON_ONCE(!sband))
		goto error;

	chan = &sband->channels[0];

	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);
	for (i = 0; i < NUM_PHY_CTX; i++) {
		/*
		 * The channel used here isn't relevant as it's
		 * going to be overwritten in the other flows.
		 * For now use the first channel we have.
		 */
		ret = iwl_mvm_phy_ctxt_add(mvm, &mvm->phy_ctxts[i],
					   &chandef, 1, 1);
		if (ret)
			goto error;
	}

	if (iwl_mvm_is_tt_in_fw(mvm)) {
		/* in order to give the responsibility of ct-kill and
		 * TX backoff to FW we need to send empty temperature reporting
		 * cmd during init time
		 */
		iwl_mvm_send_temp_report_ths_cmd(mvm);
	} else {
		/* Initialize tx backoffs to the minimal possible */
		iwl_mvm_tt_tx_backoff(mvm, 0);
	}

#ifdef CONFIG_THERMAL
	/* TODO: read the budget from BIOS / Platform NVM */

	/*
	 * In case there is no budget from BIOS / Platform NVM the default
	 * budget should be 2000mW (cooling state 0).
	 */
	if (iwl_mvm_is_ctdp_supported(mvm)) {
		ret = iwl_mvm_ctdp_command(mvm, CTDP_CMD_OPERATION_START,
					   mvm->cooling_dev.cur_state);
		if (ret)
			goto error;
	}
#endif

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_SET_LTR_GEN2))
		WARN_ON(iwl_mvm_config_ltr(mvm));

	ret = iwl_mvm_power_update_device(mvm);
	if (ret)
		goto error;

	iwl_mvm_lari_cfg(mvm);
	/*
	 * RTNL is not taken during Ct-kill, but we don't need to scan/Tx
	 * anyway, so don't init MCC.
	 */
	if (!test_bit(IWL_MVM_STATUS_HW_CTKILL, &mvm->status)) {
		ret = iwl_mvm_init_mcc(mvm);
		if (ret)
			goto error;
	}

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_UMAC_SCAN)) {
		mvm->scan_type = IWL_SCAN_TYPE_NOT_SET;
		mvm->hb_scan_type = IWL_SCAN_TYPE_NOT_SET;
		ret = iwl_mvm_config_scan(mvm);
		if (ret)
			goto error;
	}

	if (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status))
		iwl_mvm_send_recovery_cmd(mvm, ERROR_RECOVERY_UPDATE_DB);

	if (iwl_acpi_get_eckv(mvm->dev, &mvm->ext_clock_valid))
		IWL_DEBUG_INFO(mvm, "ECKV table doesn't exist in BIOS\n");

	ret = iwl_mvm_ppag_init(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_sar_init(mvm);
	if (ret == 0)
		ret = iwl_mvm_sar_geo_init(mvm);
	else if (ret < 0)
		goto error;

	iwl_mvm_tas_init(mvm);
	iwl_mvm_leds_sync(mvm);

	iwl_mvm_ftm_initiator_smooth_config(mvm);

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_RFIM_SUPPORT)) {
		if (iwl_mvm_eval_dsm_rfi(mvm) == DSM_VALUE_RFI_ENABLE)
			iwl_rfi_send_config_cmd(mvm, NULL);
	}

	IWL_DEBUG_INFO(mvm, "RT uCode started.\n");
	return 0;
 error:
	if (!iwlmvm_mod_params.init_dbg || !ret)
		iwl_mvm_stop_device(mvm);
	return ret;
}

int iwl_mvm_load_d3_fw(struct iwl_mvm *mvm)
{
	int ret, i;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_WOWLAN);
	if (ret) {
		IWL_ERR(mvm, "Failed to start WoWLAN firmware: %d\n", ret);
		goto error;
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	/* Send phy db control command and then phy db calibration*/
	ret = iwl_send_phy_db_data(mvm->phy_db);
	if (ret)
		goto error;

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		/*
		 * Add auxiliary station for scanning.
		 * Newer versions of this command implies that the fw uses
		 * internal aux station for all aux activities that don't
		 * requires a dedicated data queue.
		 * In old version the aux station uses mac id like other
		 * station and not lmac id
		 */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	return 0;
 error:
	iwl_mvm_stop_device(mvm);
	return ret;
}

void iwl_mvm_rx_card_state_notif(struct iwl_mvm *mvm,
				 struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_card_state_notif *card_state_notif = (void *)pkt->data;
	u32 flags = le32_to_cpu(card_state_notif->flags);

	IWL_DEBUG_RF_KILL(mvm, "Card state received: HW:%s SW:%s CT:%s\n",
			  (flags & HW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & SW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & CT_KILL_CARD_DISABLED) ?
			  "Reached" : "Not reached");
}

void iwl_mvm_rx_mfuart_notif(struct iwl_mvm *mvm,
			     struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_mfuart_load_notif *mfuart_notif = (void *)pkt->data;

	IWL_DEBUG_INFO(mvm,
		       "MFUART: installed ver: 0x%08x, external ver: 0x%08x, status: 0x%08x, duration: 0x%08x\n",
		       le32_to_cpu(mfuart_notif->installed_ver),
		       le32_to_cpu(mfuart_notif->external_ver),
		       le32_to_cpu(mfuart_notif->status),
		       le32_to_cpu(mfuart_notif->duration));

	if (iwl_rx_packet_payload_len(pkt) == sizeof(*mfuart_notif))
		IWL_DEBUG_INFO(mvm,
			       "MFUART: image size: 0x%08x\n",
			       le32_to_cpu(mfuart_notif->image_size));
}
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
	 */
	if (ret)
		return 0;

	/*
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
{
	union acpi_object *wifi_pkg, *data, *flags;
	int i, j, ret, tbl_rev, num_sub_bands;
	int idx = 2;
	s8 *gain;

	/*
	 * The 'flags' field is the same in v1 and in v2 so we can just
	 * use v1 to access it.
	 */
	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);

	data = iwl_acpi_get_object(mvm->dev, ACPI_PPAG_METHOD);
	if (IS_ERR(data))
		return PTR_ERR(data);

	/* try to read ppag table rev 2 or 1 (both have the same data size) */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V2, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev == 1 || tbl_rev == 2) {
			num_sub_bands = IWL_NUM_SUB_BANDS_V2;
			gain = mvm->fwrt.ppag_table.v2.gain[0];
			mvm->fwrt.ppag_ver = tbl_rev;
			IWL_DEBUG_RADIO(mvm,
					"Reading PPAG table v2 (tbl_rev=%d)\n",
					tbl_rev);
			goto read_table;
		} else {
			ret = -EINVAL;
			goto out_free;
		}
	}

	/* try to read ppag table revision 0 */
	wifi_pkg = iwl_acpi_get_wifi_pkg(mvm->dev, data,
					 ACPI_PPAG_WIFI_DATA_SIZE_V1, &tbl_rev);
	if (!IS_ERR(wifi_pkg)) {
		if (tbl_rev != 0) {
			ret = -EINVAL;
			goto out_free;
		}
		num_sub_bands = IWL_NUM_SUB_BANDS_V1;
		gain = mvm->fwrt.ppag_table.v1.gain[0];
		mvm->fwrt.ppag_ver = 0;
		IWL_DEBUG_RADIO(mvm, "Reading PPAG table v1 (tbl_rev=0)\n");
		goto read_table;
	}
	ret = PTR_ERR(wifi_pkg);
	goto out_free;

read_table:
	flags = &wifi_pkg->package.elements[1];

	if (flags->type != ACPI_TYPE_INTEGER) {
		ret = -EINVAL;
		goto out_free;
	}

	mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(flags->integer.value &
						    IWL_PPAG_MASK);

	if (!mvm->fwrt.ppag_table.v1.flags) {
		ret = 0;
		goto out_free;
	}

	/*
	 * read, verify gain values and save them into the PPAG table.
	 * first sub-band (j=0) corresponds to Low-Band (2.4GHz), and the
	 * following sub-bands to High-Band (5GHz).
	 */
	for (i = 0; i < IWL_NUM_CHAIN_LIMITS; i++) {
		for (j = 0; j < num_sub_bands; j++) {
			union acpi_object *ent;

			ent = &wifi_pkg->package.elements[idx++];
			if (ent->type != ACPI_TYPE_INTEGER) {
				ret = -EINVAL;
				goto out_free;
			}

			gain[i * num_sub_bands + j] = ent->integer.value;

			if ((j == 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_LB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_LB)) ||
			    (j != 0 &&
			     (gain[i * num_sub_bands + j] > ACPI_PPAG_MAX_HB ||
			      gain[i * num_sub_bands + j] < ACPI_PPAG_MIN_HB))) {
				mvm->fwrt.ppag_table.v1.flags = cpu_to_le32(0);
				ret = -EINVAL;
				goto out_free;
			}
		}
	}

	ret = 0;
out_free:
	kfree(data);
	return ret;
}
{
	u8 value;
	int ret = iwl_acpi_get_dsm_u8((&mvm->fwrt)->dev, 0, DSM_RFI_FUNC_ENABLE,
				      &iwl_rfi_guid, &value);

	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm, "Failed to get DSM RFI, ret=%d\n", ret);

	} else if (value >= DSM_VALUE_RFI_MAX) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI got invalid value, ret=%d\n",
				value);

	} else if (value == DSM_VALUE_RFI_ENABLE) {
		IWL_DEBUG_RADIO(mvm, "DSM RFI is evaluated to enable\n");
		return DSM_VALUE_RFI_ENABLE;
	}

	IWL_DEBUG_RADIO(mvm, "DSM RFI is disabled\n");

	/* default behaviour is disabled */
	return DSM_VALUE_RFI_DISABLE;
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
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
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v2);
		else
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v1);

		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, config_bitmap=0x%x, oem_11ax_allow_bitmap=0x%x\n",
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
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"Failed to send LARI_CONFIG_CHANGE (%d)\n",
					ret);
	}
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
	int ret;

	/* read PPAG table */
	ret = iwl_mvm_get_ppag_table(mvm);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG BIOS table invalid or unavailable. (%d)\n",
				ret);
	}

	/* read SAR tables */
	ret = iwl_sar_get_wrds_table(&mvm->fwrt);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"WRDS SAR BIOS table invalid or unavailable. (%d)\n",
				ret);
		/*
		 * If not available, don't fail and don't bother with EWRD and
		 * WGDS */

		if (!iwl_sar_get_wgds_table(&mvm->fwrt)) {
			/*
			 * If basic SAR is not available, we check for WGDS,
			 * which should *not* be available either.  If it is
			 * available, issue an error, because we can't use SAR
			 * Geo without basic SAR.
			 */
			IWL_ERR(mvm, "BIOS contains WGDS but no WRDS\n");
		}

	} else {
		ret = iwl_sar_get_ewrd_table(&mvm->fwrt);
		/* if EWRD is not available, we can still use
		* WRDS, so don't fail */
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"EWRD SAR BIOS table invalid or unavailable. (%d)\n",
					ret);

		/* read geo SAR table */
		if (iwl_sar_geo_support(&mvm->fwrt)) {
			ret = iwl_sar_get_wgds_table(&mvm->fwrt);
			if (ret < 0)
				IWL_DEBUG_RADIO(mvm,
						"Geo SAR BIOS table invalid or unavailable. (%d)\n",
						ret);
				/* we don't fail if the table is not available */
		}
	}
}
#else /* CONFIG_ACPI */

inline int iwl_mvm_sar_select_profile(struct iwl_mvm *mvm,
				      int prof_a, int prof_b)
{
	return 1;
}

inline int iwl_mvm_get_sar_geo_profile(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_sar_geo_init(struct iwl_mvm *mvm)
{
	return 0;
}

int iwl_mvm_ppag_send_cmd(struct iwl_mvm *mvm)
{
	return -ENOENT;
}

static int iwl_mvm_ppag_init(struct iwl_mvm *mvm)
{
	return 0;
}

static void iwl_mvm_tas_init(struct iwl_mvm *mvm)
{
}

static void iwl_mvm_lari_cfg(struct iwl_mvm *mvm)
{
}

static u8 iwl_mvm_eval_dsm_rfi(struct iwl_mvm *mvm)
{
	return DSM_VALUE_RFI_DISABLE;
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
}
#endif /* CONFIG_ACPI */

void iwl_mvm_send_recovery_cmd(struct iwl_mvm *mvm, u32 flags)
{
	u32 error_log_size = mvm->fw->ucode_capa.error_log_size;
	int ret;
	u32 resp;

	struct iwl_fw_error_recovery_cmd recovery_cmd = {
		.flags = cpu_to_le32(flags),
		.buf_size = 0,
	};
	struct iwl_host_cmd host_cmd = {
		.id = WIDE_ID(SYSTEM_GROUP, FW_ERROR_RECOVERY_CMD),
		.flags = CMD_WANT_SKB,
		.data = {&recovery_cmd, },
		.len = {sizeof(recovery_cmd), },
	};

	/* no error log was defined in TLV */
	if (!error_log_size)
		return;

	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		/* no buf was allocated while HW reset */
		if (!mvm->error_recovery_buf)
			return;

		host_cmd.data[1] = mvm->error_recovery_buf;
		host_cmd.len[1] =  error_log_size;
		host_cmd.dataflags[1] = IWL_HCMD_DFL_NOCOPY;
		recovery_cmd.buf_size = cpu_to_le32(error_log_size);
	}

	ret = iwl_mvm_send_cmd(mvm, &host_cmd);
	kfree(mvm->error_recovery_buf);
	mvm->error_recovery_buf = NULL;

	if (ret) {
		IWL_ERR(mvm, "Failed to send recovery cmd %d\n", ret);
		return;
	}

	/* skb respond is only relevant in ERROR_RECOVERY_UPDATE_DB */
	if (flags & ERROR_RECOVERY_UPDATE_DB) {
		resp = le32_to_cpu(*(__le32 *)host_cmd.resp_pkt->data);
		if (resp)
			IWL_ERR(mvm,
				"Failed to send recovery cmd blob was invalid %d\n",
				resp);
	}
}

static int iwl_mvm_sar_init(struct iwl_mvm *mvm)
{
	return iwl_mvm_sar_select_profile(mvm, 1, 1);
}

static int iwl_mvm_load_rt_fw(struct iwl_mvm *mvm)
{
	int ret;

	if (iwl_mvm_has_unified_ucode(mvm))
		return iwl_run_unified_mvm_ucode(mvm);

	WARN_ON(!mvm->nvm_data);
	ret = iwl_run_init_mvm_ucode(mvm);

	if (ret) {
		IWL_ERR(mvm, "Failed to run INIT ucode: %d\n", ret);

		if (iwlmvm_mod_params.init_dbg)
			return 0;
		return ret;
	}

	iwl_fw_dbg_stop_sync(&mvm->fwrt);
	iwl_trans_stop_device(mvm->trans);
	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = false;
	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_REGULAR);
	if (ret)
		return ret;

	mvm->rfkill_safe_init_done = true;

	iwl_dbg_tlv_time_point(&mvm->fwrt, IWL_FW_INI_TIME_POINT_AFTER_ALIVE,
			       NULL);

	return iwl_init_paging(&mvm->fwrt, mvm->fwrt.cur_fw_img);
}

int iwl_mvm_up(struct iwl_mvm *mvm)
{
	int ret, i;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	struct ieee80211_supported_band *sband = NULL;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_rt_fw(mvm);
	if (ret) {
		IWL_ERR(mvm, "Failed to start RT ucode: %d\n", ret);
		if (ret != -ERFKILL)
			iwl_fw_dbg_error_collect(&mvm->fwrt,
						 FW_DBG_TRIGGER_DRIVER);
		goto error;
	}

	iwl_get_shared_mem_conf(&mvm->fwrt);

	ret = iwl_mvm_sf_update(mvm, NULL, false);
	if (ret)
		IWL_ERR(mvm, "Failed to initialize Smart Fifo\n");

	if (!iwl_trans_dbg_ini_valid(mvm->trans)) {
		mvm->fwrt.dump.conf = FW_DBG_INVALID;
		/* if we have a destination, assume EARLY START */
		if (mvm->fw->dbg.dest_tlv)
			mvm->fwrt.dump.conf = FW_DBG_START_FROM_ALIVE;
		iwl_fw_start_dbg_conf(&mvm->fwrt, FW_DBG_START_FROM_ALIVE);
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	if (!iwl_mvm_has_unified_ucode(mvm)) {
		/* Send phy db control command and then phy db calibration */
		ret = iwl_send_phy_db_data(mvm->phy_db);
		if (ret)
			goto error;
	}

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_send_bt_init_conf(mvm);
	if (ret)
		goto error;

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_SOC_LATENCY_SUPPORT)) {
		ret = iwl_set_soc_latency(&mvm->fwrt);
		if (ret)
			goto error;
	}

	/* Init RSS configuration */
	ret = iwl_configure_rxq(&mvm->fwrt);
	if (ret)
		goto error;

	if (iwl_mvm_has_new_rx_api(mvm)) {
		ret = iwl_send_rss_cfg_cmd(mvm);
		if (ret) {
			IWL_ERR(mvm, "Failed to configure RSS queues: %d\n",
				ret);
			goto error;
		}
	}

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	mvm->tdls_cs.peer.sta_id = IWL_MVM_INVALID_STA;

	/* reset quota debouncing buffer - 0xff will yield invalid data */
	memset(&mvm->last_quota_cmd, 0xff, sizeof(mvm->last_quota_cmd));

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_DQA_SUPPORT)) {
		ret = iwl_mvm_send_dqa_cmd(mvm);
		if (ret)
			goto error;
	}

	/*
	 * Add auxiliary station for scanning.
	 * Newer versions of this command implies that the fw uses
	 * internal aux station for all aux activities that don't
	 * requires a dedicated data queue.
	 */
	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		 /*
		  * In old version the aux station uses mac id like other
		  * station and not lmac id
		  */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	/* Add all the PHY contexts */
	i = 0;
	while (!sband && i < NUM_NL80211_BANDS)
		sband = mvm->hw->wiphy->bands[i++];

	if (WARN_ON_ONCE(!sband))
		goto error;

	chan = &sband->channels[0];

	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);
	for (i = 0; i < NUM_PHY_CTX; i++) {
		/*
		 * The channel used here isn't relevant as it's
		 * going to be overwritten in the other flows.
		 * For now use the first channel we have.
		 */
		ret = iwl_mvm_phy_ctxt_add(mvm, &mvm->phy_ctxts[i],
					   &chandef, 1, 1);
		if (ret)
			goto error;
	}

	if (iwl_mvm_is_tt_in_fw(mvm)) {
		/* in order to give the responsibility of ct-kill and
		 * TX backoff to FW we need to send empty temperature reporting
		 * cmd during init time
		 */
		iwl_mvm_send_temp_report_ths_cmd(mvm);
	} else {
		/* Initialize tx backoffs to the minimal possible */
		iwl_mvm_tt_tx_backoff(mvm, 0);
	}

#ifdef CONFIG_THERMAL
	/* TODO: read the budget from BIOS / Platform NVM */

	/*
	 * In case there is no budget from BIOS / Platform NVM the default
	 * budget should be 2000mW (cooling state 0).
	 */
	if (iwl_mvm_is_ctdp_supported(mvm)) {
		ret = iwl_mvm_ctdp_command(mvm, CTDP_CMD_OPERATION_START,
					   mvm->cooling_dev.cur_state);
		if (ret)
			goto error;
	}
#endif

	if (!fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_SET_LTR_GEN2))
		WARN_ON(iwl_mvm_config_ltr(mvm));

	ret = iwl_mvm_power_update_device(mvm);
	if (ret)
		goto error;

	iwl_mvm_lari_cfg(mvm);
	/*
	 * RTNL is not taken during Ct-kill, but we don't need to scan/Tx
	 * anyway, so don't init MCC.
	 */
	if (!test_bit(IWL_MVM_STATUS_HW_CTKILL, &mvm->status)) {
		ret = iwl_mvm_init_mcc(mvm);
		if (ret)
			goto error;
	}

	if (fw_has_capa(&mvm->fw->ucode_capa, IWL_UCODE_TLV_CAPA_UMAC_SCAN)) {
		mvm->scan_type = IWL_SCAN_TYPE_NOT_SET;
		mvm->hb_scan_type = IWL_SCAN_TYPE_NOT_SET;
		ret = iwl_mvm_config_scan(mvm);
		if (ret)
			goto error;
	}

	if (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status))
		iwl_mvm_send_recovery_cmd(mvm, ERROR_RECOVERY_UPDATE_DB);

	if (iwl_acpi_get_eckv(mvm->dev, &mvm->ext_clock_valid))
		IWL_DEBUG_INFO(mvm, "ECKV table doesn't exist in BIOS\n");

	ret = iwl_mvm_ppag_init(mvm);
	if (ret)
		goto error;

	ret = iwl_mvm_sar_init(mvm);
	if (ret == 0)
		ret = iwl_mvm_sar_geo_init(mvm);
	else if (ret < 0)
		goto error;

	iwl_mvm_tas_init(mvm);
	iwl_mvm_leds_sync(mvm);

	iwl_mvm_ftm_initiator_smooth_config(mvm);

	if (fw_has_capa(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_RFIM_SUPPORT)) {
		if (iwl_mvm_eval_dsm_rfi(mvm) == DSM_VALUE_RFI_ENABLE)
			iwl_rfi_send_config_cmd(mvm, NULL);
	}

	IWL_DEBUG_INFO(mvm, "RT uCode started.\n");
	return 0;
 error:
	if (!iwlmvm_mod_params.init_dbg || !ret)
		iwl_mvm_stop_device(mvm);
	return ret;
}

int iwl_mvm_load_d3_fw(struct iwl_mvm *mvm)
{
	int ret, i;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_trans_start_hw(mvm->trans);
	if (ret)
		return ret;

	ret = iwl_mvm_load_ucode_wait_alive(mvm, IWL_UCODE_WOWLAN);
	if (ret) {
		IWL_ERR(mvm, "Failed to start WoWLAN firmware: %d\n", ret);
		goto error;
	}

	ret = iwl_send_tx_ant_cfg(mvm, iwl_mvm_get_valid_tx_ant(mvm));
	if (ret)
		goto error;

	/* Send phy db control command and then phy db calibration*/
	ret = iwl_send_phy_db_data(mvm->phy_db);
	if (ret)
		goto error;

	ret = iwl_send_phy_cfg_cmd(mvm);
	if (ret)
		goto error;

	/* init the fw <-> mac80211 STA mapping */
	for (i = 0; i < mvm->fw->ucode_capa.num_stations; i++)
		RCU_INIT_POINTER(mvm->fw_id_to_mac_id[i], NULL);

	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
				  ADD_STA,
				  0) < 12) {
		/*
		 * Add auxiliary station for scanning.
		 * Newer versions of this command implies that the fw uses
		 * internal aux station for all aux activities that don't
		 * requires a dedicated data queue.
		 * In old version the aux station uses mac id like other
		 * station and not lmac id
		 */
		ret = iwl_mvm_add_aux_sta(mvm, MAC_INDEX_AUX);
		if (ret)
			goto error;
	}

	return 0;
 error:
	iwl_mvm_stop_device(mvm);
	return ret;
}

void iwl_mvm_rx_card_state_notif(struct iwl_mvm *mvm,
				 struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_card_state_notif *card_state_notif = (void *)pkt->data;
	u32 flags = le32_to_cpu(card_state_notif->flags);

	IWL_DEBUG_RF_KILL(mvm, "Card state received: HW:%s SW:%s CT:%s\n",
			  (flags & HW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & SW_CARD_DISABLED) ? "Kill" : "On",
			  (flags & CT_KILL_CARD_DISABLED) ?
			  "Reached" : "Not reached");
}

void iwl_mvm_rx_mfuart_notif(struct iwl_mvm *mvm,
			     struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_mfuart_load_notif *mfuart_notif = (void *)pkt->data;

	IWL_DEBUG_INFO(mvm,
		       "MFUART: installed ver: 0x%08x, external ver: 0x%08x, status: 0x%08x, duration: 0x%08x\n",
		       le32_to_cpu(mfuart_notif->installed_ver),
		       le32_to_cpu(mfuart_notif->external_ver),
		       le32_to_cpu(mfuart_notif->status),
		       le32_to_cpu(mfuart_notif->duration));

	if (iwl_rx_packet_payload_len(pkt) == sizeof(*mfuart_notif))
		IWL_DEBUG_INFO(mvm,
			       "MFUART: image size: 0x%08x\n",
			       le32_to_cpu(mfuart_notif->image_size));
}
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
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v2);
		else
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v1);

		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, config_bitmap=0x%x, oem_11ax_allow_bitmap=0x%x\n",
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
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"Failed to send LARI_CONFIG_CHANGE (%d)\n",
					ret);
	}
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
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v2);
		else
			cmd_size = sizeof(struct iwl_lari_config_change_cmd_v1);

		IWL_DEBUG_RADIO(mvm,
				"sending LARI_CONFIG_CHANGE, config_bitmap=0x%x, oem_11ax_allow_bitmap=0x%x\n",
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
		if (ret < 0)
			IWL_DEBUG_RADIO(mvm,
					"Failed to send LARI_CONFIG_CHANGE (%d)\n",
					ret);
	}
}

void iwl_mvm_get_acpi_tables(struct iwl_mvm *mvm)
{
	int ret;

	/* read PPAG table */
	ret = iwl_mvm_get_ppag_table(mvm);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"PPAG BIOS table invalid or unavailable. (%d)\n",
				ret);
	}

	/* read SAR tables */
	ret = iwl_sar_get_wrds_table(&mvm->fwrt);
	if (ret < 0) {
		IWL_DEBUG_RADIO(mvm,
				"WRDS SAR BIOS table invalid or unavailable. (%d)\n",
				ret);
		/*
		 * If not available, don't fail and don't bother with EWRD and
		 * WGDS */

		if (!iwl_sar_get_wgds_table(&mvm->fwrt)) {
			/*
			 * If basic SAR is not available, we check for WGDS,
			 * which should *not* be available either.  If it is
			 * available, issue an error, because we can't use SAR
			 * Geo without basic SAR.
			 */
			IWL_ERR(mvm, "BIOS contains WGDS but no WRDS\n");
		}