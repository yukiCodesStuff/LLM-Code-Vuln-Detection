static struct renesas_intc_irqpin_config irqpin0_platform_data = {
	.irq_base = irq_pin(0), /* IRQ0 -> IRQ7 */
	.control_parent = true,
};

static struct resource irqpin0_resources[] = {
	DEFINE_RES_MEM(0xe6900000, 4), /* ICR1A */
	DEFINE_RES_MEM(0xe6900010, 4), /* INTPRI00A */
	DEFINE_RES_MEM(0xe6900020, 1), /* INTREQ00A */
	DEFINE_RES_MEM(0xe6900040, 1), /* INTMSK00A */
	DEFINE_RES_MEM(0xe6900060, 1), /* INTMSKCLR00A */
	DEFINE_RES_IRQ(gic_spi(1)), /* IRQ0 */
	DEFINE_RES_IRQ(gic_spi(2)), /* IRQ1 */
	DEFINE_RES_IRQ(gic_spi(3)), /* IRQ2 */
	DEFINE_RES_IRQ(gic_spi(4)), /* IRQ3 */
	DEFINE_RES_IRQ(gic_spi(5)), /* IRQ4 */
	DEFINE_RES_IRQ(gic_spi(6)), /* IRQ5 */
	DEFINE_RES_IRQ(gic_spi(7)), /* IRQ6 */
	DEFINE_RES_IRQ(gic_spi(8)), /* IRQ7 */
};

static struct platform_device irqpin0_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 0,
	.resource	= irqpin0_resources,
	.num_resources	= ARRAY_SIZE(irqpin0_resources),
	.dev		= {
		.platform_data	= &irqpin0_platform_data,
	},
};

static struct renesas_intc_irqpin_config irqpin1_platform_data = {
	.irq_base = irq_pin(8), /* IRQ8 -> IRQ15 */
	.control_parent = true, /* Disable spurious IRQ10 */
};

static struct resource irqpin1_resources[] = {
	DEFINE_RES_MEM(0xe6900004, 4), /* ICR2A */
	DEFINE_RES_MEM(0xe6900014, 4), /* INTPRI10A */
	DEFINE_RES_MEM(0xe6900024, 1), /* INTREQ10A */
	DEFINE_RES_MEM(0xe6900044, 1), /* INTMSK10A */
	DEFINE_RES_MEM(0xe6900064, 1), /* INTMSKCLR10A */
	DEFINE_RES_IRQ(gic_spi(9)), /* IRQ8 */
	DEFINE_RES_IRQ(gic_spi(10)), /* IRQ9 */
	DEFINE_RES_IRQ(gic_spi(11)), /* IRQ10 */
	DEFINE_RES_IRQ(gic_spi(12)), /* IRQ11 */
	DEFINE_RES_IRQ(gic_spi(13)), /* IRQ12 */
	DEFINE_RES_IRQ(gic_spi(14)), /* IRQ13 */
	DEFINE_RES_IRQ(gic_spi(15)), /* IRQ14 */
	DEFINE_RES_IRQ(gic_spi(16)), /* IRQ15 */
};

static struct platform_device irqpin1_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 1,
	.resource	= irqpin1_resources,
	.num_resources	= ARRAY_SIZE(irqpin1_resources),
	.dev		= {
		.platform_data	= &irqpin1_platform_data,
	},
};

static struct renesas_intc_irqpin_config irqpin2_platform_data = {
	.irq_base = irq_pin(16), /* IRQ16 -> IRQ23 */
	.control_parent = true,
};

