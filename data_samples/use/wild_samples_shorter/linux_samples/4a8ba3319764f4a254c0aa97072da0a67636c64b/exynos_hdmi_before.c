
static void hdmiphy_conf_reset(struct hdmi_context *hdata)
{
	u8 buffer[2];
	u32 reg;

	clk_disable_unprepare(hdata->res.sclk_hdmi);
	clk_set_parent(hdata->res.mout_hdmi, hdata->res.sclk_pixel);
	clk_prepare_enable(hdata->res.sclk_hdmi);

	/* operation mode */
	buffer[0] = 0x1f;
	buffer[1] = 0x00;

	if (hdata->hdmiphy_port)
		i2c_master_send(hdata->hdmiphy_port, buffer, 2);

	if (hdata->type == HDMI_TYPE13)
		reg = HDMI_V13_PHY_RSTOUT;
	else