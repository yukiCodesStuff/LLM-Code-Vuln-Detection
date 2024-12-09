CLK_OF_DECLARE_DRIVER(sun50i_h6_rtc_clk, "allwinner,sun50i-h6-rtc",
		      sun50i_h6_rtc_clk_init);

static const struct sun6i_rtc_clk_data sun8i_v3_rtc_data = {
	.rc_osc_rate = 32000,
	.has_out_clk = 1,
};