	return mt76_tx_queue_skb_raw(dev, mdev->q_mcu[qid], skb, 0);
}

int mt7915_mcu_wa_cmd(struct mt7915_dev *dev, int cmd, u32 a1, u32 a2, u32 a3)
{
	struct {
		__le32 args[3];
	} req = {
		},
	};

	return mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), false);
}

static void
mt7915_mcu_csa_finish(void *priv, u8 *mac, struct ieee80211_vif *vif)
	bfee->fb_identity_matrix = (nrow == 1 && tx_ant == 2);
}

int mt7915_mcu_set_fixed_rate_ctrl(struct mt7915_dev *dev,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   void *data, u32 field)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct sta_phy *phy = data;
	struct sta_rec_ra_fixed *ra;
	struct sk_buff *skb;
	struct tlv *tlv;
	int len = sizeof(struct sta_req_hdr) + sizeof(*ra);

	skb = mt7915_mcu_alloc_sta_req(dev, mvif, msta, len);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	tlv = mt7915_mcu_add_tlv(skb, STA_REC_RA_UPDATE, sizeof(*ra));
	ra = (struct sta_rec_ra_fixed *)tlv;

	switch (field) {
	case RATE_PARAM_AUTO:
		break;
	case RATE_PARAM_FIXED:
	case RATE_PARAM_FIXED_MCS:
	case RATE_PARAM_FIXED_GI:
	case RATE_PARAM_FIXED_HE_LTF:
		ra->phy = *phy;
		break;
	default:
		break;
	}
	ra->field = cpu_to_le32(field);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(STA_REC_UPDATE), true);
}

static int
mt7915_mcu_add_rate_ctrl_fixed(struct mt7915_dev *dev,
			       struct ieee80211_vif *vif,
			       struct ieee80211_sta *sta)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct cfg80211_chan_def *chandef = &mvif->phy->mt76->chandef;
	struct cfg80211_bitrate_mask *mask = &mvif->bitrate_mask;
	enum nl80211_band band = chandef->chan->band;
	struct sta_phy phy = {};
	int ret, nrates = 0;

#define __sta_phy_bitrate_mask_check(_mcs, _gi, _he)				\
	do {									\
		u8 i, gi = mask->control[band]._gi;				\
		gi = (_he) ? gi : gi == NL80211_TXRATE_FORCE_SGI;		\
		for (i = 0; i <= sta->bandwidth; i++) {				\
			phy.sgi |= gi << (i << (_he));				\
			phy.he_ltf |= mask->control[band].he_ltf << (i << (_he));\
		}								\
		for (i = 0; i < ARRAY_SIZE(mask->control[band]._mcs); i++) 	\
			nrates += hweight16(mask->control[band]._mcs[i]);  	\
		phy.mcs = ffs(mask->control[band]._mcs[0]) - 1;			\
	} while (0)

	if (sta->he_cap.has_he) {
		__sta_phy_bitrate_mask_check(he_mcs, he_gi, 1);
	} else if (sta->vht_cap.vht_supported) {
		__sta_phy_bitrate_mask_check(vht_mcs, gi, 0);
	} else if (sta->ht_cap.ht_supported) {
		__sta_phy_bitrate_mask_check(ht_mcs, gi, 0);
	} else {
		nrates = hweight32(mask->control[band].legacy);
		phy.mcs = ffs(mask->control[band].legacy) - 1;
	}
#undef __sta_phy_bitrate_mask_check

	/* fall back to auto rate control */
	if (mask->control[band].gi == NL80211_TXRATE_DEFAULT_GI &&
	    mask->control[band].he_gi == GENMASK(7, 0) &&
	    mask->control[band].he_ltf == GENMASK(7, 0) &&
	    nrates != 1)
		return 0;

	/* fixed single rate */
	if (nrates == 1) {
		ret = mt7915_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_MCS);
		if (ret)
			return ret;
	}

	/* fixed GI */
	if (mask->control[band].gi != NL80211_TXRATE_DEFAULT_GI ||
	    mask->control[band].he_gi != GENMASK(7, 0)) {
		struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
		u32 addr;

		/* firmware updates only TXCMD but doesn't take WTBL into
		 * account, so driver should update here to reflect the
		 * actual txrate hardware sends out.
		 */
		addr = mt7915_mac_wtbl_lmac_addr(dev, msta->wcid.idx, 7);
		if (sta->he_cap.has_he)
			mt76_rmw_field(dev, addr, GENMASK(31, 24), phy.sgi);
		else
			mt76_rmw_field(dev, addr, GENMASK(15, 12), phy.sgi);

		ret = mt7915_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_GI);
		if (ret)
			return ret;
	}

	/* fixed HE_LTF */
	if (mask->control[band].he_ltf != GENMASK(7, 0)) {
		ret = mt7915_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_HE_LTF);
		if (ret)
			return ret;
	}

	return 0;
}

