enum _BOARD_TYPE_8192CUSB {
	BOARD_USB_DONGLE		= 0,	/* USB dongle */
	BOARD_USB_High_PA		= 1,	/* USB dongle - high power PA */
	BOARD_MINICARD			= 2,	/* Minicard */
	BOARD_USB_SOLO			= 3,	/* USB solo-Slim module */
	BOARD_USB_COMBO			= 4,	/* USB Combo-Slim module */
};

#define IS_HIGHT_PA(boardtype)		\
	((boardtype == BOARD_USB_High_PA) ? true : false)

#define RTL92C_DRIVER_INFO_SIZE				4
void rtl92cu_read_eeprom_info(struct ieee80211_hw *hw);
void rtl92cu_enable_hw_security_config(struct ieee80211_hw *hw);
int rtl92cu_hw_init(struct ieee80211_hw *hw);
void rtl92cu_card_disable(struct ieee80211_hw *hw);
int rtl92cu_set_network_type(struct ieee80211_hw *hw, enum nl80211_iftype type);
void rtl92cu_set_beacon_related_registers(struct ieee80211_hw *hw);
void rtl92cu_set_beacon_interval(struct ieee80211_hw *hw);
void rtl92cu_update_interrupt_mask(struct ieee80211_hw *hw,
				   u32 add_msr, u32 rm_msr);
void rtl92cu_get_hw_reg(struct ieee80211_hw *hw, u8 variable, u8 *val);
void rtl92cu_set_hw_reg(struct ieee80211_hw *hw, u8 variable, u8 *val);

void rtl92cu_update_channel_access_setting(struct ieee80211_hw *hw);
bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid);
void rtl92cu_set_check_bssid(struct ieee80211_hw *hw, bool check_bssid);
int rtl92c_download_fw(struct ieee80211_hw *hw);
void rtl92c_set_fw_pwrmode_cmd(struct ieee80211_hw *hw, u8 mode);
void rtl92c_set_fw_rsvdpagepkt(struct ieee80211_hw *hw, bool dl_finished);
void rtl92c_set_fw_joinbss_report_cmd(struct ieee80211_hw *hw, u8 mstatus);
void rtl92c_fill_h2c_cmd(struct ieee80211_hw *hw,
			 u8 element_id, u32 cmd_len, u8 *p_cmdbuffer);
bool rtl92cu_phy_mac_config(struct ieee80211_hw *hw);

#endif