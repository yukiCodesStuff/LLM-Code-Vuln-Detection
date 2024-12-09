	void __iomem *intc_msk_base = ioremap_nocache(0xe6900040, 0x10);
	void __iomem *pfc_inta_ctrl = ioremap_nocache(0xe605807c, 0x4);

#ifdef CONFIG_ARCH_SHMOBILE_LEGACY
	void __iomem *gic_dist_base = ioremap_nocache(0xc2800000, 0x1000);
	void __iomem *gic_cpu_base = ioremap_nocache(0xc2000000, 0x1000);

	gic_init(0, 29, gic_dist_base, gic_cpu_base);
#else
	irqchip_init();
#endif

	/* route signals to GIC */
	iowrite32(0x0, pfc_inta_ctrl);
