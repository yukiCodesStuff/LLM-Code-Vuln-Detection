{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	const struct rtw89_hfc_pub_cfg *pub_cfg = &param->pub_cfg;

	if (pub_cfg->grp0 + pub_cfg->grp1 != pub_cfg->pub_max)
		return 0;

	return 0;
}

static int hfc_ch_ctrl(struct rtw89_dev *rtwdev, u8 ch)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	const struct rtw89_hfc_ch_cfg *cfg = param->ch_cfg;
	int ret = 0;
	u32 val = 0;

	ret = rtw89_mac_check_mac_en(rtwdev, RTW89_MAC_0, RTW89_DMAC_SEL);
	if (ret)
		return ret;

	ret = hfc_ch_cfg_chk(rtwdev, ch);
	if (ret)
		return ret;

	if (ch > RTW89_DMA_B1HI)
		return -EINVAL;

	val = u32_encode_bits(cfg[ch].min, B_AX_MIN_PG_MASK) |
	      u32_encode_bits(cfg[ch].max, B_AX_MAX_PG_MASK) |
	      (cfg[ch].grp ? B_AX_GRP : 0);
	rtw89_write32(rtwdev, R_AX_ACH0_PAGE_CTRL + ch * 4, val);

	return 0;
}

static int hfc_upd_ch_info(struct rtw89_dev *rtwdev, u8 ch)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	struct rtw89_hfc_ch_info *info = param->ch_info;
	const struct rtw89_hfc_ch_cfg *cfg = param->ch_cfg;
	u32 val;
	u32 ret;

	ret = rtw89_mac_check_mac_en(rtwdev, RTW89_MAC_0, RTW89_DMAC_SEL);
	if (ret)
		return ret;

	if (ch > RTW89_DMA_H2C)
		return -EINVAL;

	val = rtw89_read32(rtwdev, R_AX_ACH0_PAGE_INFO + ch * 4);
	info[ch].aval = u32_get_bits(val, B_AX_AVAL_PG_MASK);
	if (ch < RTW89_DMA_H2C)
		info[ch].used = u32_get_bits(val, B_AX_USE_PG_MASK);
	else
		info[ch].used = cfg[ch].min - info[ch].aval;

	return 0;
}

static int hfc_pub_ctrl(struct rtw89_dev *rtwdev)
{
	const struct rtw89_hfc_pub_cfg *cfg = &rtwdev->mac.hfc_param.pub_cfg;
	u32 val;
	int ret;

	ret = rtw89_mac_check_mac_en(rtwdev, RTW89_MAC_0, RTW89_DMAC_SEL);
	if (ret)
		return ret;

	ret = hfc_pub_cfg_chk(rtwdev);
	if (ret)
		return ret;

	val = u32_encode_bits(cfg->grp0, B_AX_PUBPG_G0_MASK) |
	      u32_encode_bits(cfg->grp1, B_AX_PUBPG_G1_MASK);
	rtw89_write32(rtwdev, R_AX_PUB_PAGE_CTRL1, val);

	val = u32_encode_bits(cfg->wp_thrd, B_AX_WP_THRD_MASK);
	rtw89_write32(rtwdev, R_AX_WP_PAGE_CTRL2, val);

	return 0;
}

static int hfc_upd_mix_info(struct rtw89_dev *rtwdev)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	struct rtw89_hfc_pub_cfg *pub_cfg = &param->pub_cfg;
	struct rtw89_hfc_prec_cfg *prec_cfg = &param->prec_cfg;
	struct rtw89_hfc_pub_info *info = &param->pub_info;
	u32 val;
	int ret;

	ret = rtw89_mac_check_mac_en(rtwdev, RTW89_MAC_0, RTW89_DMAC_SEL);
	if (ret)
		return ret;

	val = rtw89_read32(rtwdev, R_AX_PUB_PAGE_INFO1);
	info->g0_used = u32_get_bits(val, B_AX_G0_USE_PG_MASK);
	info->g1_used = u32_get_bits(val, B_AX_G1_USE_PG_MASK);
	val = rtw89_read32(rtwdev, R_AX_PUB_PAGE_INFO3);
	info->g0_aval = u32_get_bits(val, B_AX_G0_AVAL_PG_MASK);
	info->g1_aval = u32_get_bits(val, B_AX_G1_AVAL_PG_MASK);
	info->pub_aval =
		u32_get_bits(rtw89_read32(rtwdev, R_AX_PUB_PAGE_INFO2),
			     B_AX_PUB_AVAL_PG_MASK);
	info->wp_aval =
		u32_get_bits(rtw89_read32(rtwdev, R_AX_WP_PAGE_INFO1),
			     B_AX_WP_AVAL_PG_MASK);

	val = rtw89_read32(rtwdev, R_AX_HCI_FC_CTRL);
	param->en = val & B_AX_HCI_FC_EN ? 1 : 0;
	param->h2c_en = val & B_AX_HCI_FC_CH12_EN ? 1 : 0;
	param->mode = u32_get_bits(val, B_AX_HCI_FC_MODE_MASK);
	prec_cfg->ch011_full_cond =
		u32_get_bits(val, B_AX_HCI_FC_WD_FULL_COND_MASK);
	prec_cfg->h2c_full_cond =
		u32_get_bits(val, B_AX_HCI_FC_CH12_FULL_COND_MASK);
	prec_cfg->wp_ch07_full_cond =
		u32_get_bits(val, B_AX_HCI_FC_WP_CH07_FULL_COND_MASK);
	prec_cfg->wp_ch811_full_cond =
		u32_get_bits(val, B_AX_HCI_FC_WP_CH811_FULL_COND_MASK);

	val = rtw89_read32(rtwdev, R_AX_CH_PAGE_CTRL);
	prec_cfg->ch011_prec = u32_get_bits(val, B_AX_PREC_PAGE_CH011_MASK);
	prec_cfg->h2c_prec = u32_get_bits(val, B_AX_PREC_PAGE_CH12_MASK);

	val = rtw89_read32(rtwdev, R_AX_PUB_PAGE_CTRL2);
	pub_cfg->pub_max = u32_get_bits(val, B_AX_PUBPG_ALL_MASK);

	val = rtw89_read32(rtwdev, R_AX_WP_PAGE_CTRL1);
	prec_cfg->wp_ch07_prec = u32_get_bits(val, B_AX_PREC_PAGE_WP_CH07_MASK);
	prec_cfg->wp_ch811_prec = u32_get_bits(val, B_AX_PREC_PAGE_WP_CH811_MASK);

	val = rtw89_read32(rtwdev, R_AX_WP_PAGE_CTRL2);
	pub_cfg->wp_thrd = u32_get_bits(val, B_AX_WP_THRD_MASK);

	val = rtw89_read32(rtwdev, R_AX_PUB_PAGE_CTRL1);
	pub_cfg->grp0 = u32_get_bits(val, B_AX_PUBPG_G0_MASK);
	pub_cfg->grp1 = u32_get_bits(val, B_AX_PUBPG_G1_MASK);

	ret = hfc_pub_info_chk(rtwdev);
	if (param->en && ret)
		return ret;

	return 0;
}

