{
	int irq;

	vic_init(io_p2v(NETX_PA_VIC), 0, ~0, 0);

	for (irq = NETX_IRQ_HIF_CHAINED(0); irq <= NETX_IRQ_HIF_LAST; irq++) {
		irq_set_chip_and_handler(irq, &netx_hif_chip,
					 handle_level_irq);
		set_irq_flags(irq, IRQF_VALID);
	}