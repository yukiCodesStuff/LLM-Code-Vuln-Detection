		}

		ath10k_htt_rx_h_csum_offload(msdu);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_MMIC_STRIPPED;

		ath10k_htt_rx_h_undecap(ar, msdu, status, first_hdr, enctype,
					is_decrypted);

		/* Undecapping involves copying the original 802.11 header back

		hdr = (void *)msdu->data;
		hdr->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);

		if (frag && !fill_crypt_header &&
		    enctype == HTT_RX_MPDU_ENCRYPT_TKIP_WPA)
			status->flag &= ~RX_FLAG_IV_STRIPPED &
					~RX_FLAG_MMIC_STRIPPED;
	}
}

static void ath10k_htt_rx_h_enqueue(struct ath10k *ar,