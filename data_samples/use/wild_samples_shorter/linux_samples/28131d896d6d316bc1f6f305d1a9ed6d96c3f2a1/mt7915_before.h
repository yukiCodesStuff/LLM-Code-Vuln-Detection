
	bool dbdc_support;
	bool flash_mode;
	bool fw_debug;
	bool ibf;

	void *cal;

	struct {
int mt7915_mcu_add_obss_spr(struct mt7915_dev *dev, struct ieee80211_vif *vif,
                            bool enable);
int mt7915_mcu_add_rate_ctrl(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta);
int mt7915_mcu_add_he(struct mt7915_dev *dev, struct ieee80211_vif *vif,
		      struct ieee80211_sta *sta);
int mt7915_mcu_add_smps(struct mt7915_dev *dev, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta);
int mt7915_set_channel(struct mt7915_phy *phy);
int mt7915_mcu_set_chan_info(struct mt7915_phy *phy, int cmd);
int mt7915_mcu_set_tx(struct mt7915_dev *dev, struct ieee80211_vif *vif);
int mt7915_mcu_update_edca(struct mt7915_dev *dev, void *req);
int mt7915_mcu_set_fixed_rate(struct mt7915_dev *dev,
			      struct ieee80211_sta *sta, u32 rate);
int mt7915_mcu_set_eeprom(struct mt7915_dev *dev);
int mt7915_mcu_get_eeprom(struct mt7915_dev *dev, u32 offset);
int mt7915_mcu_set_mac(struct mt7915_dev *dev, int band, bool enable,
		       bool hdr_trans);
			   struct ieee80211_sta *sta, struct rate_info *rate);
int mt7915_mcu_rdd_cmd(struct mt7915_dev *dev, enum mt7915_rdd_cmd cmd,
		       u8 index, u8 rx_sel, u8 val);
int mt7915_mcu_fw_log_2_host(struct mt7915_dev *dev, u8 ctrl);
int mt7915_mcu_fw_dbg_ctrl(struct mt7915_dev *dev, u32 module, u8 level);
void mt7915_mcu_rx_event(struct mt7915_dev *dev, struct sk_buff *skb);
void mt7915_mcu_exit(struct mt7915_dev *dev);

		mt76_set_irq_mask(&dev->mt76, MT_INT_MASK_CSR, mask, 0);
}

bool mt7915_mac_wtbl_update(struct mt7915_dev *dev, int idx, u32 mask);
void mt7915_mac_reset_counters(struct mt7915_phy *phy);
void mt7915_mac_cca_stats_reset(struct mt7915_phy *phy);
void mt7915_mac_enable_nf(struct mt7915_dev *dev, bool ext_phy);