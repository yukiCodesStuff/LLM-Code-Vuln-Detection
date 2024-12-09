	switch (info->control.vif->type) {
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_ADHOC:
		/*
		 * Handle legacy hostapd as well, where station will be added
		 * only just before sending the association response.
		 * Also take care of the case where we send a deauth to a
		 * station that we don't have, or similarly an association
		 * response (with non-success status) for a station we can't
		 * accept.
		 * Also, disassociate frames might happen, particular with
		 * reason 7 ("Class 3 frame received from nonassociated STA").
		 */
		if (ieee80211_is_probe_resp(fc) || ieee80211_is_auth(fc) ||
		    ieee80211_is_deauth(fc) || ieee80211_is_assoc_resp(fc) ||
		    ieee80211_is_disassoc(fc))
			return mvm->probe_queue;
		if (info->hw_queue == info->control.vif->cab_queue)
			return mvmvif->cab_queue;

		WARN_ONCE(info->control.vif->type != NL80211_IFTYPE_ADHOC,
			  "fc=0x%02x", le16_to_cpu(fc));
		return mvm->probe_queue;
	case NL80211_IFTYPE_P2P_DEVICE:
		if (ieee80211_is_mgmt(fc))
			return mvm->p2p_dev_queue;
		if (info->hw_queue == info->control.vif->cab_queue)
			return mvmvif->cab_queue;

		WARN_ON_ONCE(1);
		return mvm->p2p_dev_queue;
	default:
		WARN_ONCE(1, "Not a ctrl vif, no available queue\n");
		return -1;
	}
}

int iwl_mvm_tx_skb_non_sta(struct iwl_mvm *mvm, struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *skb_info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_info info;
	struct iwl_device_cmd *dev_cmd;
	u8 sta_id;
	int hdrlen = ieee80211_hdrlen(hdr->frame_control);
	int queue;

	/* IWL_MVM_OFFCHANNEL_QUEUE is used for ROC packets that can be used
	 * in 2 different types of vifs, P2P & STATION. P2P uses the offchannel
	 * queue. STATION (HS2.0) uses the auxiliary context of the FW,
	 * and hence needs to be sent on the aux queue
	 */
	if (skb_info->hw_queue == IWL_MVM_OFFCHANNEL_QUEUE &&
	    skb_info->control.vif->type == NL80211_IFTYPE_STATION)
		skb_info->hw_queue = mvm->aux_queue;

	memcpy(&info, skb->cb, sizeof(info));

	if (WARN_ON_ONCE(info.flags & IEEE80211_TX_CTL_AMPDU))
		return -1;

	if (WARN_ON_ONCE(info.flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM &&
			 (!info.control.vif ||
			  info.hw_queue != info.control.vif->cab_queue)))
		return -1;

	queue = info.hw_queue;

	/*
	 * If the interface on which the frame is sent is the P2P_DEVICE
	 * or an AP/GO interface use the broadcast station associated
	 * with it; otherwise if the interface is a managed interface
	 * use the AP station associated with it for multicast traffic
	 * (this is not possible for unicast packets as a TLDS discovery
	 * response are sent without a station entry); otherwise use the
	 * AUX station.
	 */
	sta_id = mvm->aux_sta.sta_id;
	if (info.control.vif) {
		struct iwl_mvm_vif *mvmvif =
			iwl_mvm_vif_from_mac80211(info.control.vif);

		if (info.control.vif->type == NL80211_IFTYPE_P2P_DEVICE ||
		    info.control.vif->type == NL80211_IFTYPE_AP ||
		    info.control.vif->type == NL80211_IFTYPE_ADHOC) {
			sta_id = mvmvif->bcast_sta.sta_id;
			queue = iwl_mvm_get_ctrl_vif_queue(mvm, &info,
							   hdr->frame_control);
			if (queue < 0)
				return -1;
		} else if (info.control.vif->type == NL80211_IFTYPE_STATION &&
			   is_multicast_ether_addr(hdr->addr1)) {
			u8 ap_sta_id = ACCESS_ONCE(mvmvif->ap_sta_id);

			if (ap_sta_id != IWL_MVM_INVALID_STA)
				sta_id = ap_sta_id;
		} else if (info.control.vif->type == NL80211_IFTYPE_MONITOR) {
			queue = mvm->aux_queue;
		}
	}

	IWL_DEBUG_TX(mvm, "station Id %d, queue=%d\n", sta_id, queue);

	dev_cmd = iwl_mvm_set_tx_params(mvm, skb, &info, hdrlen, NULL, sta_id);
	if (!dev_cmd)
		return -1;

	/* From now on, we cannot access info->control */
	iwl_mvm_skb_prepare_status(skb, dev_cmd);

	if (iwl_trans_tx(mvm->trans, skb, dev_cmd, queue)) {
		iwl_trans_free_tx_cmd(mvm->trans, dev_cmd);
		return -1;
	}

	return 0;
}