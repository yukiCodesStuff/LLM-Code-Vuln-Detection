
void __init r8a7779_init_irq_dt(void)
{
#ifdef CONFIG_ARCH_SHMOBILE_LEGACY
	void __iomem *gic_dist_base = ioremap_nocache(0xf0001000, 0x1000);
	void __iomem *gic_cpu_base = ioremap_nocache(0xf0000100, 0x1000);
#endif
	gic_arch_extn.irq_set_wake = r8a7779_set_wake;

#ifdef CONFIG_ARCH_SHMOBILE_LEGACY
	gic_init(0, 29, gic_dist_base, gic_cpu_base);
#else
	irqchip_init();
#endif
	/* route all interrupts to ARM */
	__raw_writel(0xffffffff, INT2NTSR0);
	__raw_writel(0x3fffffff, INT2NTSR1);
