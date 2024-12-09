	} else {
		/* Path B */
		rtw8852a_ch_setting(rtwdev, central_ch, RF_PATH_B);
		if (is_2g)
			rtw89_phy_write32_idx(rtwdev, R_P1_MODE,
					      B_P1_MODE_SEL,
					      1, phy_idx);
		else
			rtw89_phy_write32_idx(rtwdev, R_P1_MODE,
					      B_P1_MODE_SEL,
					      1, phy_idx);
		/* SCO compensate FC setting */
		sco_comp = rtw8852a_sco_mapping(central_ch);
		rtw89_phy_write32_idx(rtwdev, R_FC0_BW, B_FC0_BW_INV,
				      sco_comp, phy_idx);
	}

	/* Band edge */
	if (is_2g)
		rtw89_phy_write32_idx(rtwdev, R_BANDEDGE, B_BANDEDGE_EN, 1,
				      phy_idx);
	else
		rtw89_phy_write32_idx(rtwdev, R_BANDEDGE, B_BANDEDGE_EN, 0,
				      phy_idx);

	/* CCK parameters */
	if (central_ch == 14) {
		rtw89_phy_write32_mask(rtwdev, R_TXFIR0, B_TXFIR_C01,
				       0x3b13ff);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR2, B_TXFIR_C23,
				       0x1c42de);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR4, B_TXFIR_C45,
				       0xfdb0ad);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR6, B_TXFIR_C67,
				       0xf60f6e);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR8, B_TXFIR_C89,
				       0xfd8f92);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRA, B_TXFIR_CAB, 0x2d011);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRC, B_TXFIR_CCD, 0x1c02c);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRE, B_TXFIR_CEF,
				       0xfff00a);
	} else {
		rtw89_phy_write32_mask(rtwdev, R_TXFIR0, B_TXFIR_C01,
				       0x3d23ff);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR2, B_TXFIR_C23,
				       0x29b354);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR4, B_TXFIR_C45, 0xfc1c8);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR6, B_TXFIR_C67,
				       0xfdb053);
		rtw89_phy_write32_mask(rtwdev, R_TXFIR8, B_TXFIR_C89,
				       0xf86f9a);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRA, B_TXFIR_CAB,
				       0xfaef92);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRC, B_TXFIR_CCD,
				       0xfe5fcc);
		rtw89_phy_write32_mask(rtwdev, R_TXFIRE, B_TXFIR_CEF,
				       0xffdff5);
	}