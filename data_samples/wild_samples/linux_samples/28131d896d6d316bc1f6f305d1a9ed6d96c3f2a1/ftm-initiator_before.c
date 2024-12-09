{
	u32 freq = peer->chandef.chan->center_freq;

	*channel = ieee80211_frequency_to_channel(freq);

	switch (peer->chandef.width) {
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
		break;
	case NL80211_CHAN_WIDTH_80:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_VHT;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		break;
	default:
		IWL_ERR(mvm, "Unsupported BW in FTM request (%d)\n",
			peer->chandef.width);
		return -EINVAL;
	}

	/* non EDCA based measurement must use HE preamble */
	if (peer->ftm.trigger_based || peer->ftm.non_trigger_based)
		*format_bw |= IWL_LOCATION_FRAME_FORMAT_HE;

	*ctrl_ch_position = (peer->chandef.width > NL80211_CHAN_WIDTH_20) ?
		iwl_mvm_get_ctrl_pos(&peer->chandef) : 0;

	return 0;
}
	switch (peer->chandef.width) {
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
		break;
	case NL80211_CHAN_WIDTH_80:
		*format_bw = IWL_LOCATION_FRAME_FORMAT_VHT;
		*format_bw |= IWL_LOCATION_BW_80MHZ << LOCATION_BW_POS;
		break;
	default:
		IWL_ERR(mvm, "Unsupported BW in FTM request (%d)\n",
			peer->chandef.width);
		return -EINVAL;
	}

	/* non EDCA based measurement must use HE preamble */
	if (peer->ftm.trigger_based || peer->ftm.non_trigger_based)
		*format_bw |= IWL_LOCATION_FRAME_FORMAT_HE;

	*ctrl_ch_position = (peer->chandef.width > NL80211_CHAN_WIDTH_20) ?
		iwl_mvm_get_ctrl_pos(&peer->chandef) : 0;

	return 0;
}

static int
iwl_mvm_ftm_put_target_v2(struct iwl_mvm *mvm,
			  struct cfg80211_pmsr_request_peer *peer,
			  struct iwl_tof_range_req_ap_entry_v2 *target)
{
	int ret;

	ret = iwl_mvm_ftm_target_chandef_v1(mvm, peer, &target->channel_num,
					    &target->bandwidth,
					    &target->ctrl_ch_position);
	if (ret)
		return ret;

	memcpy(target->bssid, peer->addr, ETH_ALEN);
	target->burst_period =
		cpu_to_le16(peer->ftm.burst_period);
	target->samples_per_burst = peer->ftm.ftms_per_burst;
	target->num_of_bursts = peer->ftm.num_bursts_exp;
	target->measure_type = 0; /* regular two-sided FTM */
	target->retries_per_sample = peer->ftm.ftmr_retries;
	target->asap_mode = peer->ftm.asap;
	target->enable_dyn_ack = IWL_MVM_FTM_INITIATOR_DYNACK;

	if (peer->ftm.request_lci)
		target->location_req |= IWL_TOF_LOC_LCI;
	if (peer->ftm.request_civicloc)
		target->location_req |= IWL_TOF_LOC_CIVIC;

	target->algo_type = IWL_MVM_FTM_INITIATOR_ALGO;

	return 0;
}

