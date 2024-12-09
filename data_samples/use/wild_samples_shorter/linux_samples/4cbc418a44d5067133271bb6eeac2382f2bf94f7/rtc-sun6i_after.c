CLK_OF_DECLARE_DRIVER(sun50i_h6_rtc_clk, "allwinner,sun50i-h6-rtc",
		      sun50i_h6_rtc_clk_init);

/*
 * The R40 user manual is self-conflicting on whether the prescaler is
 * fixed or configurable. The clock diagram shows it as fixed, but there
 * is also a configurable divider in the RTC block.
 */
static const struct sun6i_rtc_clk_data sun8i_r40_rtc_data = {
	.rc_osc_rate = 16000000,
	.fixed_prescaler = 512,
};
static void __init sun8i_r40_rtc_clk_init(struct device_node *node)
{
	sun6i_rtc_clk_init(node, &sun8i_r40_rtc_data);
}
CLK_OF_DECLARE_DRIVER(sun8i_r40_rtc_clk, "allwinner,sun8i-r40-rtc",
		      sun8i_r40_rtc_clk_init);

static const struct sun6i_rtc_clk_data sun8i_v3_rtc_data = {
	.rc_osc_rate = 32000,
	.has_out_clk = 1,
};