static void hfc_h2c_cfg(struct rtw89_dev *rtwdev)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	const struct rtw89_hfc_prec_cfg *prec_cfg = &param->prec_cfg;
	u32 val;

	val = u32_encode_bits(prec_cfg->h2c_prec, B_AX_PREC_PAGE_CH12_MASK);
	rtw89_write32(rtwdev, R_AX_CH_PAGE_CTRL, val);

	rtw89_write32_mask(rtwdev, R_AX_HCI_FC_CTRL,
			   B_AX_HCI_FC_CH12_FULL_COND_MASK,
			   prec_cfg->h2c_full_cond);
}

static void hfc_mix_cfg(struct rtw89_dev *rtwdev)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	const struct rtw89_hfc_pub_cfg *pub_cfg = &param->pub_cfg;
	const struct rtw89_hfc_prec_cfg *prec_cfg = &param->prec_cfg;
	u32 val;

	val = u32_encode_bits(prec_cfg->ch011_prec, B_AX_PREC_PAGE_CH011_MASK) |
	      u32_encode_bits(prec_cfg->h2c_prec, B_AX_PREC_PAGE_CH12_MASK);
	rtw89_write32(rtwdev, R_AX_CH_PAGE_CTRL, val);

	val = u32_encode_bits(pub_cfg->pub_max, B_AX_PUBPG_ALL_MASK);
	rtw89_write32(rtwdev, R_AX_PUB_PAGE_CTRL2, val);

	val = u32_encode_bits(prec_cfg->wp_ch07_prec,
			      B_AX_PREC_PAGE_WP_CH07_MASK) |
	      u32_encode_bits(prec_cfg->wp_ch811_prec,
			      B_AX_PREC_PAGE_WP_CH811_MASK);
	rtw89_write32(rtwdev, R_AX_WP_PAGE_CTRL1, val);

	val = u32_replace_bits(rtw89_read32(rtwdev, R_AX_HCI_FC_CTRL),
			       param->mode, B_AX_HCI_FC_MODE_MASK);
	val = u32_replace_bits(val, prec_cfg->ch011_full_cond,
			       B_AX_HCI_FC_WD_FULL_COND_MASK);
	val = u32_replace_bits(val, prec_cfg->h2c_full_cond,
			       B_AX_HCI_FC_CH12_FULL_COND_MASK);
	val = u32_replace_bits(val, prec_cfg->wp_ch07_full_cond,
			       B_AX_HCI_FC_WP_CH07_FULL_COND_MASK);
	val = u32_replace_bits(val, prec_cfg->wp_ch811_full_cond,
			       B_AX_HCI_FC_WP_CH811_FULL_COND_MASK);
	rtw89_write32(rtwdev, R_AX_HCI_FC_CTRL, val);
}

static void hfc_func_en(struct rtw89_dev *rtwdev, bool en, bool h2c_en)
{
	struct rtw89_hfc_param *param = &rtwdev->mac.hfc_param;
	u32 val;

	val = rtw89_read32(rtwdev, R_AX_HCI_FC_CTRL);
	param->en = en;
	param->h2c_en = h2c_en;
	val = en ? (val | B_AX_HCI_FC_EN) : (val & ~B_AX_HCI_FC_EN);
	val = h2c_en ? (val | B_AX_HCI_FC_CH12_EN) :
			 (val & ~B_AX_HCI_FC_CH12_EN);
	rtw89_write32(rtwdev, R_AX_HCI_FC_CTRL, val);
}

static int hfc_init(struct rtw89_dev *rtwdev, bool reset, bool en, bool h2c_en)
{
	u8 ch;
	u32 ret = 0;

	if (reset)
		ret = hfc_reset_param(rtwdev);
	if (ret)
		return ret;

	ret = rtw89_mac_check_mac_en(rtwdev, RTW89_MAC_0, RTW89_DMAC_SEL);
	if (ret)
		return ret;

	hfc_func_en(rtwdev, false, false);

	if (!en && h2c_en) {
		hfc_h2c_cfg(rtwdev);
		hfc_func_en(rtwdev, en, h2c_en);
		return ret;
	}