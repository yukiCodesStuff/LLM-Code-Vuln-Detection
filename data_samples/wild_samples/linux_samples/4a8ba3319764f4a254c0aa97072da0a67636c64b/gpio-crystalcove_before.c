		if (pending & BIT(gpio)) {
			virq = irq_find_mapping(cg->chip.irqdomain, gpio);
			generic_handle_irq(virq);
		}