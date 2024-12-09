	.xlate = irq_domain_xlate_onecell,
};

void __init icoll_of_init(struct device_node *np,
			  struct device_node *interrupt_parent)
{
	/*
	 * Interrupt Collector reset, which initializes the priority