static struct resource irqpin2_resources[] = {
	DEFINE_RES_MEM(0xe6900008, 4), /* ICR3A */
	DEFINE_RES_MEM(0xe6900018, 4), /* INTPRI20A */
	DEFINE_RES_MEM(0xe6900028, 1), /* INTREQ20A */
	DEFINE_RES_MEM(0xe6900048, 1), /* INTMSK20A */
	DEFINE_RES_MEM(0xe6900068, 1), /* INTMSKCLR20A */
	DEFINE_RES_IRQ(gic_spi(17)), /* IRQ16 */
	DEFINE_RES_IRQ(gic_spi(18)), /* IRQ17 */
	DEFINE_RES_IRQ(gic_spi(19)), /* IRQ18 */
	DEFINE_RES_IRQ(gic_spi(20)), /* IRQ19 */
	DEFINE_RES_IRQ(gic_spi(21)), /* IRQ20 */
	DEFINE_RES_IRQ(gic_spi(22)), /* IRQ21 */
	DEFINE_RES_IRQ(gic_spi(23)), /* IRQ22 */
	DEFINE_RES_IRQ(gic_spi(24)), /* IRQ23 */
};

static struct platform_device irqpin2_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 2,
	.resource	= irqpin2_resources,
	.num_resources	= ARRAY_SIZE(irqpin2_resources),
	.dev		= {
		.platform_data	= &irqpin2_platform_data,
	},
};

static struct renesas_intc_irqpin_config irqpin3_platform_data = {
	.irq_base = irq_pin(24), /* IRQ24 -> IRQ31 */
	.control_parent = true,
};

static struct resource irqpin3_resources[] = {
	DEFINE_RES_MEM(0xe690000c, 4), /* ICR4A */
	DEFINE_RES_MEM(0xe690001c, 4), /* INTPRI30A */
	DEFINE_RES_MEM(0xe690002c, 1), /* INTREQ30A */
	DEFINE_RES_MEM(0xe690004c, 1), /* INTMSK30A */
	DEFINE_RES_MEM(0xe690006c, 1), /* INTMSKCLR30A */
	DEFINE_RES_IRQ(gic_spi(25)), /* IRQ24 */
	DEFINE_RES_IRQ(gic_spi(26)), /* IRQ25 */
	DEFINE_RES_IRQ(gic_spi(27)), /* IRQ26 */
	DEFINE_RES_IRQ(gic_spi(28)), /* IRQ27 */
	DEFINE_RES_IRQ(gic_spi(29)), /* IRQ28 */
	DEFINE_RES_IRQ(gic_spi(30)), /* IRQ29 */
	DEFINE_RES_IRQ(gic_spi(31)), /* IRQ30 */
	DEFINE_RES_IRQ(gic_spi(32)), /* IRQ31 */
};

static struct platform_device irqpin3_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 3,
	.resource	= irqpin3_resources,
	.num_resources	= ARRAY_SIZE(irqpin3_resources),
	.dev		= {
		.platform_data	= &irqpin3_platform_data,
	},
};

static struct platform_device *sh73a0_early_devices[] __initdata = {
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
	&scif4_device,
	&scif5_device,
	&scif6_device,
	&scif7_device,
	&scif8_device,
	&tmu0_device,
	&ipmmu_device,
	&cmt1_device,
};

static struct platform_device *sh73a0_late_devices[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c3_device,
	&i2c4_device,
	&dma0_device,
	&mpdma0_device,
	&pmu_device,
	&irqpin0_device,
	&irqpin1_device,
	&irqpin2_device,
	&irqpin3_device,
};

#define SRCR2          IOMEM(0xe61580b0)

void __init sh73a0_add_standard_devices(void)
{
	/* Clear software reset bit on SY-DMAC module */
	__raw_writel(__raw_readl(SRCR2) & ~(1 << 18), SRCR2);

	platform_add_devices(sh73a0_early_devices,
			    ARRAY_SIZE(sh73a0_early_devices));
	platform_add_devices(sh73a0_late_devices,
			    ARRAY_SIZE(sh73a0_late_devices));
}

/* do nothing for !CONFIG_SMP or !CONFIG_HAVE_TWD */
void __init __weak sh73a0_register_twd(void) { }

