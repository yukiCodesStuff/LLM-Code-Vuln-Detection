				   u32 add_msr, u32 rm_msr);
void rtl92cu_get_hw_reg(struct ieee80211_hw *hw, u8 variable, u8 *val);
void rtl92cu_set_hw_reg(struct ieee80211_hw *hw, u8 variable, u8 *val);

void rtl92cu_update_channel_access_setting(struct ieee80211_hw *hw);
bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid);
void rtl92cu_set_check_bssid(struct ieee80211_hw *hw, bool check_bssid);