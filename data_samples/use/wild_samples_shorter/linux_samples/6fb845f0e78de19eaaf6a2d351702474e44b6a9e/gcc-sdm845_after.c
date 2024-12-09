	"core_bi_pll_test_se",
};

static const char * const gcc_parent_names_7_ao[] = {
	"bi_tcxo_ao",
	"gpll0",
	"gpll0_out_even",
	"core_bi_pll_test_se",
};
	"core_bi_pll_test_se",
};

static const char * const gcc_parent_names_8_ao[] = {
	"bi_tcxo_ao",
	"gpll0",
	"core_bi_pll_test_se",
};

static const struct parent_map gcc_parent_map_10[] = {
	{ P_BI_TCXO, 0 },
	{ P_GPLL0_OUT_MAIN, 1 },
	{ P_GPLL4_OUT_MAIN, 5 },
	.freq_tbl = ftbl_gcc_cpuss_ahb_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gcc_cpuss_ahb_clk_src",
		.parent_names = gcc_parent_names_7_ao,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};
	.freq_tbl = ftbl_gcc_cpuss_rbcpr_clk_src,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gcc_cpuss_rbcpr_clk_src",
		.parent_names = gcc_parent_names_8_ao,
		.num_parents = 3,
		.ops = &clk_rcg2_ops,
	},
};