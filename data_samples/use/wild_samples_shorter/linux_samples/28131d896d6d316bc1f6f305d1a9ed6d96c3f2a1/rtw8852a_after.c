		else
			rtw89_phy_write32_idx(rtwdev, R_P1_MODE,
					      B_P1_MODE_SEL,
					      0, phy_idx);
		/* SCO compensate FC setting */
		sco_comp = rtw8852a_sco_mapping(central_ch);
		rtw89_phy_write32_idx(rtwdev, R_FC0_BW, B_FC0_BW_INV,
				      sco_comp, phy_idx);