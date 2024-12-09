		}

		ath10k_htt_rx_h_csum_offload(msdu);
		ath10k_htt_rx_h_undecap(ar, msdu, status, first_hdr, enctype,
					is_decrypted);

		/* Undecapping involves copying the original 802.11 header back

		hdr = (void *)msdu->data;
		hdr->frame_control &= ~__cpu_to_le16(IEEE80211_FCTL_PROTECTED);
	}
}

static void ath10k_htt_rx_h_enqueue(struct ath10k *ar,