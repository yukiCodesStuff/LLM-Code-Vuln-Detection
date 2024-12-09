void __init r8a7778_init_irq_dt(void)
{
	void __iomem *base = ioremap_nocache(0xfe700000, 0x00100000);

	BUG_ON(!base);

	irqchip_init();

	/* route all interrupts to ARM */
	__raw_writel(0x73ffffff, base + INT2NTSR0);
	__raw_writel(0xffffffff, base + INT2NTSR1);
