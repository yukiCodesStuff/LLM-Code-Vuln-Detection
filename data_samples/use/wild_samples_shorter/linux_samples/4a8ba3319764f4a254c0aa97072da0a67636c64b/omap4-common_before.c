}
omap_early_initcall(omap4_sar_ram_init);

void __init omap_gic_of_init(void)
{
	struct device_node *np;
