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

	/* unmask all known interrupts in INTCS2 */
	__raw_writel(0x08330773, base + INT2SMSKCR0);
	__raw_writel(0x00311110, base + INT2SMSKCR1);

	iounmap(base);
}