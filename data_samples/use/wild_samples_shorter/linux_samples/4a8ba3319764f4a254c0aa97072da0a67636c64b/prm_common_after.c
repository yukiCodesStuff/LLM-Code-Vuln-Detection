 */
void omap_prcm_irq_cleanup(void)
{
	unsigned int irq;
	int i;

	if (!prcm_irq_setup) {
		pr_err("PRCM: IRQ handler not initialized; cannot cleanup\n");
	kfree(prcm_irq_setup->priority_mask);
	prcm_irq_setup->priority_mask = NULL;

	if (prcm_irq_setup->xlate_irq)
		irq = prcm_irq_setup->xlate_irq(prcm_irq_setup->irq);
	else
		irq = prcm_irq_setup->irq;
	irq_set_chained_handler(irq, NULL);

	if (prcm_irq_setup->base_irq > 0)
		irq_free_descs(prcm_irq_setup->base_irq,
			prcm_irq_setup->nr_regs * 32);
	int offset, i;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	unsigned int irq;

	if (!irq_setup)
		return -EINVAL;

				1 << (offset & 0x1f);
	}

	if (irq_setup->xlate_irq)
		irq = irq_setup->xlate_irq(irq_setup->irq);
	else
		irq = irq_setup->irq;
	irq_set_chained_handler(irq, omap_prcm_irq_handler);

	irq_setup->base_irq = irq_alloc_descs(-1, 0, irq_setup->nr_regs * 32,
		0);
