static void iwl_mvm_pass_packet_to_mac80211(struct iwl_mvm *mvm,
					    struct napi_struct *napi,
					    struct sk_buff *skb, int queue,
					    struct ieee80211_sta *sta,
					    bool csi)
{
	if (iwl_mvm_check_pn(mvm, skb, queue, sta))
		kfree_skb(skb);
	else
		(rate_flags & RATE_MCS_ANT_AB_MSK) >> RATE_MCS_ANT_POS;
	rx_status->chain_signal[0] = energy_a;
	rx_status->chain_signal[1] = energy_b;
	rx_status->chain_signal[2] = S8_MIN;
}

static int iwl_mvm_rx_mgmt_prot(struct ieee80211_sta *sta,
				struct ieee80211_hdr *hdr,
		while ((skb = __skb_dequeue(skb_list))) {
			iwl_mvm_pass_packet_to_mac80211(mvm, napi, skb,
							reorder_buf->queue,
							sta, false);
			reorder_buf->num_stored--;
		}
	}
	reorder_buf->head_sn = nssn;
	}

	if (FIELD_GET(IWL_RX_PHY_DATA4_HE_MU_EXT_CH2_CRC_OK, phy_data4) &&
	    (rate_n_flags & RATE_MCS_CHAN_WIDTH_MSK) != RATE_MCS_CHAN_WIDTH_20) {
		he_mu->flags1 |=
			cpu_to_le16(IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH2_RU_KNOWN |
				    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH2_CTR_26T_RU_KNOWN);

	 * the TSF/timers are not be transmitted in HE-MU.
	 */
	u8 ru = le32_get_bits(phy_data->d1, IWL_RX_PHY_DATA1_HE_RU_ALLOC_MASK);
	u32 he_type = rate_n_flags & RATE_MCS_HE_TYPE_MSK;
	u8 offs = 0;

	rx_status->bw = RATE_INFO_BW_HE_RU;


	if (he_mu)
		he_mu->flags2 |=
			le16_encode_bits(FIELD_GET(RATE_MCS_CHAN_WIDTH_MSK,
						   rate_n_flags),
					 IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW);
	else if (he_type == RATE_MCS_HE_TYPE_TRIG)
		he->data6 |=
			cpu_to_le16(IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_KNOWN) |
			le16_encode_bits(FIELD_GET(RATE_MCS_CHAN_WIDTH_MSK,
						   rate_n_flags),
					 IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW);
}


	stbc = (rate_n_flags & RATE_MCS_STBC_MSK) >> RATE_MCS_STBC_POS;
	rx_status->nss =
		((rate_n_flags & RATE_VHT_MCS_NSS_MSK) >>
					RATE_VHT_MCS_NSS_POS) + 1;
	rx_status->rate_idx = rate_n_flags & RATE_VHT_MCS_RATE_CODE_MSK;
	rx_status->encoding = RX_ENC_HE;
	rx_status->enc_flags |= stbc << RX_ENC_FLAG_STBC_SHIFT;
	if (rate_n_flags & RATE_MCS_BF_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_BF;
		}
		break;
	case 3:
		if ((he_type == RATE_MCS_HE_TYPE_SU ||
		     he_type == RATE_MCS_HE_TYPE_EXT_SU) &&
		    rate_n_flags & RATE_MCS_SGI_MSK)
			rx_status->he_gi = NL80211_RATE_INFO_HE_GI_0_8;
		else
			rx_status->he_gi = NL80211_RATE_INFO_HE_GI_3_2;
		ltf = IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE_4X;
		break;
	}

	he->data5 |= le16_encode_bits(ltf,
				      IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE);
	struct iwl_mvm_rx_phy_data phy_data = {
		.info_type = IWL_RX_PHY_INFO_TYPE_NONE,
	};
	bool csi = false;

	if (unlikely(test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status)))
		return;

		phy_data.d2 = desc->v1.phy_data2;
		phy_data.d3 = desc->v1.phy_data3;
	}

	len = le16_to_cpu(desc->mpdu_len);

	if (unlikely(len + desc_size > pkt_len)) {
		break;
	}

	if (rate_n_flags & RATE_MCS_HE_MSK)
		iwl_mvm_rx_he(mvm, skb, &phy_data, rate_n_flags,
			      phy_info, queue);

	iwl_mvm_decode_lsig(skb, &phy_data);
		rx_status->flag |= RX_FLAG_FAILED_FCS_CRC;
	}
	/* set the preamble flag if appropriate */
	if (rate_n_flags & RATE_MCS_CCK_MSK &&
	    phy_info & IWL_RX_MPDU_PHY_SHORT_PREAMBLE)
		rx_status->enc_flags |= RX_ENC_FLAG_SHORTPRE;

	if (likely(!(phy_info & IWL_RX_MPDU_PHY_TSF_OVERLOAD))) {
		}
	}

	if (!(rate_n_flags & RATE_MCS_CCK_MSK) &&
	    rate_n_flags & RATE_MCS_SGI_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_SHORT_GI;
	if (rate_n_flags & RATE_HT_MCS_GF_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_HT_GF;
	if (rate_n_flags & RATE_MCS_LDPC_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_LDPC;
	if (rate_n_flags & RATE_MCS_HT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->encoding = RX_ENC_HT;
		rx_status->rate_idx = rate_n_flags & RATE_HT_MCS_INDEX_MSK;
		rx_status->enc_flags |= stbc << RX_ENC_FLAG_STBC_SHIFT;
	} else if (rate_n_flags & RATE_MCS_VHT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->nss =
			((rate_n_flags & RATE_VHT_MCS_NSS_MSK) >>
						RATE_VHT_MCS_NSS_POS) + 1;
		rx_status->rate_idx = rate_n_flags & RATE_VHT_MCS_RATE_CODE_MSK;
		rx_status->encoding = RX_ENC_VHT;
		rx_status->enc_flags |= stbc << RX_ENC_FLAG_STBC_SHIFT;
		if (rate_n_flags & RATE_MCS_BF_MSK)
			rx_status->enc_flags |= RX_ENC_FLAG_BF;
	} else if (!(rate_n_flags & RATE_MCS_HE_MSK)) {
		int rate = iwl_mvm_legacy_rate_to_mac80211_idx(rate_n_flags,
							       rx_status->band);

		if (WARN(rate < 0 || rate > 0xFF,
			 "Invalid rate flags 0x%x, band %d,\n",
			 rate_n_flags, rx_status->band)) {

	if (!iwl_mvm_reorder(mvm, napi, queue, sta, skb, desc))
		iwl_mvm_pass_packet_to_mac80211(mvm, napi, skb, queue,
						sta, csi);
out:
	rcu_read_unlock();
}

	struct ieee80211_sta *sta = NULL;
	struct sk_buff *skb;
	u8 channel, energy_a, energy_b;
	struct iwl_mvm_rx_phy_data phy_data = {
		.info_type = le32_get_bits(desc->phy_info[1],
					   IWL_RX_PHY_DATA1_INFO_TYPE_MASK),
		.d0 = desc->phy_info[0],
		.d1 = desc->phy_info[1],
	};

	if (unlikely(iwl_rx_packet_payload_len(pkt) < sizeof(*desc)))
		return;

		break;
	}

	if (rate_n_flags & RATE_MCS_HE_MSK)
		iwl_mvm_rx_he(mvm, skb, &phy_data, rate_n_flags,
			      phy_info, queue);

	iwl_mvm_decode_lsig(skb, &phy_data);

	rcu_read_lock();

	if (!(rate_n_flags & RATE_MCS_CCK_MSK) &&
	    rate_n_flags & RATE_MCS_SGI_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_SHORT_GI;
	if (rate_n_flags & RATE_HT_MCS_GF_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_HT_GF;
	if (rate_n_flags & RATE_MCS_LDPC_MSK)
		rx_status->enc_flags |= RX_ENC_FLAG_LDPC;
	if (rate_n_flags & RATE_MCS_HT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->encoding = RX_ENC_HT;
		rx_status->rate_idx = rate_n_flags & RATE_HT_MCS_INDEX_MSK;
		rx_status->enc_flags |= stbc << RX_ENC_FLAG_STBC_SHIFT;
	} else if (rate_n_flags & RATE_MCS_VHT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->rate_idx = rate_n_flags & RATE_VHT_MCS_RATE_CODE_MSK;
		rx_status->encoding = RX_ENC_VHT;
		rx_status->enc_flags |= stbc << RX_ENC_FLAG_STBC_SHIFT;
		if (rate_n_flags & RATE_MCS_BF_MSK)
			rx_status->enc_flags |= RX_ENC_FLAG_BF;
		rx_status->nss =
			le32_get_bits(desc->rx_vec[0],
				      RX_NO_DATA_RX_VEC0_VHT_NSTS_MSK) + 1;
	} else if (rate_n_flags & RATE_MCS_HE_MSK) {
		rx_status->nss =
			le32_get_bits(desc->rx_vec[0],
				      RX_NO_DATA_RX_VEC0_HE_NSTS_MSK) + 1;
	} else {
		int rate = iwl_mvm_legacy_rate_to_mac80211_idx(rate_n_flags,
							       rx_status->band);

		if (WARN(rate < 0 || rate > 0xFF,
			 "Invalid rate flags 0x%x, band %d,\n",