static const struct berlin2_gate_data bg2q_gates[] __initconst = {
	{ "gfx2daxi",	"perif",	5 },
	{ "geth0",	"perif",	8 },
	{ "sata",	"perif",	9 },
	{ "ahbapb",	"perif",	10, CLK_IGNORE_UNUSED },
	{ "usb0",	"perif",	11 },
	{ "usb1",	"perif",	12 },
	{ "usb2",	"perif",	13 },
	{ "usb3",	"perif",	14 },
	{ "pbridge",	"perif",	15, CLK_IGNORE_UNUSED },
	{ "sdio",	"perif",	16, CLK_IGNORE_UNUSED },
	{ "nfc",	"perif",	18 },
	{ "pcie",	"perif",	22 },
};

static void __init berlin2q_clock_setup(struct device_node *np)
{
	const char *parent_names[9];
	struct clk *clk;
	int n;

	gbase = of_iomap(np, 0);
	if (!gbase) {
		pr_err("%s: Unable to map global base\n", np->full_name);
		return;
	}

	/* BG2Q CPU PLL is not part of global registers */
	cpupll_base = of_iomap(np, 1);
	if (!cpupll_base) {
		pr_err("%s: Unable to map cpupll base\n", np->full_name);
		iounmap(gbase);
		return;
	}

	/* overwrite default clock names with DT provided ones */
	clk = of_clk_get_by_name(np, clk_names[REFCLK]);
	if (!IS_ERR(clk)) {
		clk_names[REFCLK] = __clk_get_name(clk);
		clk_put(clk);
	}

	/* simple register PLLs */
	clk = berlin2_pll_register(&bg2q_pll_map, gbase + REG_SYSPLLCTL0,
				   clk_names[SYSPLL], clk_names[REFCLK], 0);
	if (IS_ERR(clk))
		goto bg2q_fail;

	clk = berlin2_pll_register(&bg2q_pll_map, cpupll_base,
				   clk_names[CPUPLL], clk_names[REFCLK], 0);
	if (IS_ERR(clk))
		goto bg2q_fail;

	/* TODO: add BG2Q AVPLL */

	/*
	 * TODO: add reference clock bypass switches:
	 * memPLLSWBypass, cpuPLLSWBypass, and sysPLLSWBypass
	 */

	/* clock divider cells */
	for (n = 0; n < ARRAY_SIZE(bg2q_divs); n++) {
		const struct berlin2_div_data *dd = &bg2q_divs[n];
		int k;

		for (k = 0; k < dd->num_parents; k++)
			parent_names[k] = clk_names[dd->parent_ids[k]];

		clks[CLKID_SYS + n] = berlin2_div_register(&dd->map, gbase,
				dd->name, dd->div_flags, parent_names,
				dd->num_parents, dd->flags, &lock);
	}

	/* clock gate cells */
	for (n = 0; n < ARRAY_SIZE(bg2q_gates); n++) {
		const struct berlin2_gate_data *gd = &bg2q_gates[n];

		clks[CLKID_GFX2DAXI + n] = clk_register_gate(NULL, gd->name,
			    gd->parent_name, gd->flags, gbase + REG_CLKENABLE,
			    gd->bit_idx, 0, &lock);
	}

	/*
	 * twdclk is derived from cpu/3
	 * TODO: use cpupll until cpuclk is not available
	 */
	clks[CLKID_TWD] =
		clk_register_fixed_factor(NULL, "twd", clk_names[CPUPLL],
					  0, 1, 3);

	/* check for errors on leaf clocks */
	for (n = 0; n < MAX_CLKS; n++) {
		if (!IS_ERR(clks[n]))
			continue;

		pr_err("%s: Unable to register leaf clock %d\n",
		       np->full_name, n);
		goto bg2q_fail;
	}

	/* register clk-provider */
	clk_data.clks = clks;
	clk_data.clk_num = MAX_CLKS;
	of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);

	return;

bg2q_fail:
	iounmap(cpupll_base);
	iounmap(gbase);
}