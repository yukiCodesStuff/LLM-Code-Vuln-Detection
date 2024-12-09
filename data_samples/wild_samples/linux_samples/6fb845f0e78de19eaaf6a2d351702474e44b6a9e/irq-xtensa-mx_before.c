{
	unsigned int mask = 1u << d->hwirq;

	if (mask & (XCHAL_INTTYPE_MASK_EXTERN_EDGE |
				XCHAL_INTTYPE_MASK_EXTERN_LEVEL)) {
		set_er(1u << (xtensa_get_ext_irq_no(d->hwirq) -
					HW_IRQ_MX_BASE), MIENG);
	} else {
		mask = __this_cpu_read(cached_irq_mask) & ~mask;
		__this_cpu_write(cached_irq_mask, mask);
		xtensa_set_sr(mask, intenable);
	}
}
{
	xtensa_set_sr(1 << d->hwirq, intclear);
}

static int xtensa_mx_irq_retrigger(struct irq_data *d)
{
	xtensa_set_sr(1 << d->hwirq, intset);
	return 1;
}