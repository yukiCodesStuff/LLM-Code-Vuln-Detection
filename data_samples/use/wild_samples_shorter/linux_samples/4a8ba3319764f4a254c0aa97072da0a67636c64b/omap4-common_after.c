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
