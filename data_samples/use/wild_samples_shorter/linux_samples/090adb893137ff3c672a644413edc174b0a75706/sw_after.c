	.update_interrupt_mask = rtl92cu_update_interrupt_mask,
	.get_hw_reg = rtl92cu_get_hw_reg,
	.set_hw_reg = rtl92cu_set_hw_reg,
	.update_rate_tbl = rtl92cu_update_hal_rate_tbl,
	.fill_tx_desc = rtl92cu_tx_fill_desc,
	.fill_fake_txdesc = rtl92cu_fill_fake_txdesc,
	.fill_tx_cmddesc = rtl92cu_tx_fill_cmddesc,
	.cmd_send_packet = rtl92cu_cmd_send_packet,
	.phy_lc_calibrate = _rtl92cu_phy_lc_calibrate,
	.phy_set_bw_mode_callback = rtl92cu_phy_set_bw_mode_callback,
	.dm_dynamic_txpower = rtl92cu_dm_dynamic_txpower,
	.fill_h2c_cmd = rtl92c_fill_h2c_cmd,
};

static struct rtl_mod_params rtl92cu_mod_params = {
	.sw_crypto = 0,