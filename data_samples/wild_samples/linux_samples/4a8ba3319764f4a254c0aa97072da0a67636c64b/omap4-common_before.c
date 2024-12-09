{
	unsigned long sar_base;

	/*
	 * To avoid code running on other OMAPs in
	 * multi-omap builds
	 */
	if (cpu_is_omap44xx())
		sar_base = OMAP44XX_SAR_RAM_BASE;
	else if (soc_is_omap54xx())
		sar_base = OMAP54XX_SAR_RAM_BASE;
	else
		return -ENOMEM;

	/* Static mapping, never released */
	sar_ram_base = ioremap(sar_base, SZ_16K);
	if (WARN_ON(!sar_ram_base))
		return -ENOMEM;

	return 0;
}
omap_early_initcall(omap4_sar_ram_init);

void __init omap_gic_of_init(void)
{
	struct device_node *np;

	/* Extract GIC distributor and TWD bases for OMAP4460 ROM Errata WA */
	if (!cpu_is_omap446x())
		goto skip_errata_init;

	np = of_find_compatible_node(NULL, NULL, "arm,cortex-a9-gic");
	gic_dist_base_addr = of_iomap(np, 0);
	WARN_ON(!gic_dist_base_addr);

	np = of_find_compatible_node(NULL, NULL, "arm,cortex-a9-twd-timer");
	twd_base = of_iomap(np, 0);
	WARN_ON(!twd_base);

skip_errata_init:
	omap_wakeupgen_init();
#ifdef CONFIG_IRQ_CROSSBAR
	irqcrossbar_init();
#endif
	irqchip_init();
}