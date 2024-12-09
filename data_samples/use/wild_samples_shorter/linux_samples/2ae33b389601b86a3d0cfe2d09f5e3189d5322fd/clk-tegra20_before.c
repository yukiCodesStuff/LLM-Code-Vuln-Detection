	TEGRA_CLK_DUPLICATE(usbd,   "tegra-ehci.0", NULL),
	TEGRA_CLK_DUPLICATE(usbd,   "tegra-otg",    NULL),
	TEGRA_CLK_DUPLICATE(cclk,   NULL,           "cpu"),
	TEGRA_CLK_DUPLICATE(twd,    "smp_twd",      NULL),
	TEGRA_CLK_DUPLICATE(clk_max, NULL, NULL), /* Must be the last entry */
};

static const struct of_device_id pmc_match[] __initconst = {