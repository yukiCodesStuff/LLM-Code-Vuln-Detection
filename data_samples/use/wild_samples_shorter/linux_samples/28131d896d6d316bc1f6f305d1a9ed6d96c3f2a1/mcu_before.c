	return mt76_tx_queue_skb_raw(dev, mdev->q_mcu[qid], skb, 0);
}

static int
mt7915_mcu_wa_cmd(struct mt7915_dev *dev, int cmd, u32 a1, u32 a2, u32 a3)
{
	struct {
		__le32 args[3];
	} req = {
		},
	};

	return mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), true);
}

static void
mt7915_mcu_csa_finish(void *priv, u8 *mac, struct ieee80211_vif *vif)
	bfee->fb_identity_matrix = (nrow == 1 && tx_ant == 2);
}

static void
mt7915_mcu_sta_rate_ctrl_tlv(struct sk_buff *skb, struct mt7915_dev *dev,
			     struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	}

	if (sta->ht_cap.ht_supported) {
		const u8 *mcs_mask = mask->control[band].ht_mcs;

		ra->supp_mode |= MODE_HT;
		ra->af = sta->ht_cap.ampdu_factor;
		ra->ht_gf = !!(sta->ht_cap.cap & IEEE80211_HT_CAP_GRN_FLD);

		    (sta->ht_cap.cap & IEEE80211_HT_CAP_LDPC_CODING))
			cap |= STA_CAP_LDPC;

		mt7915_mcu_set_sta_ht_mcs(sta, ra->ht_mcs, mcs_mask);
		ra->supp_ht_mcs = *(__le32 *)ra->ht_mcs;
	}

	if (sta->vht_cap.vht_supported) {
		const u16 *mcs_mask = mask->control[band].vht_mcs;
		u8 af;

		ra->supp_mode |= MODE_VHT;
		af = FIELD_GET(IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK,
		    (sta->vht_cap.cap & IEEE80211_VHT_CAP_RXLDPC))
			cap |= STA_CAP_VHT_LDPC;

		mt7915_mcu_set_sta_vht_mcs(sta, ra->supp_vht_mcs, mcs_mask);
	}

	if (sta->he_cap.has_he) {
		ra->supp_mode |= MODE_HE;
}

int mt7915_mcu_add_rate_ctrl(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct sk_buff *skb;
	int len = sizeof(struct sta_req_hdr) + sizeof(struct sta_rec_ra);

	skb = mt7915_mcu_alloc_sta_req(dev, mvif, msta, len);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	mt7915_mcu_sta_rate_ctrl_tlv(skb, dev, vif, sta);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(STA_REC_UPDATE), true);
}

int mt7915_mcu_add_he(struct mt7915_dev *dev, struct ieee80211_vif *vif,
		      struct ieee80211_sta *sta)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct sk_buff *skb;
	int len;

	if (!sta->he_cap.has_he)
		return 0;

	len = sizeof(struct sta_req_hdr) + sizeof(struct sta_rec_he);

	skb = mt7915_mcu_alloc_sta_req(dev, mvif, msta, len);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	mt7915_mcu_sta_he_tlv(skb, sta, vif);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(STA_REC_UPDATE), true);
}

static int
mt7915_mcu_add_group(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			      len);
	if (ie && ie[1] >= sizeof(*ht)) {
		ht = (void *)(ie + 2);
		bc = le32_to_cpu(ht->cap_info);

		vc->ldpc |= !!(bc & IEEE80211_HT_CAP_LDPC_CODING);
	}

	ie = cfg80211_find_ie(WLAN_EID_VHT_CAPABILITY, mgmt->u.beacon.variable,
			      len);
	return 0;
}

int mt7915_mcu_fw_log_2_host(struct mt7915_dev *dev, u8 ctrl)
{
	struct {
		u8 ctrl_val;
		u8 pad[3];
		.ctrl_val = ctrl
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(FW_LOG_2_HOST), &data,
				 sizeof(data), true);
}

		return ret;

	set_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state);
	ret = mt7915_mcu_fw_log_2_host(dev, 0);
	if (ret)
		return ret;

	ret = mt7915_mcu_set_mwds(dev, 1);