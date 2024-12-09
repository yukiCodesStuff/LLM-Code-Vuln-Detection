static struct tegra_clk_duplicate tegra_clk_duplicates[] = {
	TEGRA_CLK_DUPLICATE(usbd, "utmip-pad", NULL),
	TEGRA_CLK_DUPLICATE(usbd, "tegra-ehci.0", NULL),
	TEGRA_CLK_DUPLICATE(usbd, "tegra-otg", NULL),
	TEGRA_CLK_DUPLICATE(bsev, "tegra-avp", "bsev"),
	TEGRA_CLK_DUPLICATE(bsev, "nvavp", "bsev"),
	TEGRA_CLK_DUPLICATE(vde, "tegra-aes", "vde"),
	TEGRA_CLK_DUPLICATE(bsea, "tegra-aes", "bsea"),
	TEGRA_CLK_DUPLICATE(bsea, "nvavp", "bsea"),
	TEGRA_CLK_DUPLICATE(cml1, "tegra_sata_cml", NULL),
	TEGRA_CLK_DUPLICATE(cml0, "tegra_pcie", "cml"),
	TEGRA_CLK_DUPLICATE(pciex, "tegra_pcie", "pciex"),
	TEGRA_CLK_DUPLICATE(twd, "smp_twd", NULL),
	TEGRA_CLK_DUPLICATE(vcp, "nvavp", "vcp"),
	TEGRA_CLK_DUPLICATE(clk_max, NULL, NULL), /* MUST be the last entry */
};

static const struct of_device_id pmc_match[] __initconst = {
	{ .compatible = "nvidia,tegra30-pmc" },
	{},
};

void __init tegra30_clock_init(struct device_node *np)
{
	struct device_node *node;
	int i;

	clk_base = of_iomap(np, 0);
	if (!clk_base) {
		pr_err("ioremap tegra30 CAR failed\n");
		return;
	}

	node = of_find_matching_node(NULL, pmc_match);
	if (!node) {
		pr_err("Failed to find pmc node\n");
		BUG();
	}

	pmc_base = of_iomap(node, 0);
	if (!pmc_base) {
		pr_err("Can't map pmc registers\n");
		BUG();
	}

	tegra30_osc_clk_init();
	tegra30_fixed_clk_init();
	tegra30_pll_init();
	tegra30_super_clk_init();
	tegra30_periph_clk_init();
	tegra30_audio_clk_init();
	tegra30_pmc_clk_init();

	for (i = 0; i < ARRAY_SIZE(clks); i++) {
		if (IS_ERR(clks[i])) {
			pr_err("Tegra30 clk %d: register failed with %ld\n",
			       i, PTR_ERR(clks[i]));
			BUG();
		}
		if (!clks[i])
			clks[i] = ERR_PTR(-EINVAL);
	}

	tegra_init_dup_clks(tegra_clk_duplicates, clks, clk_max);

	clk_data.clks = clks;
	clk_data.clk_num = ARRAY_SIZE(clks);
	of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);

	tegra_init_from_table(init_table, clks, clk_max);

	tegra_cpu_car_ops = &tegra30_cpu_car_ops;
}