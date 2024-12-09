	.name		= "pcmcdclk",
};

static struct clk *clkset_vpllsrc_list[] = {
	[0] = &clk_fin_vpll,
	[1] = &clk_sclk_hdmi27m,
};

static struct clk init_clocks_off[] = {
	{
		.name		= "rot",
		.parent		= &clk_hclk_dsys.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= (1<<29),
	.ctrlbit	= (1<<19),
};

static struct clk clk_pdma0 = {
	.name		= "pdma0",
	.parent		= &clk_hclk_psys.clk,
	.enable		= s5pv210_clk_ip0_ctrl,
	.ctrlbit	= (1 << 3),
};

static struct clk clk_pdma1 = {
	.name		= "pdma1",
	.parent		= &clk_hclk_psys.clk,
	.enable		= s5pv210_clk_ip0_ctrl,
	.ctrlbit	= (1 << 4),
};

static struct clk *clkset_uart_list[] = {
	[6] = &clk_mout_mpll.clk,
	[7] = &clk_mout_epll.clk,
};
	&clk_hsmmc1,
	&clk_hsmmc2,
	&clk_hsmmc3,
	&clk_pdma0,
	&clk_pdma1,
};

/* Clock initialisation code */
static struct clksrc_clk *sysclks[] = {
	CLKDEV_INIT(NULL, "spi_busclk0", &clk_p),
	CLKDEV_INIT("s5pv210-spi.0", "spi_busclk1", &clk_sclk_spi0.clk),
	CLKDEV_INIT("s5pv210-spi.1", "spi_busclk1", &clk_sclk_spi1.clk),
	CLKDEV_INIT("dma-pl330.0", "apb_pclk", &clk_pdma0),
	CLKDEV_INIT("dma-pl330.1", "apb_pclk", &clk_pdma1),
};

void __init s5pv210_register_clocks(void)
{
	for (ptr = 0; ptr < ARRAY_SIZE(clk_cdev); ptr++)
		s3c_disable_clocks(clk_cdev[ptr], 1);

	s3c_pwmclk_init();
}