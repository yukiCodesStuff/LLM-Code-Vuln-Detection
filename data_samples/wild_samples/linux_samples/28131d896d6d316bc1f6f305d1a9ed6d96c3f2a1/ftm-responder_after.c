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
					   u8 *format_bw, u8 *ctrl_ch_position,
					   u8 cmd_ver)
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
	case NL80211_CHAN_WIDTH_160:
		if (cmd_ver >= 9) {
			*format_bw = IWL_LOCATION_FRAME_FORMAT_HE;
			*format_bw |= IWL_LOCATION_BW_160MHZ << LOCATION_BW_POS;
			*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
			break;
		}
		fallthrough;
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
	case NL80211_CHAN_WIDTH_160:
		if (cmd_ver >= 9) {
			*format_bw = IWL_LOCATION_FRAME_FORMAT_HE;
			*format_bw |= IWL_LOCATION_BW_160MHZ << LOCATION_BW_POS;
			*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
			break;
		}
	} else {
		/* All versions up to version 8 have the same size */
		cmd_size = sizeof(struct iwl_tof_responder_config_cmd_v8);
	}

	if (cmd_ver >= 8)
		iwl_mvm_ftm_responder_set_ndp(mvm, &cmd);

	if (cmd_ver >= 7)
		err = iwl_mvm_ftm_responder_set_bw_v2(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position,
						      cmd_ver);
	else
		err = iwl_mvm_ftm_responder_set_bw_v1(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);

	if (err) {
		IWL_ERR(mvm, "Failed to set responder bandwidth\n");
		return err;
	}