void __init sh73a0_earlytimer_init(void)
{
	shmobile_init_delay();
	sh73a0_clock_init();
	shmobile_earlytimer_init();
	sh73a0_register_twd();
}

void __init sh73a0_add_early_devices(void)
{
	early_platform_add_devices(sh73a0_early_devices,
				   ARRAY_SIZE(sh73a0_early_devices));

	/* setup early console here as well */
	shmobile_setup_console();
}

#ifdef CONFIG_USE_OF

void __init sh73a0_add_standard_devices_dt(void)
{
	/* clocks are setup late during boot in the case of DT */
	sh73a0_clock_init();

	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

#define RESCNT2 IOMEM(0xe6188020)
static void sh73a0_restart(enum reboot_mode mode, const char *cmd)
{
	/* Do soft power on reset */
	writel((1 << 31), RESCNT2);
}

static const char *sh73a0_boards_compat_dt[] __initdata = {
	"renesas,sh73a0",
	NULL,
};

DT_MACHINE_START(SH73A0_DT, "Generic SH73A0 (Flattened Device Tree)")
	.smp		= smp_ops(sh73a0_smp_ops),
	.map_io		= sh73a0_map_io,
	.init_early	= shmobile_init_delay,
	.init_machine	= sh73a0_add_standard_devices_dt,
	.init_late	= shmobile_init_late,
	.restart	= sh73a0_restart,
	.dt_compat	= sh73a0_boards_compat_dt,
MACHINE_END
#endif /* CONFIG_USE_OF */
static struct renesas_intc_irqpin_config irqpin2_platform_data = {
	.irq_base = irq_pin(16), /* IRQ16 -> IRQ23 */
	.control_parent = true,
};

static struct resource irqpin2_resources[] = {
	DEFINE_RES_MEM(0xe6900008, 4), /* ICR3A */
	DEFINE_RES_MEM(0xe6900018, 4), /* INTPRI20A */
	DEFINE_RES_MEM(0xe6900028, 1), /* INTREQ20A */
	DEFINE_RES_MEM(0xe6900048, 1), /* INTMSK20A */
	DEFINE_RES_MEM(0xe6900068, 1), /* INTMSKCLR20A */
	DEFINE_RES_IRQ(gic_spi(17)), /* IRQ16 */
	DEFINE_RES_IRQ(gic_spi(18)), /* IRQ17 */
	DEFINE_RES_IRQ(gic_spi(19)), /* IRQ18 */
	DEFINE_RES_IRQ(gic_spi(20)), /* IRQ19 */
	DEFINE_RES_IRQ(gic_spi(21)), /* IRQ20 */
	DEFINE_RES_IRQ(gic_spi(22)), /* IRQ21 */
	DEFINE_RES_IRQ(gic_spi(23)), /* IRQ22 */
	DEFINE_RES_IRQ(gic_spi(24)), /* IRQ23 */
};

static struct platform_device irqpin2_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 2,
	.resource	= irqpin2_resources,
	.num_resources	= ARRAY_SIZE(irqpin2_resources),
	.dev		= {
		.platform_data	= &irqpin2_platform_data,
	},
};

static struct renesas_intc_irqpin_config irqpin3_platform_data = {
	.irq_base = irq_pin(24), /* IRQ24 -> IRQ31 */
	.control_parent = true,
};

static struct resource irqpin3_resources[] = {
	DEFINE_RES_MEM(0xe690000c, 4), /* ICR4A */
	DEFINE_RES_MEM(0xe690001c, 4), /* INTPRI30A */
	DEFINE_RES_MEM(0xe690002c, 1), /* INTREQ30A */
	DEFINE_RES_MEM(0xe690004c, 1), /* INTMSK30A */
	DEFINE_RES_MEM(0xe690006c, 1), /* INTMSKCLR30A */
	DEFINE_RES_IRQ(gic_spi(25)), /* IRQ24 */
	DEFINE_RES_IRQ(gic_spi(26)), /* IRQ25 */
	DEFINE_RES_IRQ(gic_spi(27)), /* IRQ26 */
	DEFINE_RES_IRQ(gic_spi(28)), /* IRQ27 */
	DEFINE_RES_IRQ(gic_spi(29)), /* IRQ28 */
	DEFINE_RES_IRQ(gic_spi(30)), /* IRQ29 */
	DEFINE_RES_IRQ(gic_spi(31)), /* IRQ30 */
	DEFINE_RES_IRQ(gic_spi(32)), /* IRQ31 */
};

