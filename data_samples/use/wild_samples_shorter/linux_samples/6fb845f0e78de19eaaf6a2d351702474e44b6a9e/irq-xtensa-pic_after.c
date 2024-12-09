
static int xtensa_irq_retrigger(struct irq_data *d)
{
	unsigned int mask = 1u << d->hwirq;

	if (WARN_ON(mask & ~XCHAL_INTTYPE_MASK_SOFTWARE))
		return 0;
	xtensa_set_sr(mask, intset);
	return 1;
}

static struct irq_chip xtensa_irq_chip = {