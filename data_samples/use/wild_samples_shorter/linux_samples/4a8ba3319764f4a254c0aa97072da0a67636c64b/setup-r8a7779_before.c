
void __init r8a7779_init_irq_dt(void)
{
	gic_arch_extn.irq_set_wake = r8a7779_set_wake;

	irqchip_init();

	/* route all interrupts to ARM */
	__raw_writel(0xffffffff, INT2NTSR0);
	__raw_writel(0x3fffffff, INT2NTSR1);
