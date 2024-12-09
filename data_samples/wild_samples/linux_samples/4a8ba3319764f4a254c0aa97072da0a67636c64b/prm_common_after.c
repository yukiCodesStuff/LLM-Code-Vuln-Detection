{
	int i;

	if (!prcm_irq_setup || !name)
		return -ENOENT;

	for (i = 0; i < prcm_irq_setup->nr_irqs; i++)
		if (!strcmp(prcm_irq_setup->irqs[i].name, name))
			return prcm_irq_setup->base_irq +
				prcm_irq_setup->irqs[i].offset;

	return -ENOENT;
}

/**
 * omap_prcm_irq_cleanup - reverses memory allocated and other steps
 * done by omap_prcm_register_chain_handler()
 *
 * No return value.
 */
void omap_prcm_irq_cleanup(void)
{
	unsigned int irq;
	int i;

	if (!prcm_irq_setup) {
		pr_err("PRCM: IRQ handler not initialized; cannot cleanup\n");
		return;
	}

	if (prcm_irq_chips) {
		for (i = 0; i < prcm_irq_setup->nr_regs; i++) {
			if (prcm_irq_chips[i])
				irq_remove_generic_chip(prcm_irq_chips[i],
					0xffffffff, 0, 0);
			prcm_irq_chips[i] = NULL;
		}
		kfree(prcm_irq_chips);
		prcm_irq_chips = NULL;
	}

	kfree(prcm_irq_setup->saved_mask);
	prcm_irq_setup->saved_mask = NULL;

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
	prcm_irq_setup->base_irq = 0;
}
		for (i = 0; i < prcm_irq_setup->nr_regs; i++) {
			if (prcm_irq_chips[i])
				irq_remove_generic_chip(prcm_irq_chips[i],
					0xffffffff, 0, 0);
			prcm_irq_chips[i] = NULL;
		}
		kfree(prcm_irq_chips);
		prcm_irq_chips = NULL;
	}

	kfree(prcm_irq_setup->saved_mask);
	prcm_irq_setup->saved_mask = NULL;

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
	prcm_irq_setup->base_irq = 0;
}

void omap_prcm_irq_prepare(void)
{
	prcm_irq_setup->suspended = true;
}

void omap_prcm_irq_complete(void)
{
	prcm_irq_setup->suspended = false;

	/* If we have not saved the masks, do not attempt to restore */
	if (!prcm_irq_setup->suspend_save_flag)
		return;

	prcm_irq_setup->suspend_save_flag = false;

	/*
	 * Re-enable all masked PRCM irq sources, this causes the PRCM
	 * interrupt to fire immediately if the events were masked
	 * previously in the chain handler
	 */
	prcm_irq_setup->restore_irqen(prcm_irq_setup->saved_mask);
}

/**
 * omap_prcm_register_chain_handler - initializes the prcm chained interrupt
 * handler based on provided parameters
 * @irq_setup: hardware data about the underlying PRM/PRCM
 *
 * Set up the PRCM chained interrupt handler on the PRCM IRQ.  Sets up
 * one generic IRQ chip per PRM interrupt status/enable register pair.
 * Returns 0 upon success, -EINVAL if called twice or if invalid
 * arguments are passed, or -ENOMEM on any other error.
 */
int omap_prcm_register_chain_handler(struct omap_prcm_irq_setup *irq_setup)
{
	int nr_regs;
	u32 mask[OMAP_PRCM_MAX_NR_PENDING_REG];
	int offset, i;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	unsigned int irq;

	if (!irq_setup)
		return -EINVAL;

	nr_regs = irq_setup->nr_regs;

	if (prcm_irq_setup) {
		pr_err("PRCM: already initialized; won't reinitialize\n");
		return -EINVAL;
	}
{
	int nr_regs;
	u32 mask[OMAP_PRCM_MAX_NR_PENDING_REG];
	int offset, i;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	unsigned int irq;

	if (!irq_setup)
		return -EINVAL;

	nr_regs = irq_setup->nr_regs;

	if (prcm_irq_setup) {
		pr_err("PRCM: already initialized; won't reinitialize\n");
		return -EINVAL;
	}
	for (i = 0; i < irq_setup->nr_irqs; i++) {
		offset = irq_setup->irqs[i].offset;
		mask[offset >> 5] |= 1 << (offset & 0x1f);
		if (irq_setup->irqs[i].priority)
			irq_setup->priority_mask[offset >> 5] |=
				1 << (offset & 0x1f);
	}

	if (irq_setup->xlate_irq)
		irq = irq_setup->xlate_irq(irq_setup->irq);
	else
		irq = irq_setup->irq;
	irq_set_chained_handler(irq, omap_prcm_irq_handler);

	irq_setup->base_irq = irq_alloc_descs(-1, 0, irq_setup->nr_regs * 32,
		0);

	if (irq_setup->base_irq < 0) {
		pr_err("PRCM: failed to allocate irq descs: %d\n",
			irq_setup->base_irq);
		goto err;
	}

	for (i = 0; i < irq_setup->nr_regs; i++) {
		gc = irq_alloc_generic_chip("PRCM", 1,
			irq_setup->base_irq + i * 32, prm_base,
			handle_level_irq);

		if (!gc) {
			pr_err("PRCM: failed to allocate generic chip\n");
			goto err;
		}