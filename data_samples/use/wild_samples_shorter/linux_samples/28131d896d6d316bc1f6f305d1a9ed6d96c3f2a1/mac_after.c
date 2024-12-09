	const struct rtw89_hfc_pub_cfg *pub_cfg = &param->pub_cfg;

	if (pub_cfg->grp0 + pub_cfg->grp1 != pub_cfg->pub_max)
		return -EFAULT;

	return 0;
}
