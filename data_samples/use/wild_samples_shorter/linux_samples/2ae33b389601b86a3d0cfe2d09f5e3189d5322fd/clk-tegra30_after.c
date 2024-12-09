	TEGRA_CLK_DUPLICATE(cml1, "tegra_sata_cml", NULL),
	TEGRA_CLK_DUPLICATE(cml0, "tegra_pcie", "cml"),
	TEGRA_CLK_DUPLICATE(pciex, "tegra_pcie", "pciex"),
	TEGRA_CLK_DUPLICATE(vcp, "nvavp", "vcp"),
	TEGRA_CLK_DUPLICATE(clk_max, NULL, NULL), /* MUST be the last entry */
};
