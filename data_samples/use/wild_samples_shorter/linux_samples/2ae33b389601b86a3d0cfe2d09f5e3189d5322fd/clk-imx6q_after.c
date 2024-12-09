static struct clk_onecell_data clk_data;

static enum mx6q_clks const clks_init_on[] __initconst = {
	mmdc_ch0_axi, rom, pll1_sys,
};

static struct clk_div_table clk_enet_ref_table[] = {
	{ .val = 0, .div = 20, },