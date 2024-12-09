		if (pending & BIT(gpio)) {
			virq = irq_find_mapping(cg->chip.irqdomain, gpio);
			handle_nested_irq(virq);
		}