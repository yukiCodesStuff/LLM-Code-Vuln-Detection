
	if (irq == -1) {
		irq = xen_allocate_irq_dynamic();
		if (irq == -1)
			goto out;

		irq_set_chip_and_handler_name(irq, &xen_dynamic_chip,
					      handle_edge_irq, "event");

	if (irq == -1) {
		irq = xen_allocate_irq_dynamic();
		if (irq == -1)
			goto out;

		irq_set_chip_and_handler_name(irq, &xen_percpu_chip,
					      handle_percpu_irq, "virq");