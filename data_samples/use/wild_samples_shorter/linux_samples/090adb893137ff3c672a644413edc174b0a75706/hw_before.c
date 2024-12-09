	}
}

void rtl92cu_update_hal_rate_table(struct ieee80211_hw *hw,
				   struct ieee80211_sta *sta,
				   u8 rssi_level)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u32 ratr_value = (u32) mac->basic_rates;
	u8 *mcsrate = mac->mcs;
	u8 ratr_index = 0;
	u8 nmode = mac->ht_enable;
	u8 mimo_ps = 1;
	u16 shortgi_rate = 0;
	u32 tmp_ratr_value = 0;
	u8 curtxbw_40mhz = mac->bw_40;
	u8 curshortgi_40mhz = mac->sgi_40;
	u8 curshortgi_20mhz = mac->sgi_20;
	enum wireless_mode wirelessmode = mac->mode;

	ratr_value |= ((*(u16 *) (mcsrate))) << 12;
	switch (wirelessmode) {
	case WIRELESS_MODE_B:
		if (ratr_value & 0x0000000c)
			ratr_value &= 0x0000000d;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		nmode = 1;
		if (mimo_ps == 0) {
			ratr_value &= 0x0007F005;
		} else {
			u32 ratr_mask;

				ratr_mask = 0x000ff005;
			else
				ratr_mask = 0x0f0ff005;
			if (curtxbw_40mhz)
				ratr_mask |= 0x00000010;
			ratr_value &= ratr_mask;
		}
		break;
	default:
			ratr_value &= 0x000ff0ff;
		else
			ratr_value &= 0x0f0ff0ff;
		break;
	}
	ratr_value &= 0x0FFFFFFF;
	if (nmode && ((curtxbw_40mhz && curshortgi_40mhz) ||
	    (!curtxbw_40mhz && curshortgi_20mhz))) {
		ratr_value |= 0x10000000;
		tmp_ratr_value = (ratr_value >> 12);
		for (shortgi_rate = 15; shortgi_rate > 0; shortgi_rate--) {
			if ((1 << shortgi_rate) & tmp_ratr_value)
				break;
		}
		shortgi_rate = (shortgi_rate << 12) | (shortgi_rate << 8) |
			       (shortgi_rate << 4) | (shortgi_rate);
	}
	rtl_write_dword(rtlpriv, REG_ARFR0 + ratr_index * 4, ratr_value);
}

void rtl92cu_update_hal_rate_mask(struct ieee80211_hw *hw, u8 rssi_level)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_phy *rtlphy = &(rtlpriv->phy);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u32 ratr_bitmap = (u32) mac->basic_rates;
	u8 *p_mcsrate = mac->mcs;
	u8 ratr_index = 0;
	u8 curtxbw_40mhz = mac->bw_40;
	u8 curshortgi_40mhz = mac->sgi_40;
	u8 curshortgi_20mhz = mac->sgi_20;
	enum wireless_mode wirelessmode = mac->mode;
	bool shortgi = false;
	u8 rate_mask[5];
	u8 macid = 0;
	u8 mimops = 1;

	ratr_bitmap |= (p_mcsrate[1] << 20) | (p_mcsrate[0] << 12);
	switch (wirelessmode) {
	case WIRELESS_MODE_B:
		ratr_index = RATR_INX_WIRELESS_B;
		if (ratr_bitmap & 0x0000000c)
		break;
	case WIRELESS_MODE_G:
		ratr_index = RATR_INX_WIRELESS_GB;
		if (rssi_level == 1)
			ratr_bitmap &= 0x00000f00;
		else if (rssi_level == 2)
			ratr_bitmap &= 0x00000ff0;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		ratr_index = RATR_INX_WIRELESS_NGB;
		if (mimops == 0) {
			if (rssi_level == 1)
				ratr_bitmap &= 0x00070000;
			else if (rssi_level == 2)
				ratr_bitmap &= 0x0007f000;
				}
			}
		}
		if ((curtxbw_40mhz && curshortgi_40mhz) ||
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		break;
	default:
		ratr_index = RATR_INX_WIRELESS_NGB;
		if (rtlphy->rf_type == RF_1T2R)
			ratr_bitmap &= 0x000ff0ff;
		else
			ratr_bitmap &= 0x0f0ff0ff;
		break;
	}
	RT_TRACE(rtlpriv, COMP_RATR, DBG_DMESG, "ratr_bitmap :%x\n",
		 ratr_bitmap);
	*(u32 *)&rate_mask = ((ratr_bitmap & 0x0fffffff) |
				      ratr_index << 28);
	rate_mask[4] = macid | (shortgi ? 0x20 : 0x00) | 0x80;
	RT_TRACE(rtlpriv, COMP_RATR, DBG_DMESG,
		 "Rate_index:%x, ratr_val:%x, %5phC\n",
		 ratr_index, ratr_bitmap, rate_mask);
	rtl92c_fill_h2c_cmd(hw, H2C_RA_MASK, 5, rate_mask);
}

void rtl92cu_update_channel_access_setting(struct ieee80211_hw *hw)
{