static struct irq_domain_ops icoll_irq_domain_ops = {
	.map = icoll_irq_domain_map,
	.xlate = irq_domain_xlate_onecell,
};

void __init icoll_of_init(struct device_node *np,
			  struct device_node *interrupt_parent)
{
	/*
	 * Interrupt Collector reset, which initializes the priority
	 * for each irq to level 0.
	 */
	mxs_reset_block(icoll_base + HW_ICOLL_CTRL);

	icoll_domain = irq_domain_add_linear(np, ICOLL_NUM_IRQS,
					     &icoll_irq_domain_ops, NULL);
	WARN_ON(!icoll_domain);
}