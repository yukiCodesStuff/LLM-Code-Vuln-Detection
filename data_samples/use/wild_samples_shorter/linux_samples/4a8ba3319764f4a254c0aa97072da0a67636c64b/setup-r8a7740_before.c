	void __iomem *intc_msk_base = ioremap_nocache(0xe6900040, 0x10);
	void __iomem *pfc_inta_ctrl = ioremap_nocache(0xe605807c, 0x4);

	irqchip_init();

	/* route signals to GIC */
	iowrite32(0x0, pfc_inta_ctrl);
