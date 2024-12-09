static const struct of_device_id ppc_clk_ids[] __initconst = {
	{ .compatible = "fsl,qoriq-clockgen-1.0", },
	{ .compatible = "fsl,qoriq-clockgen-2.0", },
	{}
};

static struct platform_driver ppc_corenet_clk_driver __initdata = {
	.driver = {
		.name = "ppc_corenet_clock",
		.of_match_table = ppc_clk_ids,
	},
	.probe = ppc_corenet_clk_probe,
};

static int __init ppc_corenet_clk_init(void)
{
	return platform_driver_register(&ppc_corenet_clk_driver);
}
subsys_initcall(ppc_corenet_clk_init);