static void
mt7915_mcu_sta_rate_ctrl_tlv(struct sk_buff *skb, struct mt7915_dev *dev,
			     struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	}

	if (sta->ht_cap.ht_supported) {
		ra->supp_mode |= MODE_HT;
		ra->af = sta->ht_cap.ampdu_factor;
		ra->ht_gf = !!(sta->ht_cap.cap & IEEE80211_HT_CAP_GRN_FLD);

		    (sta->ht_cap.cap & IEEE80211_HT_CAP_LDPC_CODING))
			cap |= STA_CAP_LDPC;

		mt7915_mcu_set_sta_ht_mcs(sta, ra->ht_mcs,
					  mask->control[band].ht_mcs);
		ra->supp_ht_mcs = *(__le32 *)ra->ht_mcs;
	}

	if (sta->vht_cap.vht_supported) {
		u8 af;

		ra->supp_mode |= MODE_VHT;
		af = FIELD_GET(IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK,
		    (sta->vht_cap.cap & IEEE80211_VHT_CAP_RXLDPC))
			cap |= STA_CAP_VHT_LDPC;

		mt7915_mcu_set_sta_vht_mcs(sta, ra->supp_vht_mcs,
					   mask->control[band].vht_mcs);
	}

	if (sta->he_cap.has_he) {
		ra->supp_mode |= MODE_HE;
}

int mt7915_mcu_add_rate_ctrl(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta, bool changed)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct sk_buff *skb;
	int ret;

	skb = mt7915_mcu_alloc_sta_req(dev, mvif, msta,
				       MT7915_STA_UPDATE_MAX_SIZE);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* firmware rc algorithm refers to sta_rec_he for HE control.
	 * once dev->rc_work changes the settings driver should also
	 * update sta_rec_he here.
	 */
	if (sta->he_cap.has_he && changed)
		mt7915_mcu_sta_he_tlv(skb, sta, vif);

	/* sta_rec_ra accommodates BW, NSS and only MCS range format
	 * i.e 0-{7,8,9} for VHT.
	 */
	mt7915_mcu_sta_rate_ctrl_tlv(skb, dev, vif, sta);

	ret = mt76_mcu_skb_send_msg(&dev->mt76, skb,
				    MCU_EXT_CMD(STA_REC_UPDATE), true);
	if (ret)
		return ret;

	/* sta_rec_ra_fixed accommodates single rate, (HE)GI and HE_LTE,
	 * and updates as peer fixed rate parameters, which overrides
	 * sta_rec_ra and firmware rate control algorithm.
	 */
	return mt7915_mcu_add_rate_ctrl_fixed(dev, vif, sta);
}

static int
mt7915_mcu_add_group(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			      len);
	if (ie && ie[1] >= sizeof(*ht)) {
		ht = (void *)(ie + 2);
		vc->ldpc |= !!(le16_to_cpu(ht->cap_info) &
			       IEEE80211_HT_CAP_LDPC_CODING);
	}

	ie = cfg80211_find_ie(WLAN_EID_VHT_CAPABILITY, mgmt->u.beacon.variable,
			      len);
	return 0;
}

int mt7915_mcu_fw_log_2_host(struct mt7915_dev *dev, u8 type, u8 ctrl)
{
	struct {
		u8 ctrl_val;
		u8 pad[3];
		.ctrl_val = ctrl
	};

	if (type == MCU_FW_LOG_WA)
		return mt76_mcu_send_msg(&dev->mt76, MCU_WA_EXT_CMD(FW_LOG_2_HOST),
					 &data, sizeof(data), true);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(FW_LOG_2_HOST), &data,
				 sizeof(data), true);
}

		return ret;

	set_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state);
	ret = mt7915_mcu_fw_log_2_host(dev, MCU_FW_LOG_WM, 0);
	if (ret)
		return ret;

	ret = mt7915_mcu_fw_log_2_host(dev, MCU_FW_LOG_WA, 0);
	if (ret)
		return ret;

	ret = mt7915_mcu_set_mwds(dev, 1);