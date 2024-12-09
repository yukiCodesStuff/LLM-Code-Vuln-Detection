{
	struct mt7921_phy *phy = mt7921_hw_phy(hw);
	struct mt7921_dev *dev = phy->dev;
	struct wiphy *wiphy = hw->wiphy;

	hw->queues = 4;
	hw->max_rx_aggregation_subframes = 64;
	hw->max_tx_aggregation_subframes = 128;
	hw->netdev_features = NETIF_F_RXCSUM;

	hw->radiotap_timestamp.units_pos =
		IEEE80211_RADIOTAP_TIMESTAMP_UNIT_US;

	phy->slottime = 9;

	hw->sta_data_size = sizeof(struct mt7921_sta);
	hw->vif_data_size = sizeof(struct mt7921_vif);

	wiphy->iface_combinations = if_comb;
	wiphy->flags &= ~(WIPHY_FLAG_IBSS_RSN | WIPHY_FLAG_4ADDR_AP |
			  WIPHY_FLAG_4ADDR_STATION);
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	wiphy->n_iface_combinations = ARRAY_SIZE(if_comb);
	wiphy->max_scan_ie_len = MT76_CONNAC_SCAN_IE_LEN;
	wiphy->max_scan_ssids = 4;
	wiphy->max_sched_scan_plan_interval =
		MT76_CONNAC_MAX_TIME_SCHED_SCAN_INTERVAL;
	wiphy->max_sched_scan_ie_len = IEEE80211_MAX_DATA_LEN;
	wiphy->max_sched_scan_ssids = MT76_CONNAC_MAX_SCHED_SCAN_SSID;
	wiphy->max_match_sets = MT76_CONNAC_MAX_SCAN_MATCH;
	wiphy->max_sched_scan_reqs = 1;
	wiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
	wiphy->reg_notifier = mt7921_regd_notifier;
	wiphy->sar_capa = &mt76_sar_capa;

	phy->mt76->frp = devm_kcalloc(dev->mt76.dev,
				      wiphy->sar_capa->num_freq_ranges,
				      sizeof(struct mt76_freq_range_power),
				      GFP_KERNEL);
	if (!phy->mt76->frp)
		return -ENOMEM;

	wiphy->features |= NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR |
			   NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR;
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_SET_SCAN_DWELL);

	ieee80211_hw_set(hw, SINGLE_SCAN_ON_ALL_BANDS);
	ieee80211_hw_set(hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(hw, SUPPORTS_TX_ENCAP_OFFLOAD);
	ieee80211_hw_set(hw, SUPPORTS_RX_DECAP_OFFLOAD);
	ieee80211_hw_set(hw, WANT_MONITOR_VIF);
	ieee80211_hw_set(hw, SUPPORTS_PS);
	ieee80211_hw_set(hw, SUPPORTS_DYNAMIC_PS);

	if (dev->pm.enable)
		ieee80211_hw_set(hw, CONNECTION_MONITOR);

	hw->max_tx_fragments = 4;

	return 0;
}

static void
mt7921_mac_init_band(struct mt7921_dev *dev, u8 band)
{
	mt76_rmw_field(dev, MT_TMAC_CTCR0(band),
		       MT_TMAC_CTCR0_INS_DDLMT_REFTIME, 0x3f);
	mt76_set(dev, MT_TMAC_CTCR0(band),
		 MT_TMAC_CTCR0_INS_DDLMT_VHT_SMPDU_EN |
		 MT_TMAC_CTCR0_INS_DDLMT_EN);

	mt76_set(dev, MT_WF_RMAC_MIB_TIME0(band), MT_WF_RMAC_MIB_RXTIME_EN);
	mt76_set(dev, MT_WF_RMAC_MIB_AIRTIME0(band), MT_WF_RMAC_MIB_RXTIME_EN);

	/* enable MIB tx-rx time reporting */
	mt76_set(dev, MT_MIB_SCR1(band), MT_MIB_TXDUR_EN);
	mt76_set(dev, MT_MIB_SCR1(band), MT_MIB_RXDUR_EN);

	mt76_rmw_field(dev, MT_DMA_DCR0(band), MT_DMA_DCR0_MAX_RX_LEN, 1536);
	/* disable rx rate report by default due to hw issues */
	mt76_clear(dev, MT_DMA_DCR0(band), MT_DMA_DCR0_RXD_G5_EN);
}

int mt7921_mac_init(struct mt7921_dev *dev)
{
	int i;

	mt76_rmw_field(dev, MT_MDP_DCR1, MT_MDP_DCR1_MAX_RX_LEN, 1536);
	/* enable hardware de-agg */
	mt76_set(dev, MT_MDP_DCR0, MT_MDP_DCR0_DAMSDU_EN);
	/* enable hardware rx header translation */
	mt76_set(dev, MT_MDP_DCR0, MT_MDP_DCR0_RX_HDR_TRANS_EN);

	for (i = 0; i < MT7921_WTBL_SIZE; i++)
		mt7921_mac_wtbl_update(dev, i,
				       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);
	for (i = 0; i < 2; i++)
		mt7921_mac_init_band(dev, i);

	dev->mt76.rxfilter = mt76_rr(dev, MT_WF_RFCR(0));

	return mt76_connac_mcu_set_rts_thresh(&dev->mt76, 0x92b, 0);
}
EXPORT_SYMBOL_GPL(mt7921_mac_init);

static int __mt7921_init_hardware(struct mt7921_dev *dev)
{
	int ret;

	/* force firmware operation mode into normal state,
	 * which should be set before firmware download stage.
	 */
	mt76_wr(dev, MT_SWDEF_MODE, MT_SWDEF_NORMAL_MODE);
	ret = mt7921_mcu_init(dev);
	if (ret)
		goto out;

	mt76_eeprom_override(&dev->mphy);

	ret = mt7921_mcu_set_eeprom(dev);
	if (ret)
		goto out;

	ret = mt7921_mac_init(dev);
out:
	return ret;
}

static int mt7921_init_hardware(struct mt7921_dev *dev)
{
	int ret, idx, i;

	set_bit(MT76_STATE_INITIALIZED, &dev->mphy.state);

	for (i = 0; i < MT7921_MCU_INIT_RETRY_COUNT; i++) {
		ret = __mt7921_init_hardware(dev);
		if (!ret)
			break;

		mt7921_init_reset(dev);
	}