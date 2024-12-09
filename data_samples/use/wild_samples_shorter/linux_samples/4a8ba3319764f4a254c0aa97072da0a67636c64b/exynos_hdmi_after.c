
static void hdmiphy_conf_reset(struct hdmi_context *hdata)
{
	u32 reg;

	clk_disable_unprepare(hdata->res.sclk_hdmi);
	clk_set_parent(hdata->res.mout_hdmi, hdata->res.sclk_pixel);
	clk_prepare_enable(hdata->res.sclk_hdmi);

	/* operation mode */
	hdmiphy_reg_writeb(hdata, HDMIPHY_MODE_SET_DONE,
				HDMI_PHY_ENABLE_MODE_SET);

	if (hdata->type == HDMI_TYPE13)
		reg = HDMI_V13_PHY_RSTOUT;
	else