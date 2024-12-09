{
	unsigned int mask = 1u << d->hwirq;

	if (mask & (XCHAL_INTTYPE_MASK_EXTERN_EDGE |
		    XCHAL_INTTYPE_MASK_EXTERN_LEVEL)) {
		unsigned int ext_irq = xtensa_get_ext_irq_no(d->hwirq);

		if (ext_irq >= HW_IRQ_MX_BASE) {
			set_er(1u << (ext_irq - HW_IRQ_MX_BASE), MIENG);
			return;
		}
	}
{
	xtensa_set_sr(1 << d->hwirq, intclear);
}

static int xtensa_mx_irq_retrigger(struct irq_data *d)
{
	unsigned int mask = 1u << d->hwirq;

	if (WARN_ON(mask & ~XCHAL_INTTYPE_MASK_SOFTWARE))
		return 0;
	xtensa_set_sr(mask, intset);
	return 1;
}