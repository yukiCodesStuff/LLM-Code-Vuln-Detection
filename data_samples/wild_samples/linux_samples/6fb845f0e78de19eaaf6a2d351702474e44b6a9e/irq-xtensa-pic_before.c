{
	xtensa_set_sr(1 << d->hwirq, intclear);
}

static int xtensa_irq_retrigger(struct irq_data *d)
{
	xtensa_set_sr(1 << d->hwirq, intset);
	return 1;
}