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

static struct of_device_id gic_match[] = {
	{ .compatible = "arm,cortex-a9-gic", },
	{ .compatible = "arm,cortex-a15-gic", },
	{ },
};

static struct device_node *gic_node;

unsigned int omap4_xlate_irq(unsigned int hwirq)
{
	struct of_phandle_args irq_data;
	unsigned int irq;

	if (!gic_node)
		gic_node = of_find_matching_node(NULL, gic_match);

	if (WARN_ON(!gic_node))
		return hwirq;

	irq_data.np = gic_node;
	irq_data.args_count = 3;
	irq_data.args[0] = 0;
	irq_data.args[1] = hwirq - OMAP44XX_IRQ_GIC_START;
	irq_data.args[2] = IRQ_TYPE_LEVEL_HIGH;

	irq = irq_create_of_mapping(&irq_data);
	if (WARN_ON(!irq))
		irq = hwirq;

	return irq;
}

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