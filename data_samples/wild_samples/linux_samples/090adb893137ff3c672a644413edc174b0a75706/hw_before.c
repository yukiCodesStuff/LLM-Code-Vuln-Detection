	case HW_VAR_CORRECT_TSF:{
			u8 btype_ibss = val[0];

			if (btype_ibss)
				_rtl92cu_stop_tx_beacon(hw);
			_rtl92cu_set_bcn_ctrl_reg(hw, 0, BIT(3));
			rtl_write_dword(rtlpriv, REG_TSFTR, (u32)(mac->tsf &
					0xffffffff));
			rtl_write_dword(rtlpriv, REG_TSFTR + 4,
					(u32)((mac->tsf >> 32) & 0xffffffff));
			_rtl92cu_set_bcn_ctrl_reg(hw, BIT(3), 0);
			if (btype_ibss)
				_rtl92cu_resume_tx_beacon(hw);
			break;
		}
	case HW_VAR_MGT_FILTER:
		rtl_write_word(rtlpriv, REG_RXFLTMAP0, *(u16 *)val);
		break;
	case HW_VAR_CTRL_FILTER:
		rtl_write_word(rtlpriv, REG_RXFLTMAP1, *(u16 *)val);
		break;
	case HW_VAR_DATA_FILTER:
		rtl_write_word(rtlpriv, REG_RXFLTMAP2, *(u16 *)val);
		break;
	default:
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "switch case not processed\n");
		break;
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
		else
			ratr_value &= 0x0000000f;
		break;
	case WIRELESS_MODE_G:
		ratr_value &= 0x00000FF5;
		break;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		nmode = 1;
		if (mimo_ps == 0) {
			ratr_value &= 0x0007F005;
		} else {
			u32 ratr_mask;

			if (get_rf_type(rtlphy) == RF_1T2R ||
			    get_rf_type(rtlphy) == RF_1T1R)
				ratr_mask = 0x000ff005;
			else
				ratr_mask = 0x0f0ff005;
			if (curtxbw_40mhz)
				ratr_mask |= 0x00000010;
			ratr_value &= ratr_mask;
		}
		break;
	default:
		if (rtlphy->rf_type == RF_1T2R)
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
	switch (wirelessmode) {
	case WIRELESS_MODE_B:
		if (ratr_value & 0x0000000c)
			ratr_value &= 0x0000000d;
		else
			ratr_value &= 0x0000000f;
		break;
	case WIRELESS_MODE_G:
		ratr_value &= 0x00000FF5;
		break;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		nmode = 1;
		if (mimo_ps == 0) {
			ratr_value &= 0x0007F005;
		} else {
			u32 ratr_mask;

			if (get_rf_type(rtlphy) == RF_1T2R ||
			    get_rf_type(rtlphy) == RF_1T1R)
				ratr_mask = 0x000ff005;
			else
				ratr_mask = 0x0f0ff005;
			if (curtxbw_40mhz)
				ratr_mask |= 0x00000010;
			ratr_value &= ratr_mask;
		}
		break;
	default:
		if (rtlphy->rf_type == RF_1T2R)
			ratr_value &= 0x000ff0ff;
		else
			ratr_value &= 0x0f0ff0ff;
		break;
	}
		} else {
			u32 ratr_mask;

			if (get_rf_type(rtlphy) == RF_1T2R ||
			    get_rf_type(rtlphy) == RF_1T1R)
				ratr_mask = 0x000ff005;
			else
				ratr_mask = 0x0f0ff005;
			if (curtxbw_40mhz)
				ratr_mask |= 0x00000010;
			ratr_value &= ratr_mask;
		}
		break;
	default:
		if (rtlphy->rf_type == RF_1T2R)
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
			ratr_bitmap &= 0x0000000d;
		else
			ratr_bitmap &= 0x0000000f;
		break;
	case WIRELESS_MODE_G:
		ratr_index = RATR_INX_WIRELESS_GB;
		if (rssi_level == 1)
			ratr_bitmap &= 0x00000f00;
		else if (rssi_level == 2)
			ratr_bitmap &= 0x00000ff0;
		else
			ratr_bitmap &= 0x00000ff5;
		break;
	case WIRELESS_MODE_A:
		ratr_index = RATR_INX_WIRELESS_A;
		ratr_bitmap &= 0x00000ff0;
		break;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		ratr_index = RATR_INX_WIRELESS_NGB;
		if (mimops == 0) {
			if (rssi_level == 1)
				ratr_bitmap &= 0x00070000;
			else if (rssi_level == 2)
				ratr_bitmap &= 0x0007f000;
			else
				ratr_bitmap &= 0x0007f005;
		} else {
			if (rtlphy->rf_type == RF_1T2R ||
			    rtlphy->rf_type == RF_1T1R) {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff005;
				}
			} else {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff005;
				}
			}
		}
		if ((curtxbw_40mhz && curshortgi_40mhz) ||
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		}
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
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u16 sifs_timer;

	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SLOT_TIME,
				      &mac->slot_time);
	if (!mac->ht_enable)
		sifs_timer = 0x0a0a;
	else
		sifs_timer = 0x0e0e;
	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SIFS, (u8 *)&sifs_timer);
}

bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	enum rf_pwrstate e_rfpowerstate_toset, cur_rfstate;
	u8 u1tmp = 0;
	bool actuallyset = false;
	unsigned long flag = 0;
	/* to do - usb autosuspend */
	u8 usb_autosuspend = 0;

	if (ppsc->swrf_processing)
		return false;
	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
	if (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		return false;
	} else {
		ppsc->rfchange_inprogress = true;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	cur_rfstate = ppsc->rfpwr_state;
	if (usb_autosuspend) {
		/* to do................... */
	} else {
		if (ppsc->pwrdown_mode) {
			u1tmp = rtl_read_byte(rtlpriv, REG_HSISR);
			e_rfpowerstate_toset = (u1tmp & BIT(7)) ?
					       ERFOFF : ERFON;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "pwrdown, 0x5c(BIT7)=%02x\n", u1tmp);
		} else {
			rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG,
				       rtl_read_byte(rtlpriv,
				       REG_MAC_PINMUX_CFG) & ~(BIT(3)));
			u1tmp = rtl_read_byte(rtlpriv, REG_GPIO_IO_SEL);
			e_rfpowerstate_toset  = (u1tmp & BIT(3)) ?
						 ERFON : ERFOFF;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "GPIO_IN=%02x\n", u1tmp);
		}
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD, "N-SS RF =%x\n",
			 e_rfpowerstate_toset);
	}
	if ((ppsc->hwradiooff) && (e_rfpowerstate_toset == ERFON)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio ON, RF ON\n");
		ppsc->hwradiooff = false;
		actuallyset = true;
	} else if ((!ppsc->hwradiooff) && (e_rfpowerstate_toset  ==
		    ERFOFF)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio OFF\n");
		ppsc->hwradiooff = true;
		actuallyset = true;
	} else {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "pHalData->bHwRadioOff and eRfPowerStateToSet do not match: pHalData->bHwRadioOff %x, eRfPowerStateToSet %x\n",
			 ppsc->hwradiooff, e_rfpowerstate_toset);
	}
	if (actuallyset) {
		ppsc->hwradiooff = true;
		if (e_rfpowerstate_toset == ERFON) {
			if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM) &&
			     RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
				 && RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		/* For power down module, we need to enable register block
		 * contrl reg at 0x1c. Then enable power down control bit
		 * of register 0x04 BIT4 and BIT15 as 1.
		 */
		if (ppsc->pwrdown_mode && e_rfpowerstate_toset == ERFOFF) {
			/* Enable register area 0x0-0xc. */
			rtl_write_byte(rtlpriv, REG_RSV_CTRL, 0x0);
			if (IS_HARDWARE_TYPE_8723U(rtlhal)) {
				/*
				 * We should configure HW PDn source for WiFi
				 * ONLY, and then our HW will be set in
				 * power-down mode if PDn source from all
				 * functions are configured.
				 */
				u1tmp = rtl_read_byte(rtlpriv,
						      REG_MULTI_FUNC_CTRL);
				rtl_write_byte(rtlpriv, REG_MULTI_FUNC_CTRL,
					       (u1tmp|WL_HWPDN_EN));
			} else {
				rtl_write_word(rtlpriv, REG_APS_FSMCO, 0x8812);
			}
		}
		if (e_rfpowerstate_toset == ERFOFF) {
			if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if (ppsc->reg_rfps_level & RT_RF_OFF_LEVL_PCI_D3)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
	} else if (e_rfpowerstate_toset == ERFOFF || cur_rfstate == ERFOFF) {
		/* Enter D3 or ASPM after GPIO had been done. */
		if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
		else if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	} else {
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	*valid = 1;
	return !ppsc->hwradiooff;
}
		} else {
			u32 ratr_mask;

			if (get_rf_type(rtlphy) == RF_1T2R ||
			    get_rf_type(rtlphy) == RF_1T1R)
				ratr_mask = 0x000ff005;
			else
				ratr_mask = 0x0f0ff005;
			if (curtxbw_40mhz)
				ratr_mask |= 0x00000010;
			ratr_value &= ratr_mask;
		}
		break;
	default:
		if (rtlphy->rf_type == RF_1T2R)
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
	switch (wirelessmode) {
	case WIRELESS_MODE_B:
		ratr_index = RATR_INX_WIRELESS_B;
		if (ratr_bitmap & 0x0000000c)
			ratr_bitmap &= 0x0000000d;
		else
			ratr_bitmap &= 0x0000000f;
		break;
	case WIRELESS_MODE_G:
		ratr_index = RATR_INX_WIRELESS_GB;
		if (rssi_level == 1)
			ratr_bitmap &= 0x00000f00;
		else if (rssi_level == 2)
			ratr_bitmap &= 0x00000ff0;
		else
			ratr_bitmap &= 0x00000ff5;
		break;
	case WIRELESS_MODE_A:
		ratr_index = RATR_INX_WIRELESS_A;
		ratr_bitmap &= 0x00000ff0;
		break;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		ratr_index = RATR_INX_WIRELESS_NGB;
		if (mimops == 0) {
			if (rssi_level == 1)
				ratr_bitmap &= 0x00070000;
			else if (rssi_level == 2)
				ratr_bitmap &= 0x0007f000;
			else
				ratr_bitmap &= 0x0007f005;
		} else {
			if (rtlphy->rf_type == RF_1T2R ||
			    rtlphy->rf_type == RF_1T1R) {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff005;
				}
			} else {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff005;
				}
			}
		}
		if ((curtxbw_40mhz && curshortgi_40mhz) ||
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		}
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
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u16 sifs_timer;

	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SLOT_TIME,
				      &mac->slot_time);
	if (!mac->ht_enable)
		sifs_timer = 0x0a0a;
	else
		sifs_timer = 0x0e0e;
	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SIFS, (u8 *)&sifs_timer);
}

bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	enum rf_pwrstate e_rfpowerstate_toset, cur_rfstate;
	u8 u1tmp = 0;
	bool actuallyset = false;
	unsigned long flag = 0;
	/* to do - usb autosuspend */
	u8 usb_autosuspend = 0;

	if (ppsc->swrf_processing)
		return false;
	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
	if (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		return false;
	} else {
		ppsc->rfchange_inprogress = true;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	cur_rfstate = ppsc->rfpwr_state;
	if (usb_autosuspend) {
		/* to do................... */
	} else {
		if (ppsc->pwrdown_mode) {
			u1tmp = rtl_read_byte(rtlpriv, REG_HSISR);
			e_rfpowerstate_toset = (u1tmp & BIT(7)) ?
					       ERFOFF : ERFON;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "pwrdown, 0x5c(BIT7)=%02x\n", u1tmp);
		} else {
			rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG,
				       rtl_read_byte(rtlpriv,
				       REG_MAC_PINMUX_CFG) & ~(BIT(3)));
			u1tmp = rtl_read_byte(rtlpriv, REG_GPIO_IO_SEL);
			e_rfpowerstate_toset  = (u1tmp & BIT(3)) ?
						 ERFON : ERFOFF;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "GPIO_IN=%02x\n", u1tmp);
		}
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD, "N-SS RF =%x\n",
			 e_rfpowerstate_toset);
	}
	if ((ppsc->hwradiooff) && (e_rfpowerstate_toset == ERFON)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio ON, RF ON\n");
		ppsc->hwradiooff = false;
		actuallyset = true;
	} else if ((!ppsc->hwradiooff) && (e_rfpowerstate_toset  ==
		    ERFOFF)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio OFF\n");
		ppsc->hwradiooff = true;
		actuallyset = true;
	} else {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "pHalData->bHwRadioOff and eRfPowerStateToSet do not match: pHalData->bHwRadioOff %x, eRfPowerStateToSet %x\n",
			 ppsc->hwradiooff, e_rfpowerstate_toset);
	}
	if (actuallyset) {
		ppsc->hwradiooff = true;
		if (e_rfpowerstate_toset == ERFON) {
			if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM) &&
			     RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
				 && RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		/* For power down module, we need to enable register block
		 * contrl reg at 0x1c. Then enable power down control bit
		 * of register 0x04 BIT4 and BIT15 as 1.
		 */
		if (ppsc->pwrdown_mode && e_rfpowerstate_toset == ERFOFF) {
			/* Enable register area 0x0-0xc. */
			rtl_write_byte(rtlpriv, REG_RSV_CTRL, 0x0);
			if (IS_HARDWARE_TYPE_8723U(rtlhal)) {
				/*
				 * We should configure HW PDn source for WiFi
				 * ONLY, and then our HW will be set in
				 * power-down mode if PDn source from all
				 * functions are configured.
				 */
				u1tmp = rtl_read_byte(rtlpriv,
						      REG_MULTI_FUNC_CTRL);
				rtl_write_byte(rtlpriv, REG_MULTI_FUNC_CTRL,
					       (u1tmp|WL_HWPDN_EN));
			} else {
				rtl_write_word(rtlpriv, REG_APS_FSMCO, 0x8812);
			}
		}
		if (e_rfpowerstate_toset == ERFOFF) {
			if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if (ppsc->reg_rfps_level & RT_RF_OFF_LEVL_PCI_D3)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
	} else if (e_rfpowerstate_toset == ERFOFF || cur_rfstate == ERFOFF) {
		/* Enter D3 or ASPM after GPIO had been done. */
		if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
		else if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	} else {
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	*valid = 1;
	return !ppsc->hwradiooff;
}
	switch (wirelessmode) {
	case WIRELESS_MODE_B:
		ratr_index = RATR_INX_WIRELESS_B;
		if (ratr_bitmap & 0x0000000c)
			ratr_bitmap &= 0x0000000d;
		else
			ratr_bitmap &= 0x0000000f;
		break;
	case WIRELESS_MODE_G:
		ratr_index = RATR_INX_WIRELESS_GB;
		if (rssi_level == 1)
			ratr_bitmap &= 0x00000f00;
		else if (rssi_level == 2)
			ratr_bitmap &= 0x00000ff0;
		else
			ratr_bitmap &= 0x00000ff5;
		break;
	case WIRELESS_MODE_A:
		ratr_index = RATR_INX_WIRELESS_A;
		ratr_bitmap &= 0x00000ff0;
		break;
	case WIRELESS_MODE_N_24G:
	case WIRELESS_MODE_N_5G:
		ratr_index = RATR_INX_WIRELESS_NGB;
		if (mimops == 0) {
			if (rssi_level == 1)
				ratr_bitmap &= 0x00070000;
			else if (rssi_level == 2)
				ratr_bitmap &= 0x0007f000;
			else
				ratr_bitmap &= 0x0007f005;
		} else {
			if (rtlphy->rf_type == RF_1T2R ||
			    rtlphy->rf_type == RF_1T1R) {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x000f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x000ff000;
					else
						ratr_bitmap &= 0x000ff005;
				}
			} else {
				if (curtxbw_40mhz) {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff015;
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff005;
				}
			}
		}
		if ((curtxbw_40mhz && curshortgi_40mhz) ||
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		}
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
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u16 sifs_timer;

	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SLOT_TIME,
				      &mac->slot_time);
	if (!mac->ht_enable)
		sifs_timer = 0x0a0a;
	else
		sifs_timer = 0x0e0e;
	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SIFS, (u8 *)&sifs_timer);
}

bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	enum rf_pwrstate e_rfpowerstate_toset, cur_rfstate;
	u8 u1tmp = 0;
	bool actuallyset = false;
	unsigned long flag = 0;
	/* to do - usb autosuspend */
	u8 usb_autosuspend = 0;

	if (ppsc->swrf_processing)
		return false;
	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
	if (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		return false;
	} else {
		ppsc->rfchange_inprogress = true;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	cur_rfstate = ppsc->rfpwr_state;
	if (usb_autosuspend) {
		/* to do................... */
	} else {
		if (ppsc->pwrdown_mode) {
			u1tmp = rtl_read_byte(rtlpriv, REG_HSISR);
			e_rfpowerstate_toset = (u1tmp & BIT(7)) ?
					       ERFOFF : ERFON;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "pwrdown, 0x5c(BIT7)=%02x\n", u1tmp);
		} else {
			rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG,
				       rtl_read_byte(rtlpriv,
				       REG_MAC_PINMUX_CFG) & ~(BIT(3)));
			u1tmp = rtl_read_byte(rtlpriv, REG_GPIO_IO_SEL);
			e_rfpowerstate_toset  = (u1tmp & BIT(3)) ?
						 ERFON : ERFOFF;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "GPIO_IN=%02x\n", u1tmp);
		}
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD, "N-SS RF =%x\n",
			 e_rfpowerstate_toset);
	}
	if ((ppsc->hwradiooff) && (e_rfpowerstate_toset == ERFON)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio ON, RF ON\n");
		ppsc->hwradiooff = false;
		actuallyset = true;
	} else if ((!ppsc->hwradiooff) && (e_rfpowerstate_toset  ==
		    ERFOFF)) {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "GPIOChangeRF  - HW Radio OFF\n");
		ppsc->hwradiooff = true;
		actuallyset = true;
	} else {
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD,
			 "pHalData->bHwRadioOff and eRfPowerStateToSet do not match: pHalData->bHwRadioOff %x, eRfPowerStateToSet %x\n",
			 ppsc->hwradiooff, e_rfpowerstate_toset);
	}
	if (actuallyset) {
		ppsc->hwradiooff = true;
		if (e_rfpowerstate_toset == ERFON) {
			if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM) &&
			     RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if ((ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
				 && RT_IN_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3))
				RT_CLEAR_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		/* For power down module, we need to enable register block
		 * contrl reg at 0x1c. Then enable power down control bit
		 * of register 0x04 BIT4 and BIT15 as 1.
		 */
		if (ppsc->pwrdown_mode && e_rfpowerstate_toset == ERFOFF) {
			/* Enable register area 0x0-0xc. */
			rtl_write_byte(rtlpriv, REG_RSV_CTRL, 0x0);
			if (IS_HARDWARE_TYPE_8723U(rtlhal)) {
				/*
				 * We should configure HW PDn source for WiFi
				 * ONLY, and then our HW will be set in
				 * power-down mode if PDn source from all
				 * functions are configured.
				 */
				u1tmp = rtl_read_byte(rtlpriv,
						      REG_MULTI_FUNC_CTRL);
				rtl_write_byte(rtlpriv, REG_MULTI_FUNC_CTRL,
					       (u1tmp|WL_HWPDN_EN));
			} else {
				rtl_write_word(rtlpriv, REG_APS_FSMCO, 0x8812);
			}
		}
		if (e_rfpowerstate_toset == ERFOFF) {
			if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
			else if (ppsc->reg_rfps_level & RT_RF_OFF_LEVL_PCI_D3)
				RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		}
	} else if (e_rfpowerstate_toset == ERFOFF || cur_rfstate == ERFOFF) {
		/* Enter D3 or ASPM after GPIO had been done. */
		if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_ASPM)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_ASPM);
		else if (ppsc->reg_rfps_level  & RT_RF_OFF_LEVL_PCI_D3)
			RT_SET_PS_LEVEL(ppsc, RT_RF_OFF_LEVL_PCI_D3);
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	} else {
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
		ppsc->rfchange_inprogress = false;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	*valid = 1;
	return !ppsc->hwradiooff;
}
				} else {
					if (rssi_level == 1)
						ratr_bitmap &= 0x0f0f0000;
					else if (rssi_level == 2)
						ratr_bitmap &= 0x0f0ff000;
					else
						ratr_bitmap &= 0x0f0ff005;
				}
			}
		}
		if ((curtxbw_40mhz && curshortgi_40mhz) ||
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		}
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
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u16 sifs_timer;

	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SLOT_TIME,
				      &mac->slot_time);
	if (!mac->ht_enable)
		sifs_timer = 0x0a0a;
	else
		sifs_timer = 0x0e0e;
	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SIFS, (u8 *)&sifs_timer);
}

bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	enum rf_pwrstate e_rfpowerstate_toset, cur_rfstate;
	u8 u1tmp = 0;
	bool actuallyset = false;
	unsigned long flag = 0;
	/* to do - usb autosuspend */
	u8 usb_autosuspend = 0;

	if (ppsc->swrf_processing)
		return false;
	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
	if (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		return false;
	} else {
		ppsc->rfchange_inprogress = true;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}
	cur_rfstate = ppsc->rfpwr_state;
	if (usb_autosuspend) {
		/* to do................... */
	} else {
		if (ppsc->pwrdown_mode) {
			u1tmp = rtl_read_byte(rtlpriv, REG_HSISR);
			e_rfpowerstate_toset = (u1tmp & BIT(7)) ?
					       ERFOFF : ERFON;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "pwrdown, 0x5c(BIT7)=%02x\n", u1tmp);
		} else {
			rtl_write_byte(rtlpriv, REG_MAC_PINMUX_CFG,
				       rtl_read_byte(rtlpriv,
				       REG_MAC_PINMUX_CFG) & ~(BIT(3)));
			u1tmp = rtl_read_byte(rtlpriv, REG_GPIO_IO_SEL);
			e_rfpowerstate_toset  = (u1tmp & BIT(3)) ?
						 ERFON : ERFOFF;
			RT_TRACE(rtlpriv, COMP_POWER, DBG_DMESG,
				 "GPIO_IN=%02x\n", u1tmp);
		}
		RT_TRACE(rtlpriv, COMP_POWER, DBG_LOUD, "N-SS RF =%x\n",
			 e_rfpowerstate_toset);
	}
		    (!curtxbw_40mhz && curshortgi_20mhz)) {
			if (macid == 0)
				shortgi = true;
			else if (macid == 1)
				shortgi = false;
		}
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
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	u16 sifs_timer;

	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SLOT_TIME,
				      &mac->slot_time);
	if (!mac->ht_enable)
		sifs_timer = 0x0a0a;
	else
		sifs_timer = 0x0e0e;
	rtlpriv->cfg->ops->set_hw_reg(hw, HW_VAR_SIFS, (u8 *)&sifs_timer);
}

bool rtl92cu_gpio_radio_on_off_checking(struct ieee80211_hw *hw, u8 * valid)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	enum rf_pwrstate e_rfpowerstate_toset, cur_rfstate;
	u8 u1tmp = 0;
	bool actuallyset = false;
	unsigned long flag = 0;
	/* to do - usb autosuspend */
	u8 usb_autosuspend = 0;

	if (ppsc->swrf_processing)
		return false;
	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flag);
	if (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
		return false;
	} else {
		ppsc->rfchange_inprogress = true;
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flag);
	}