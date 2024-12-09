 */
void omap_prcm_irq_cleanup(void)
{
	int i;

	if (!prcm_irq_setup) {
		pr_err("PRCM: IRQ handler not initialized; cannot cleanup\n");
	kfree(prcm_irq_setup->priority_mask);
	prcm_irq_setup->priority_mask = NULL;

	irq_set_chained_handler(prcm_irq_setup->irq, NULL);

	if (prcm_irq_setup->base_irq > 0)
		irq_free_descs(prcm_irq_setup->base_irq,
			prcm_irq_setup->nr_regs * 32);
	int offset, i;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;

	if (!irq_setup)
		return -EINVAL;

				1 << (offset & 0x1f);
	}

	irq_set_chained_handler(irq_setup->irq, omap_prcm_irq_handler);

	irq_setup->base_irq = irq_alloc_descs(-1, 0, irq_setup->nr_regs * 32,
		0);
