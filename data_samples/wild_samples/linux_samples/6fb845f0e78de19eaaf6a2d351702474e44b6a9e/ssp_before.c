{
	struct resource *res;
	struct ssp_device *ssp;

	ssp = platform_get_drvdata(pdev);
	if (ssp == NULL)
		return -ENODEV;

	iounmap(ssp->mmio_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	clk_put(ssp->clk);

	mutex_lock(&ssp_lock);
	list_del(&ssp->node);
	mutex_unlock(&ssp_lock);

	kfree(ssp);
	return 0;
}

static const struct platform_device_id ssp_id_table[] = {
	{ "pxa25x-ssp",		PXA25x_SSP },
	{ "pxa25x-nssp",	PXA25x_NSSP },
	{ "pxa27x-ssp",		PXA27x_SSP },
	{ "pxa3xx-ssp",		PXA3xx_SSP },
	{ "pxa168-ssp",		PXA168_SSP },
	{ "pxa910-ssp",		PXA910_SSP },
	{ },
};

static struct platform_driver pxa_ssp_driver = {
	.probe		= pxa_ssp_probe,
	.remove		= pxa_ssp_remove,
	.driver		= {
		.name		= "pxa2xx-ssp",
		.of_match_table	= of_match_ptr(pxa_ssp_of_ids),
	},
	.id_table	= ssp_id_table,
};

static int __init pxa_ssp_init(void)
{
	return platform_driver_register(&pxa_ssp_driver);
}

static void __exit pxa_ssp_exit(void)
{
	platform_driver_unregister(&pxa_ssp_driver);
}

arch_initcall(pxa_ssp_init);
module_exit(pxa_ssp_exit);

MODULE_DESCRIPTION("PXA SSP driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");
{
	struct resource *res;
	struct ssp_device *ssp;

	ssp = platform_get_drvdata(pdev);
	if (ssp == NULL)
		return -ENODEV;

	iounmap(ssp->mmio_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	clk_put(ssp->clk);

	mutex_lock(&ssp_lock);
	list_del(&ssp->node);
	mutex_unlock(&ssp_lock);

	kfree(ssp);
	return 0;
}

static const struct platform_device_id ssp_id_table[] = {
	{ "pxa25x-ssp",		PXA25x_SSP },
	{ "pxa25x-nssp",	PXA25x_NSSP },
	{ "pxa27x-ssp",		PXA27x_SSP },
	{ "pxa3xx-ssp",		PXA3xx_SSP },
	{ "pxa168-ssp",		PXA168_SSP },
	{ "pxa910-ssp",		PXA910_SSP },
	{ },
};

static struct platform_driver pxa_ssp_driver = {
	.probe		= pxa_ssp_probe,
	.remove		= pxa_ssp_remove,
	.driver		= {
		.name		= "pxa2xx-ssp",
		.of_match_table	= of_match_ptr(pxa_ssp_of_ids),
	},
	.id_table	= ssp_id_table,
};

static int __init pxa_ssp_init(void)
{
	return platform_driver_register(&pxa_ssp_driver);
}

static void __exit pxa_ssp_exit(void)
{
	platform_driver_unregister(&pxa_ssp_driver);
}

arch_initcall(pxa_ssp_init);
module_exit(pxa_ssp_exit);

MODULE_DESCRIPTION("PXA SSP driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");