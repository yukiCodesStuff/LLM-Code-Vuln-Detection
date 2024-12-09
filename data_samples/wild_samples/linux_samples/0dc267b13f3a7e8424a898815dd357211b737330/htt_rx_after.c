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

		ath10k_htt_rx_h_csum_offload(msdu);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_MMIC_STRIPPED;

		ath10k_htt_rx_h_undecap(ar, msdu, status, first_hdr, enctype,
					is_decrypted);

		/* Undecapping involves copying the original 802.11 header back
		 * to sk_buff. If frame is protected and hardware has decrypted
		 * it then remove the protected bit.
		 */
		if (!is_decrypted)
			continue;
		if (is_mgmt)
			continue;

		if (fill_crypt_header)
			continue;

		hdr = (void *)msdu->data;
		hdr->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_IV_STRIPPED &
					~RX_FLAG_MMIC_STRIPPED;
	}
}

static void ath10k_htt_rx_h_enqueue(struct ath10k *ar,
				    struct sk_buff_head *amsdu,
				    struct ieee80211_rx_status *status)
{
	struct sk_buff *msdu;
	struct sk_buff *first_subframe;

	first_subframe = skb_peek(amsdu);

	while ((msdu = __skb_dequeue(amsdu))) {
		/* Setup per-MSDU flags */
		if (skb_queue_empty(amsdu))
			status->flag &= ~RX_FLAG_AMSDU_MORE;
		else
			status->flag |= RX_FLAG_AMSDU_MORE;

		if (msdu == first_subframe) {
			first_subframe = NULL;
			status->flag &= ~RX_FLAG_ALLOW_SAME_PN;
		} else {
			status->flag |= RX_FLAG_ALLOW_SAME_PN;
		}

		ath10k_htt_rx_h_queue_msdu(ar, status, msdu);
	}
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

		ath10k_htt_rx_h_csum_offload(msdu);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_MMIC_STRIPPED;

		ath10k_htt_rx_h_undecap(ar, msdu, status, first_hdr, enctype,
					is_decrypted);

		/* Undecapping involves copying the original 802.11 header back
		 * to sk_buff. If frame is protected and hardware has decrypted
		 * it then remove the protected bit.
		 */
		if (!is_decrypted)
			continue;
		if (is_mgmt)
			continue;

		if (fill_crypt_header)
			continue;

		hdr = (void *)msdu->data;
		hdr->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_IV_STRIPPED &
					~RX_FLAG_MMIC_STRIPPED;
	}
}

static void ath10k_htt_rx_h_enqueue(struct ath10k *ar,
				    struct sk_buff_head *amsdu,
				    struct ieee80211_rx_status *status)
{
	struct sk_buff *msdu;
	struct sk_buff *first_subframe;

	first_subframe = skb_peek(amsdu);

	while ((msdu = __skb_dequeue(amsdu))) {
		/* Setup per-MSDU flags */
		if (skb_queue_empty(amsdu))
			status->flag &= ~RX_FLAG_AMSDU_MORE;
		else
			status->flag |= RX_FLAG_AMSDU_MORE;

		if (msdu == first_subframe) {
			first_subframe = NULL;
			status->flag &= ~RX_FLAG_ALLOW_SAME_PN;
		} else {
			status->flag |= RX_FLAG_ALLOW_SAME_PN;
		}

		ath10k_htt_rx_h_queue_msdu(ar, status, msdu);
	}