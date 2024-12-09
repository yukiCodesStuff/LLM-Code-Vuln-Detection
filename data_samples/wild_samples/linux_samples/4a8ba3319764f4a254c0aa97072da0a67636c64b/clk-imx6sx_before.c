	if (IS_ENABLED(CONFIG_USB_MXS_PHY)) {
		clk_prepare_enable(clks[IMX6SX_CLK_USBPHY1_GATE]);
		clk_prepare_enable(clks[IMX6SX_CLK_USBPHY2_GATE]);
	}

	/* Set the default 132MHz for EIM module */
	clk_set_parent(clks[IMX6SX_CLK_EIM_SLOW_SEL], clks[IMX6SX_CLK_PLL2_PFD2]);
	clk_set_rate(clks[IMX6SX_CLK_EIM_SLOW], 132000000);

	/* set parent clock for LCDIF1 pixel clock */
	clk_set_parent(clks[IMX6SX_CLK_LCDIF1_PRE_SEL], clks[IMX6SX_CLK_PLL5_VIDEO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_LCDIF1_SEL], clks[IMX6SX_CLK_LCDIF1_PODF]);

	/* Set the parent clks of PCIe lvds1 and pcie_axi to be pcie ref, axi */
	if (clk_set_parent(clks[IMX6SX_CLK_LVDS1_SEL], clks[IMX6SX_CLK_PCIE_REF_125M]))
		pr_err("Failed to set pcie bus parent clk.\n");
	if (clk_set_parent(clks[IMX6SX_CLK_PCIE_AXI_SEL], clks[IMX6SX_CLK_AXI]))
		pr_err("Failed to set pcie parent clk.\n");

	/*
	 * Init enet system AHB clock, set to 200Mhz
	 * pll2_pfd2_396m-> ENET_PODF-> ENET_AHB
	 */
	clk_set_parent(clks[IMX6SX_CLK_ENET_PRE_SEL], clks[IMX6SX_CLK_PLL2_PFD2]);
	clk_set_parent(clks[IMX6SX_CLK_ENET_SEL], clks[IMX6SX_CLK_ENET_PODF]);
	clk_set_rate(clks[IMX6SX_CLK_ENET_PODF], 200000000);
	clk_set_rate(clks[IMX6SX_CLK_ENET_REF], 125000000);
	clk_set_rate(clks[IMX6SX_CLK_ENET2_REF], 125000000);

	/* Audio clocks */
	clk_set_rate(clks[IMX6SX_CLK_PLL4_AUDIO_DIV], 393216000);

	clk_set_parent(clks[IMX6SX_CLK_SPDIF_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_SPDIF_PODF], 98304000);

	clk_set_parent(clks[IMX6SX_CLK_AUDIO_SEL], clks[IMX6SX_CLK_PLL3_USB_OTG]);
	clk_set_rate(clks[IMX6SX_CLK_AUDIO_PODF], 24000000);

	clk_set_parent(clks[IMX6SX_CLK_SSI1_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_SSI2_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_parent(clks[IMX6SX_CLK_SSI3_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_SSI1_PODF], 24576000);
	clk_set_rate(clks[IMX6SX_CLK_SSI2_PODF], 24576000);
	clk_set_rate(clks[IMX6SX_CLK_SSI3_PODF], 24576000);

	clk_set_parent(clks[IMX6SX_CLK_ESAI_SEL], clks[IMX6SX_CLK_PLL4_AUDIO_DIV]);
	clk_set_rate(clks[IMX6SX_CLK_ESAI_PODF], 24576000);

	/* Set parent clock for vadc */
	clk_set_parent(clks[IMX6SX_CLK_VID_SEL], clks[IMX6SX_CLK_PLL3_USB_OTG]);

	/* default parent of can_sel clock is invalid, manually set it here */
	clk_set_parent(clks[IMX6SX_CLK_CAN_SEL], clks[IMX6SX_CLK_PLL3_60M]);

	/* Update gpu clock from default 528M to 720M */
	clk_set_parent(clks[IMX6SX_CLK_GPU_CORE_SEL], clks[IMX6SX_CLK_PLL3_PFD0]);
	clk_set_parent(clks[IMX6SX_CLK_GPU_AXI_SEL], clks[IMX6SX_CLK_PLL3_PFD0]);

	/* Set initial power mode */
	imx6q_set_lpm(WAIT_CLOCKED);
}
CLK_OF_DECLARE(imx6sx, "fsl,imx6sx-ccm", imx6sx_clocks_init);