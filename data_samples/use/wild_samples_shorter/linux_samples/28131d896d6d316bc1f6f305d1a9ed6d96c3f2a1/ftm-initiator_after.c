			      u8 *ctrl_ch_position)
{
	u32 freq = peer->chandef.chan->center_freq;
	u8 cmd_ver;

	*channel = ieee80211_frequency_to_channel(freq);

	switch (peer->chandef.width) {
		*format_bw = IWL_LOCATION_FRAME_FORMAT_VHT;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		break;
	case NL80211_CHAN_WIDTH_160:
		cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LOCATION_GROUP,
						TOF_RANGE_REQ_CMD,
						IWL_FW_CMD_VER_UNKNOWN);

		if (cmd_ver >= 13) {
			*format_bw = IWL_LOCATION_FRAME_FORMAT_HE;
			*format_bw |= IWL_LOCATION_BW_160MHZ << LOCATION_BW_POS;
			break;
		}
		fallthrough;
	default:
		IWL_ERR(mvm, "Unsupported BW in FTM request (%d)\n",
			peer->chandef.width);
		return -EINVAL;
static bool iwl_mvm_ftm_resp_size_validation(u8 ver, unsigned int pkt_len)
{
	switch (ver) {
	case 9:
	case 8:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v8);
	case 7:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v7);
		int peer_idx;

		if (new_api) {
			if (notif_ver >= 8) {
				fw_ap = &fw_resp_v8->ap[i];
				iwl_mvm_ftm_pasn_update_pn(mvm, fw_ap);
			} else if (notif_ver == 7) {
				fw_ap = (void *)&fw_resp_v7->ap[i];