	if (!peer) {
		ath10k_dbg(ar, ATH10K_DBG_HTT, "invalid peer: %u\n", peer_id);
		goto err;
	}

	num_mpdu_ranges = MS(__le32_to_cpu(rx_hl->hdr.info1),
			     HTT_RX_INDICATION_INFO1_NUM_MPDU_RANGES);

	tot_hdr_len = sizeof(struct htt_resp_hdr) +
		      sizeof(rx_hl->hdr) +
		      sizeof(rx_hl->ppdu) +
		      sizeof(rx_hl->prefix) +
		      sizeof(rx_hl->fw_desc) +
		      sizeof(struct htt_rx_indication_mpdu_range) * num_mpdu_ranges;

	tid =  MS(rx_hl->hdr.info0, HTT_RX_INDICATION_INFO0_EXT_TID);
	rx_desc = (struct htt_hl_rx_desc *)(skb->data + tot_hdr_len);
	rx_desc_info = __le32_to_cpu(rx_desc->info);

	hdr = (struct ieee80211_hdr *)((u8 *)rx_desc + rx_hl->fw_desc.len);

	if (is_multicast_ether_addr(hdr->addr1)) {
		/* Discard the fragment with multicast DA */
		goto err;
	}