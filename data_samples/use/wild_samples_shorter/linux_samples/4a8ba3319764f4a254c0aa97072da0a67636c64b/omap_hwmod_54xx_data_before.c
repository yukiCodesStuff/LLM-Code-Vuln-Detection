	.class		= &omap54xx_dma_hwmod_class,
	.clkdm_name	= "dma_clkdm",
	.mpu_irqs	= omap54xx_dma_system_irqs,
	.main_clk	= "l3_iclk_div",
	.prcm = {
		.omap4 = {
			.clkctrl_offs = OMAP54XX_CM_DMA_DMA_SYSTEM_CLKCTRL_OFFSET,