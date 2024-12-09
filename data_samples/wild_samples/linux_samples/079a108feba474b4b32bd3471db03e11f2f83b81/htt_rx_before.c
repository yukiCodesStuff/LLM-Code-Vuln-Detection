{
	struct ath10k *ar = htt->ar;
	struct ath10k_peer *peer;
	struct htt_rx_indication_mpdu_range *mpdu_ranges;
	struct fw_rx_desc_hl *fw_desc;
	enum htt_txrx_sec_cast_type sec_index;
	enum htt_security_types sec_type;
	union htt_rx_pn_t new_pn = {0};
	struct htt_hl_rx_desc *rx_desc;
	struct ieee80211_hdr *hdr;
	struct ieee80211_rx_status *rx_status;
	u16 peer_id;
	u8 rx_desc_len;
	int num_mpdu_ranges;
	size_t tot_hdr_len;
	struct ieee80211_channel *ch;
	bool pn_invalid, qos, first_msdu;
	u32 tid, rx_desc_info;

	peer_id = __le16_to_cpu(rx->hdr.peer_id);
	tid = MS(rx->hdr.info0, HTT_RX_INDICATION_INFO0_EXT_TID);

	spin_lock_bh(&ar->data_lock);
	peer = ath10k_peer_find_by_id(ar, peer_id);
	spin_unlock_bh(&ar->data_lock);
	if (!peer && peer_id != HTT_INVALID_PEERID)
		ath10k_warn(ar, "Got RX ind from invalid peer: %u\n", peer_id);

	if (!peer)
		return true;

	num_mpdu_ranges = MS(__le32_to_cpu(rx->hdr.info1),
			     HTT_RX_INDICATION_INFO1_NUM_MPDU_RANGES);
	mpdu_ranges = htt_rx_ind_get_mpdu_ranges_hl(rx);
	fw_desc = &rx->fw_desc;
	rx_desc_len = fw_desc->len;

	/* I have not yet seen any case where num_mpdu_ranges > 1.
	 * qcacld does not seem handle that case either, so we introduce the
	 * same limitiation here as well.
	 */
	if (num_mpdu_ranges > 1)
		ath10k_warn(ar,
			    "Unsupported number of MPDU ranges: %d, ignoring all but the first\n",
			    num_mpdu_ranges);

	if (mpdu_ranges->mpdu_range_status !=
	    HTT_RX_IND_MPDU_STATUS_OK &&
	    mpdu_ranges->mpdu_range_status !=
	    HTT_RX_IND_MPDU_STATUS_TKIP_MIC_ERR) {
		ath10k_dbg(ar, ATH10K_DBG_HTT, "htt mpdu_range_status %d\n",
			   mpdu_ranges->mpdu_range_status);
		goto err;
	}