static struct platform_device irqpin3_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 3,
	.resource	= irqpin3_resources,
	.num_resources	= ARRAY_SIZE(irqpin3_resources),
	.dev		= {
		.platform_data	= &irqpin3_platform_data,
	},
};

static struct platform_device *sh73a0_early_devices[] __initdata = {
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
	&scif4_device,
	&scif5_device,
	&scif6_device,
	&scif7_device,
	&scif8_device,
	&tmu0_device,
	&ipmmu_device,
	&cmt1_device,
};

static struct platform_device *sh73a0_late_devices[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c3_device,
	&i2c4_device,
	&dma0_device,
	&mpdma0_device,
	&pmu_device,
	&irqpin0_device,
	&irqpin1_device,
	&irqpin2_device,
	&irqpin3_device,
};

#define SRCR2          IOMEM(0xe61580b0)

void __init sh73a0_add_standard_devices(void)
{
	/* Clear software reset bit on SY-DMAC module */
	__raw_writel(__raw_readl(SRCR2) & ~(1 << 18), SRCR2);

	platform_add_devices(sh73a0_early_devices,
			    ARRAY_SIZE(sh73a0_early_devices));
	platform_add_devices(sh73a0_late_devices,
			    ARRAY_SIZE(sh73a0_late_devices));
}

/* do nothing for !CONFIG_SMP or !CONFIG_HAVE_TWD */
void __init __weak sh73a0_register_twd(void) { }

void __init sh73a0_earlytimer_init(void)
{
	shmobile_init_delay();
	sh73a0_clock_init();
	shmobile_earlytimer_init();
	sh73a0_register_twd();
}

void __init sh73a0_add_early_devices(void)
{
	early_platform_add_devices(sh73a0_early_devices,
				   ARRAY_SIZE(sh73a0_early_devices));

	/* setup early console here as well */
	shmobile_setup_console();
}

#ifdef CONFIG_USE_OF

