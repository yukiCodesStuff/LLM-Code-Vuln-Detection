	.init_irq	= omap_intc_of_init,
	.handle_irq	= omap3_intc_handle_irq,
	.init_machine	= omap_generic_init,
	.init_time	= omap3_sync32k_timer_init,
	.dt_compat	= omap3_boards_compat,
	.restart	= omap3xxx_restart,
MACHINE_END
	.init_irq	= omap_intc_of_init,
	.handle_irq	= omap3_intc_handle_irq,
	.init_machine	= omap_generic_init,
	.init_time	= omap3_secure_sync32k_timer_init,
	.dt_compat	= omap3_gp_boards_compat,
	.restart	= omap3xxx_restart,
MACHINE_END