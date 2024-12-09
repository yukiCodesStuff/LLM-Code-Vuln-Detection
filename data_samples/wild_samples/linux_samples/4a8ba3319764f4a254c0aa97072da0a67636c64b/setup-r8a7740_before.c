{
	void __iomem *intc_prio_base = ioremap_nocache(0xe6900010, 0x10);
	void __iomem *intc_msk_base = ioremap_nocache(0xe6900040, 0x10);
	void __iomem *pfc_inta_ctrl = ioremap_nocache(0xe605807c, 0x4);

	irqchip_init();

	/* route signals to GIC */
	iowrite32(0x0, pfc_inta_ctrl);

	/*
	 * To mask the shared interrupt to SPI 149 we must ensure to set
	 * PRIO *and* MASK. Else we run into IRQ floods when registering
	 * the intc_irqpin devices
	 */
	iowrite32(0x0, intc_prio_base + 0x0);
	iowrite32(0x0, intc_prio_base + 0x4);
	iowrite32(0x0, intc_prio_base + 0x8);
	iowrite32(0x0, intc_prio_base + 0xc);
	iowrite8(0xff, intc_msk_base + 0x0);
	iowrite8(0xff, intc_msk_base + 0x4);
	iowrite8(0xff, intc_msk_base + 0x8);
	iowrite8(0xff, intc_msk_base + 0xc);

	iounmap(intc_prio_base);
	iounmap(intc_msk_base);
	iounmap(pfc_inta_ctrl);
}

static void __init r8a7740_generic_init(void)
{
	r8a7740_meram_workaround();

#ifdef CONFIG_CACHE_L2X0
	/* Shared attribute override enable, 32K*8way */
	l2x0_init(IOMEM(0xf0002000), 0x00400000, 0xc20f0fff);
#endif
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

#define RESCNT2 IOMEM(0xe6188020)
static void r8a7740_restart(enum reboot_mode mode, const char *cmd)
{
	/* Do soft power on reset */
	writel(1 << 31, RESCNT2);
}

static const char *r8a7740_boards_compat_dt[] __initdata = {
	"renesas,r8a7740",
	NULL,
};

DT_MACHINE_START(R8A7740_DT, "Generic R8A7740 (Flattened Device Tree)")
	.map_io		= r8a7740_map_io,
	.init_early	= shmobile_init_delay,
	.init_irq	= r8a7740_init_irq_of,
	.init_machine	= r8a7740_generic_init,
	.init_late	= shmobile_init_late,
	.dt_compat	= r8a7740_boards_compat_dt,
	.restart	= r8a7740_restart,
MACHINE_END

#endif /* CONFIG_USE_OF */