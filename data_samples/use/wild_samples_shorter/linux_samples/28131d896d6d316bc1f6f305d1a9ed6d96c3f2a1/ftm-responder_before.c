}

static int iwl_mvm_ftm_responder_set_bw_v2(struct cfg80211_chan_def *chandef,
					   u8 *format_bw,
					   u8 *ctrl_ch_position)
{
	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_LEGACY;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		*ctrl_ch_position = iwl_mvm_get_ctrl_pos(chandef);
		break;
	default:
		return -ENOTSUPP;
	}


	if (cmd_ver >= 7)
		err = iwl_mvm_ftm_responder_set_bw_v2(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);
	else
		err = iwl_mvm_ftm_responder_set_bw_v1(chandef, &cmd.format_bw,
						      &cmd.ctrl_ch_position);
