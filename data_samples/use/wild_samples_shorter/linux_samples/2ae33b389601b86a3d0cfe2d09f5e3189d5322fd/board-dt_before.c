
	struct device_node *np = of_find_compatible_node(
		NULL, NULL, "marvell,kirkwood-gating-clock");

	struct of_phandle_args clkspec;

	clkspec.np = np;
	clkspec.args_count = 1;

	clkspec.args[0] = CGC_BIT_GE0;
	orion_clkdev_add(NULL, "mv643xx_eth_port.0",
			 of_clk_get_from_provider(&clkspec));

	clkspec.args[0] = CGC_BIT_PEX0;
	orion_clkdev_add("0", "pcie",
			 of_clk_get_from_provider(&clkspec));

	orion_clkdev_add("1", "pcie",
			 of_clk_get_from_provider(&clkspec));

	clkspec.args[0] = CGC_BIT_GE1;
	orion_clkdev_add(NULL, "mv643xx_eth_port.1",
			 of_clk_get_from_provider(&clkspec));
}

static void __init kirkwood_of_clk_init(void)
{