#define FTM_PUT_FLAG(flag)	(target->initiator_ap_flags |= \
				 cpu_to_le32(IWL_INITIATOR_AP_FLAGS_##flag))

static void
iwl_mvm_ftm_put_target_common(struct iwl_mvm *mvm,
			      struct cfg80211_pmsr_request_peer *peer,
			      struct iwl_tof_range_req_ap_entry_v6 *target)
{
	memcpy(target->bssid, peer->addr, ETH_ALEN);
	target->burst_period =
		cpu_to_le16(peer->ftm.burst_period);
	target->samples_per_burst = peer->ftm.ftms_per_burst;
	target->num_of_bursts = peer->ftm.num_bursts_exp;
	target->ftmr_max_retries = peer->ftm.ftmr_retries;
	target->initiator_ap_flags = cpu_to_le32(0);

	if (peer->ftm.asap)
		FTM_PUT_FLAG(ASAP);

	if (peer->ftm.request_lci)
		FTM_PUT_FLAG(LCI_REQUEST);

	if (peer->ftm.request_civicloc)
		FTM_PUT_FLAG(CIVIC_REQUEST);

	if (IWL_MVM_FTM_INITIATOR_DYNACK)
		FTM_PUT_FLAG(DYN_ACK);

	if (IWL_MVM_FTM_INITIATOR_ALGO == IWL_TOF_ALGO_TYPE_LINEAR_REG)
		FTM_PUT_FLAG(ALGO_LR);
	else if (IWL_MVM_FTM_INITIATOR_ALGO == IWL_TOF_ALGO_TYPE_FFT)
		FTM_PUT_FLAG(ALGO_FFT);

	if (peer->ftm.trigger_based)
		FTM_PUT_FLAG(TB);
	else if (peer->ftm.non_trigger_based)
		FTM_PUT_FLAG(NON_TB);

	if ((peer->ftm.trigger_based || peer->ftm.non_trigger_based) &&
	    peer->ftm.lmr_feedback)
		FTM_PUT_FLAG(LMR_FEEDBACK);
}

static int
iwl_mvm_ftm_put_target_v3(struct iwl_mvm *mvm,
			  struct cfg80211_pmsr_request_peer *peer,
			  struct iwl_tof_range_req_ap_entry_v3 *target)
{
	int ret;

	ret = iwl_mvm_ftm_target_chandef_v1(mvm, peer, &target->channel_num,
					    &target->bandwidth,
					    &target->ctrl_ch_position);
	if (ret)
		return ret;

	/*
	 * Versions 3 and 4 has some common fields, so
	 * iwl_mvm_ftm_put_target_common() can be used for version 7 too.
	 */
	iwl_mvm_ftm_put_target_common(mvm, peer, (void *)target);

	return 0;
}

static int
iwl_mvm_ftm_put_target_v4(struct iwl_mvm *mvm,
			  struct cfg80211_pmsr_request_peer *peer,
			  struct iwl_tof_range_req_ap_entry_v4 *target)
{
	int ret;

	ret = iwl_mvm_ftm_target_chandef_v2(mvm, peer, &target->channel_num,
					    &target->format_bw,
					    &target->ctrl_ch_position);
	if (ret)
		return ret;

	iwl_mvm_ftm_put_target_common(mvm, peer, (void *)target);

	return 0;
}

static int
iwl_mvm_ftm_put_target(struct iwl_mvm *mvm, struct ieee80211_vif *vif,
		       struct cfg80211_pmsr_request_peer *peer,
		       struct iwl_tof_range_req_ap_entry_v6 *target)
{
	int ret;

	ret = iwl_mvm_ftm_target_chandef_v2(mvm, peer, &target->channel_num,
					    &target->format_bw,
					    &target->ctrl_ch_position);
	if (ret)
		return ret;

	iwl_mvm_ftm_put_target_common(mvm, peer, target);

	if (vif->bss_conf.assoc &&
	    !memcmp(peer->addr, vif->bss_conf.bssid, ETH_ALEN)) {
		struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
		struct ieee80211_sta *sta;

		rcu_read_lock();

		sta = rcu_dereference(mvm->fw_id_to_mac_id[mvmvif->ap_sta_id]);
		if (sta->mfp)
			FTM_PUT_FLAG(PMF);

		rcu_read_unlock();

		target->sta_id = mvmvif->ap_sta_id;
	} else {
		target->sta_id = IWL_MVM_INVALID_STA;
	}
{
	switch (ver) {
	case 8:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v8);
	case 7:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v7);
	case 6:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v6);
	case 5:
		return pkt_len == sizeof(struct iwl_tof_range_rsp_ntfy_v5);
	default:
		WARN_ONCE(1, "FTM: unsupported range response version %u", ver);
		return false;
	}
}
	for (i = 0; i < num_of_aps && i < IWL_MVM_TOF_MAX_APS; i++) {
		struct cfg80211_pmsr_result result = {};
		struct iwl_tof_range_rsp_ap_entry_ntfy_v6 *fw_ap;
		int peer_idx;

		if (new_api) {
			if (notif_ver == 8) {
				fw_ap = &fw_resp_v8->ap[i];
				iwl_mvm_ftm_pasn_update_pn(mvm, fw_ap);
			} else if (notif_ver == 7) {
				fw_ap = (void *)&fw_resp_v7->ap[i];
			} else {
				fw_ap = (void *)&fw_resp_v6->ap[i];
			}

			result.final = fw_ap->last_burst;
			result.ap_tsf = le32_to_cpu(fw_ap->start_tsf);
			result.ap_tsf_valid = 1;
		} else {
			/* the first part is the same for old and new APIs */
			fw_ap = (void *)&fw_resp_v5->ap[i];
			/*
			 * FIXME: the firmware needs to report this, we don't
			 * even know the number of bursts the responder picked
			 * (if we asked it to)
			 */
			result.final = 0;
		}

		peer_idx = iwl_mvm_ftm_find_peer(mvm->ftm_initiator.req,
						 fw_ap->bssid);
		if (peer_idx < 0) {
			IWL_WARN(mvm,
				 "Unknown address (%pM, target #%d) in FTM response\n",
				 fw_ap->bssid, i);
			continue;
		}

		switch (fw_ap->measure_status) {
		case IWL_TOF_ENTRY_SUCCESS:
			result.status = NL80211_PMSR_STATUS_SUCCESS;
			break;
		case IWL_TOF_ENTRY_TIMING_MEASURE_TIMEOUT:
			result.status = NL80211_PMSR_STATUS_TIMEOUT;
			break;
		case IWL_TOF_ENTRY_NO_RESPONSE:
			result.status = NL80211_PMSR_STATUS_FAILURE;
			result.ftm.failure_reason =
				NL80211_PMSR_FTM_FAILURE_NO_RESPONSE;
			break;
		case IWL_TOF_ENTRY_REQUEST_REJECTED:
			result.status = NL80211_PMSR_STATUS_FAILURE;
			result.ftm.failure_reason =
				NL80211_PMSR_FTM_FAILURE_PEER_BUSY;
			result.ftm.busy_retry_time = fw_ap->refusal_period;
			break;
		default:
			result.status = NL80211_PMSR_STATUS_FAILURE;
			result.ftm.failure_reason =
				NL80211_PMSR_FTM_FAILURE_UNSPECIFIED;
			break;
		}
		memcpy(result.addr, fw_ap->bssid, ETH_ALEN);
		result.host_time = iwl_mvm_ftm_get_host_time(mvm,
							     fw_ap->timestamp);
		result.type = NL80211_PMSR_TYPE_FTM;
		result.ftm.burst_index = mvm->ftm_initiator.responses[peer_idx];
		mvm->ftm_initiator.responses[peer_idx]++;
		result.ftm.rssi_avg = fw_ap->rssi;
		result.ftm.rssi_avg_valid = 1;
		result.ftm.rssi_spread = fw_ap->rssi_spread;
		result.ftm.rssi_spread_valid = 1;
		result.ftm.rtt_avg = (s32)le32_to_cpu(fw_ap->rtt);
		result.ftm.rtt_avg_valid = 1;
		result.ftm.rtt_variance = le32_to_cpu(fw_ap->rtt_variance);
		result.ftm.rtt_variance_valid = 1;
		result.ftm.rtt_spread = le32_to_cpu(fw_ap->rtt_spread);
		result.ftm.rtt_spread_valid = 1;

		iwl_mvm_ftm_get_lci_civic(mvm, &result);

		iwl_mvm_ftm_rtt_smoothing(mvm, &result);

		cfg80211_pmsr_report(mvm->ftm_initiator.req_wdev,
				     mvm->ftm_initiator.req,
				     &result, GFP_KERNEL);

		if (fw_has_api(&mvm->fw->ucode_capa,
			       IWL_UCODE_TLV_API_FTM_RTT_ACCURACY))
			IWL_DEBUG_INFO(mvm, "RTT confidence: %hhu\n",
				       fw_ap->rttConfidence);

		iwl_mvm_debug_range_resp(mvm, i, &result);
	}

	if (last_in_batch) {
		cfg80211_pmsr_complete(mvm->ftm_initiator.req_wdev,
				       mvm->ftm_initiator.req,
				       GFP_KERNEL);
		iwl_mvm_ftm_reset(mvm);
	}
}

