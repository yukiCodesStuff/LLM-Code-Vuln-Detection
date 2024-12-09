
static int xtensa_irq_retrigger(struct irq_data *d)
{
	xtensa_set_sr(1 << d->hwirq, intset);
	return 1;
}

static struct irq_chip xtensa_irq_chip = {