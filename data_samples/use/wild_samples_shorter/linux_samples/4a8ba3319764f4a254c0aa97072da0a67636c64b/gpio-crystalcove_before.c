	for (gpio = 0; gpio < CRYSTALCOVE_GPIO_NUM; gpio++) {
		if (pending & BIT(gpio)) {
			virq = irq_find_mapping(cg->chip.irqdomain, gpio);
			generic_handle_irq(virq);
		}
	}

	return IRQ_HANDLED;