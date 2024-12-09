static const char * const imx25_dt_board_compat[] __initconst = {
	"fsl,imx25",
	NULL
};

DT_MACHINE_START(IMX25_DT, "Freescale i.MX25 (Device Tree Support)")
	.map_io		= mx25_map_io,
	.init_early	= imx25_init_early,
	.init_irq	= mx25_init_irq,
	.handle_irq	= imx25_handle_irq,
	.init_time	= imx25_timer_init,
	.init_machine	= imx25_dt_init,
	.dt_compat	= imx25_dt_board_compat,
	.restart	= mxc_restart,
MACHINE_END