void iwl_mvm_ftm_lc_notif(struct iwl_mvm *mvm, struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	const struct ieee80211_mgmt *mgmt = (void *)pkt->data;
	size_t len = iwl_rx_packet_payload_len(pkt);
	struct iwl_mvm_loc_entry *entry;
	const u8 *ies, *lci, *civic, *msr_ie;
	size_t ies_len, lci_len = 0, civic_len = 0;
	size_t baselen = IEEE80211_MIN_ACTION_SIZE +
			 sizeof(mgmt->u.action.u.ftm);
	static const u8 rprt_type_lci = IEEE80211_SPCT_MSR_RPRT_TYPE_LCI;
	static const u8 rprt_type_civic = IEEE80211_SPCT_MSR_RPRT_TYPE_CIVIC;

	if (len <= baselen)
		return;

	lockdep_assert_held(&mvm->mutex);

	ies = mgmt->u.action.u.ftm.variable;
	ies_len = len - baselen;

	msr_ie = cfg80211_find_ie_match(WLAN_EID_MEASURE_REPORT, ies, ies_len,
					&rprt_type_lci, 1, 4);
	if (msr_ie) {
		lci = msr_ie + 2;
		lci_len = msr_ie[1];
	}

	msr_ie = cfg80211_find_ie_match(WLAN_EID_MEASURE_REPORT, ies, ies_len,
					&rprt_type_civic, 1, 4);
	if (msr_ie) {
		civic = msr_ie + 2;
		civic_len = msr_ie[1];
	}

	entry = kmalloc(sizeof(*entry) + lci_len + civic_len, GFP_KERNEL);
	if (!entry)
		return;

	memcpy(entry->addr, mgmt->bssid, ETH_ALEN);

	entry->lci_len = lci_len;
	if (lci_len)
		memcpy(entry->buf, lci, lci_len);

	entry->civic_len = civic_len;
	if (civic_len)
		memcpy(entry->buf + lci_len, civic, civic_len);

	list_add_tail(&entry->list, &mvm->ftm_initiator.loc_list);
}