#endif

#ifdef CONFIG_ARCH_OMAP3
/* Some boards need board name for legacy userspace in /proc/cpuinfo */
static const char *const n900_boards_compat[] __initconst = {
	"nokia,omap3-n900",
	NULL,
};

DT_MACHINE_START(OMAP3_N900_DT, "Nokia RX-51 board")
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap3430_init_early,
	.init_machine	= omap_generic_init,
	.init_late	= omap3_init_late,
	.init_time	= omap3_sync32k_timer_init,
	.dt_compat	= n900_boards_compat,
	.restart	= omap3xxx_restart,
MACHINE_END

/* Generic omap3 boards, most boards can use these */
static const char *const omap3_boards_compat[] __initconst = {
	"ti,omap3430",
	"ti,omap3",
	NULL,