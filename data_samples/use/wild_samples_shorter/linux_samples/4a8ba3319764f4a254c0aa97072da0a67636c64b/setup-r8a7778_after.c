void __init r8a7778_init_irq_dt(void)
{
	void __iomem *base = ioremap_nocache(0xfe700000, 0x00100000);
#ifdef CONFIG_ARCH_SHMOBILE_LEGACY
	void __iomem *gic_dist_base = ioremap_nocache(0xfe438000, 0x1000);
	void __iomem *gic_cpu_base = ioremap_nocache(0xfe430000, 0x1000);
#endif

	BUG_ON(!base);

#ifdef CONFIG_ARCH_SHMOBILE_LEGACY
	gic_init(0, 29, gic_dist_base, gic_cpu_base);
#else
	irqchip_init();
#endif
	/* route all interrupts to ARM */
	__raw_writel(0x73ffffff, base + INT2NTSR0);
	__raw_writel(0xffffffff, base + INT2NTSR1);