void __init sh73a0_add_standard_devices_dt(void)
{
	/* clocks are setup late during boot in the case of DT */
	sh73a0_clock_init();

	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

#define RESCNT2 IOMEM(0xe6188020)
static void sh73a0_restart(enum reboot_mode mode, const char *cmd)
{
	/* Do soft power on reset */
	writel((1 << 31), RESCNT2);
}

static const char *sh73a0_boards_compat_dt[] __initdata = {
	"renesas,sh73a0",
	NULL,
};

DT_MACHINE_START(SH73A0_DT, "Generic SH73A0 (Flattened Device Tree)")
	.smp		= smp_ops(sh73a0_smp_ops),
	.map_io		= sh73a0_map_io,
	.init_early	= shmobile_init_delay,
	.init_machine	= sh73a0_add_standard_devices_dt,
	.init_late	= shmobile_init_late,
	.restart	= sh73a0_restart,
	.dt_compat	= sh73a0_boards_compat_dt,
MACHINE_END
#endif /* CONFIG_USE_OF */
static struct renesas_intc_irqpin_config irqpin3_platform_data = {
	.irq_base = irq_pin(24), /* IRQ24 -> IRQ31 */
	.control_parent = true,
};

static struct resource irqpin3_resources[] = {
	DEFINE_RES_MEM(0xe690000c, 4), /* ICR4A */
	DEFINE_RES_MEM(0xe690001c, 4), /* INTPRI30A */
	DEFINE_RES_MEM(0xe690002c, 1), /* INTREQ30A */
	DEFINE_RES_MEM(0xe690004c, 1), /* INTMSK30A */
	DEFINE_RES_MEM(0xe690006c, 1), /* INTMSKCLR30A */
	DEFINE_RES_IRQ(gic_spi(25)), /* IRQ24 */
	DEFINE_RES_IRQ(gic_spi(26)), /* IRQ25 */
	DEFINE_RES_IRQ(gic_spi(27)), /* IRQ26 */
	DEFINE_RES_IRQ(gic_spi(28)), /* IRQ27 */
	DEFINE_RES_IRQ(gic_spi(29)), /* IRQ28 */
	DEFINE_RES_IRQ(gic_spi(30)), /* IRQ29 */
	DEFINE_RES_IRQ(gic_spi(31)), /* IRQ30 */
	DEFINE_RES_IRQ(gic_spi(32)), /* IRQ31 */
};

static struct platform_device irqpin3_device = {
	.name		= "renesas_intc_irqpin",
	.id		= 3,
	.resource	= irqpin3_resources,
	.num_resources	= ARRAY_SIZE(irqpin3_resources),
	.dev		= {
		.platform_data	= &irqpin3_platform_data,
	},
};

static struct platform_device *sh73a0_early_devices[] __initdata = {
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
	&scif4_device,
	&scif5_device,
	&scif6_device,
	&scif7_device,
	&scif8_device,
	&tmu0_device,
	&ipmmu_device,
	&cmt1_device,
};

static struct platform_device *sh73a0_late_devices[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c3_device,
	&i2c4_device,
	&dma0_device,
	&mpdma0_device,
	&pmu_device,
	&irqpin0_device,
	&irqpin1_device,
	&irqpin2_device,
	&irqpin3_device,
};

#define SRCR2          IOMEM(0xe61580b0)

void __init sh73a0_add_standard_devices(void)
{
	/* Clear software reset bit on SY-DMAC module */
	__raw_writel(__raw_readl(SRCR2) & ~(1 << 18), SRCR2);

	platform_add_devices(sh73a0_early_devices,
			    ARRAY_SIZE(sh73a0_early_devices));
	platform_add_devices(sh73a0_late_devices,
			    ARRAY_SIZE(sh73a0_late_devices));
}

/* do nothing for !CONFIG_SMP or !CONFIG_HAVE_TWD */
void __init __weak sh73a0_register_twd(void) { }

void __init sh73a0_earlytimer_init(void)
{
	shmobile_init_delay();
	sh73a0_clock_init();
	shmobile_earlytimer_init();
	sh73a0_register_twd();
}

void __init sh73a0_add_early_devices(void)
{
	early_platform_add_devices(sh73a0_early_devices,
				   ARRAY_SIZE(sh73a0_early_devices));

	/* setup early console here as well */
	shmobile_setup_console();
}

#ifdef CONFIG_USE_OF

void __init sh73a0_add_standard_devices_dt(void)
{
	/* clocks are setup late during boot in the case of DT */
	sh73a0_clock_init();

	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

#define RESCNT2 IOMEM(0xe6188020)
static void sh73a0_restart(enum reboot_mode mode, const char *cmd)
{
	/* Do soft power on reset */
	writel((1 << 31), RESCNT2);
}

static const char *sh73a0_boards_compat_dt[] __initdata = {
	"renesas,sh73a0",
	NULL,
};

DT_MACHINE_START(SH73A0_DT, "Generic SH73A0 (Flattened Device Tree)")
	.smp		= smp_ops(sh73a0_smp_ops),
	.map_io		= sh73a0_map_io,
	.init_early	= shmobile_init_delay,
	.init_machine	= sh73a0_add_standard_devices_dt,
	.init_late	= shmobile_init_late,
	.restart	= sh73a0_restart,
	.dt_compat	= sh73a0_boards_compat_dt,
MACHINE_END
#endif /* CONFIG_USE_OF */