	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		*bw = IWL_TOF_BW_20_LEGACY;
		break;
	case NL80211_CHAN_WIDTH_20:
		*bw = IWL_TOF_BW_20_HT;
		break;
	case NL80211_CHAN_WIDTH_40:
		*bw = IWL_TOF_BW_40;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	case NL80211_CHAN_WIDTH_80:
		*bw = IWL_TOF_BW_80;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int iwl_mvm_ftm_responder_set_bw_v2(struct cfg80211_chan_def *chandef,
					   u8 *format_bw,
					   u8 *ctrl_ch_position)
{
	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_LEGACY;
		*format_bw |= IWL_LOCATION_BW_20MHZ << LOCATION_BW_POS;
		break;
	case NL80211_CHAN_WIDTH_20:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_HT;
		*format_bw |= IWL_LOCATION_BW_20MHZ << LOCATION_BW_POS;
		break;
	case NL80211_CHAN_WIDTH_40:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_HT;
		*format_bw |= IWL_LOCATION_BW_40MHZ << LOCATION_BW_POS;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	case NL80211_CHAN_WIDTH_80:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_VHT;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}
	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_LEGACY;
		*format_bw |= IWL_LOCATION_BW_20MHZ << LOCATION_BW_POS;
		break;
	case NL80211_CHAN_WIDTH_20:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_HT;
		*format_bw |= IWL_LOCATION_BW_20MHZ << LOCATION_BW_POS;
		break;
	case NL80211_CHAN_WIDTH_40:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_HT;
		*format_bw |= IWL_LOCATION_BW_40MHZ << LOCATION_BW_POS;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	case NL80211_CHAN_WIDTH_80:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_VHT;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

static void
iwl_mvm_ftm_responder_set_ndp(struct iwl_mvm *mvm,
			      struct iwl_tof_responder_config_cmd_v9 *cmd)
{
	/* Up to 2 R2I STS are allowed on the responder */
	u32 r2i_max_sts = IWL_MVM_FTM_R2I_MAX_STS < 2 ?
		IWL_MVM_FTM_R2I_MAX_STS : 1;

	cmd->r2i_ndp_params = IWL_MVM_FTM_R2I_MAX_REP |
		(r2i_max_sts << IWL_RESPONDER_STS_POS) |
		(IWL_MVM_FTM_R2I_MAX_TOTAL_LTF << IWL_RESPONDER_TOTAL_LTF_POS);
	cmd->i2r_ndp_params = IWL_MVM_FTM_I2R_MAX_REP |
		(IWL_MVM_FTM_I2R_MAX_STS << IWL_RESPONDER_STS_POS) |
		(IWL_MVM_FTM_I2R_MAX_TOTAL_LTF << IWL_RESPONDER_TOTAL_LTF_POS);
	cmd->cmd_valid_fields |=
		cpu_to_le32(IWL_TOF_RESPONDER_CMD_VALID_NDP_PARAMS);
}

static int
iwl_mvm_ftm_responder_cmd(struct iwl_mvm *mvm,
			  struct ieee80211_vif *vif,
			  struct cfg80211_chan_def *chandef)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	/*
	 * The command structure is the same for versions 6, 7 and 8 (only the
	 * field interpretation is different), so the same struct can be use
	 * for all cases.
	 */
	struct iwl_tof_responder_config_cmd_v9 cmd = {
		.channel_num = chandef->chan->hw_value,
		.cmd_valid_fields =
			cpu_to_le32(IWL_TOF_RESPONDER_CMD_VALID_CHAN_INFO |
				    IWL_TOF_RESPONDER_CMD_VALID_BSSID |
				    IWL_TOF_RESPONDER_CMD_VALID_STA_ID),
		.sta_id = mvmvif->bcast_sta.sta_id,
	};
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LOCATION_GROUP,
					   TOF_RESPONDER_CONFIG_CMD, 6);
	int err;
	int cmd_size;

	lockdep_assert_held(&mvm->mutex);

	/* Use a default of bss_color=1 for now */
	if (cmd_ver == 9) {
		cmd.cmd_valid_fields |=
			cpu_to_le32(IWL_TOF_RESPONDER_CMD_VALID_BSS_COLOR |
				    IWL_TOF_RESPONDER_CMD_VALID_MIN_MAX_TIME_BETWEEN_MSR);
		cmd.bss_color = 1;
		cmd.min_time_between_msr =
			cpu_to_le16(IWL_MVM_FTM_NON_TB_MIN_TIME_BETWEEN_MSR);
		cmd.max_time_between_msr =
			cpu_to_le16(IWL_MVM_FTM_NON_TB_MAX_TIME_BETWEEN_MSR);
		cmd_size = sizeof(struct iwl_tof_responder_config_cmd_v9);
	} else {
		/* All versions up to version 8 have the same size */
		cmd_size = sizeof(struct iwl_tof_responder_config_cmd_v8);
	}

	if (cmd_ver >= 8)
		iwl_mvm_ftm_responder_set_ndp(mvm, &cmd);

	if (cmd_ver >= 7)
		err = iwl_mvm_ftm_responder_set_bw_v2(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);
	else
		err = iwl_mvm_ftm_responder_set_bw_v1(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);

	if (err) {
		IWL_ERR(mvm, "Failed to set responder bandwidth\n");
		return err;
	}

	memcpy(cmd.bssid, vif->addr, ETH_ALEN);

	return iwl_mvm_send_cmd_pdu(mvm, iwl_cmd_id(TOF_RESPONDER_CONFIG_CMD,
						    LOCATION_GROUP, 0),
				    0, cmd_size, &cmd);
}
	} else {
		/* All versions up to version 8 have the same size */
		cmd_size = sizeof(struct iwl_tof_responder_config_cmd_v8);
	}

	if (cmd_ver >= 8)
		iwl_mvm_ftm_responder_set_ndp(mvm, &cmd);

	if (cmd_ver >= 7)
		err = iwl_mvm_ftm_responder_set_bw_v2(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);
	else
		err = iwl_mvm_ftm_responder_set_bw_v1(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);

	if (err) {
		IWL_ERR(mvm, "Failed to set responder bandwidth\n");
		return err;
	}