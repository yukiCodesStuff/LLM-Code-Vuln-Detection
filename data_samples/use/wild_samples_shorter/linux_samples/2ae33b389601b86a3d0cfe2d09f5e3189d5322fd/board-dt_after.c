
	struct device_node *np = of_find_compatible_node(
		NULL, NULL, "marvell,kirkwood-gating-clock");
	struct of_phandle_args clkspec;
	struct clk *clk;

	clkspec.np = np;
	clkspec.args_count = 1;

	clkspec.args[0] = CGC_BIT_PEX0;
	orion_clkdev_add("0", "pcie",
			 of_clk_get_from_provider(&clkspec));

	orion_clkdev_add("1", "pcie",
			 of_clk_get_from_provider(&clkspec));

	clkspec.args[0] = CGC_BIT_SDIO;
	orion_clkdev_add(NULL, "mvsdio",
			 of_clk_get_from_provider(&clkspec));

	/*
	 * The ethernet interfaces forget the MAC address assigned by
	 * u-boot if the clocks are turned off. Until proper DT support
	 * is available we always enable them for now.
	 */
	clkspec.args[0] = CGC_BIT_GE0;
	clk = of_clk_get_from_provider(&clkspec);
	orion_clkdev_add(NULL, "mv643xx_eth_port.0", clk);
	clk_prepare_enable(clk);

	clkspec.args[0] = CGC_BIT_GE1;
	clk = of_clk_get_from_provider(&clkspec);
	orion_clkdev_add(NULL, "mv643xx_eth_port.1", clk);
	clk_prepare_enable(clk);
}

static void __init kirkwood_of_clk_init(void)
{