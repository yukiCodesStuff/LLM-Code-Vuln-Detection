	{}
};

static struct platform_driver ppc_corenet_clk_driver = {
	.driver = {
		.name = "ppc_corenet_clock",
		.of_match_table = ppc_clk_ids,
	},