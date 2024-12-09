	NULL
};

static void __init imx25_timer_init(void)
{
	mx25_clocks_init_dt();
}

DT_MACHINE_START(IMX25_DT, "Freescale i.MX25 (Device Tree Support)")
	.map_io		= mx25_map_io,
	.init_early	= imx25_init_early,
	.init_irq	= mx25_init_irq,