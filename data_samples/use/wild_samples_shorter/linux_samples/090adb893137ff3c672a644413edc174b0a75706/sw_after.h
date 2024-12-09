u32 rtl92cu_phy_query_rf_reg(struct ieee80211_hw *hw,
			    enum radio_path rfpath, u32 regaddr, u32 bitmask);
void rtl92cu_phy_set_bw_mode_callback(struct ieee80211_hw *hw);
void rtl92cu_update_hal_rate_tbl(struct ieee80211_hw *hw,
				 struct ieee80211_sta *sta,
				 u8 rssi_level);

#endif