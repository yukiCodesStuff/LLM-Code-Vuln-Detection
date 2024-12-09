	if (enctype == HTT_RX_MPDU_ENCRYPT_AES_CCM_WPA2) {
		pn = ehdr[0];
		pn |= (u64)ehdr[1] << 8;
		pn |= (u64)ehdr[4] << 16;
		pn |= (u64)ehdr[5] << 24;
		pn |= (u64)ehdr[6] << 32;
		pn |= (u64)ehdr[7] << 40;
	}
	return pn;
}

static bool ath10k_htt_rx_h_frag_multicast_check(struct ath10k *ar,
						 struct sk_buff *skb,
						 u16 offset)
{
	struct ieee80211_hdr *hdr;

	hdr = (struct ieee80211_hdr *)(skb->data + offset);
	return !is_multicast_ether_addr(hdr->addr1);
}

static bool ath10k_htt_rx_h_frag_pn_check(struct ath10k *ar,
					  struct sk_buff *skb,
					  u16 peer_id,
					  u16 offset,
					  enum htt_rx_mpdu_encrypt_type enctype)
{
	struct ath10k_peer *peer;
	union htt_rx_pn_t *last_pn, new_pn = {0};
	struct ieee80211_hdr *hdr;
	bool more_frags;
	u8 tid, frag_number;
	u32 seq;

	peer = ath10k_peer_find_by_id(ar, peer_id);
	if (!peer) {
		ath10k_dbg(ar, ATH10K_DBG_HTT, "invalid peer for frag pn check\n");
		return false;
	}
{
	struct sk_buff *first;
	struct sk_buff *last;
	struct sk_buff *msdu, *temp;
	struct htt_rx_desc *rxd;
	struct ieee80211_hdr *hdr;
	enum htt_rx_mpdu_encrypt_type enctype;
	u8 first_hdr[64];
	u8 *qos;
	bool has_fcs_err;
	bool has_crypto_err;
	bool has_tkip_err;
	bool has_peer_idx_invalid;
	bool is_decrypted;
	bool is_mgmt;
	u32 attention;
	bool frag_pn_check = true, multicast_check = true;

	if (skb_queue_empty(amsdu))
		return;

	first = skb_peek(amsdu);
	rxd = (void *)first->data - sizeof(*rxd);

	is_mgmt = !!(rxd->attention.flags &
		     __cpu_to_le32(RX_ATTENTION_FLAGS_MGMT_TYPE));

	enctype = MS(__le32_to_cpu(rxd->mpdu_start.info0),
		     RX_MPDU_START_INFO0_ENCRYPT_TYPE);

	/* First MSDU's Rx descriptor in an A-MSDU contains full 802.11
	 * decapped header. It'll be used for undecapping of each MSDU.
	 */
	hdr = (void *)rxd->rx_hdr_status;
	memcpy(first_hdr, hdr, RX_HTT_HDR_STATUS_LEN);

	if (rx_hdr)
		memcpy(rx_hdr, hdr, RX_HTT_HDR_STATUS_LEN);

	/* Each A-MSDU subframe will use the original header as the base and be
	 * reported as a separate MSDU so strip the A-MSDU bit from QoS Ctl.
	 */
	hdr = (void *)first_hdr;

	if (ieee80211_is_data_qos(hdr->frame_control)) {
		qos = ieee80211_get_qos_ctl(hdr);
		qos[0] &= ~IEEE80211_QOS_CTL_A_MSDU_PRESENT;
	}
	skb_queue_walk(amsdu, msdu) {
		if (frag && !fill_crypt_header && is_decrypted &&
		    enctype == HTT_RX_MPDU_ENCRYPT_AES_CCM_WPA2)
			frag_pn_check = ath10k_htt_rx_h_frag_pn_check(ar,
								      msdu,
								      peer_id,
								      0,
								      enctype);

		if (frag)
			multicast_check = ath10k_htt_rx_h_frag_multicast_check(ar,
									       msdu,
									       0);

		if (!frag_pn_check || !multicast_check) {
			/* Discard the fragment with invalid PN or multicast DA
			 */
			temp = msdu->prev;
			__skb_unlink(msdu, amsdu);
			dev_kfree_skb_any(msdu);
			msdu = temp;
			frag_pn_check = true;
			multicast_check = true;
			continue;
		}