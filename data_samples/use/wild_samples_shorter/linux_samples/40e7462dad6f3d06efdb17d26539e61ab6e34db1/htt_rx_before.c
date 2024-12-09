	rx_desc = (struct htt_hl_rx_desc *)(skb->data + tot_hdr_len);
	rx_desc_info = __le32_to_cpu(rx_desc->info);

	if (!MS(rx_desc_info, HTT_RX_DESC_HL_INFO_ENCRYPTED)) {
		spin_unlock_bh(&ar->data_lock);
		return ath10k_htt_rx_proc_rx_ind_hl(htt, &resp->rx_ind_hl, skb,
						    HTT_RX_NON_PN_CHECK,
						    HTT_RX_NON_TKIP_MIC);
	}

	hdr = (struct ieee80211_hdr *)((u8 *)rx_desc + rx_hl->fw_desc.len);

	if (ieee80211_has_retry(hdr->frame_control))
		goto err;

	hdr_space = ieee80211_hdrlen(hdr->frame_control);