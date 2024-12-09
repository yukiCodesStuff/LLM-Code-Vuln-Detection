	omap_register_i2c_bus(bus, clkrate, &pmic_i2c_board_info, 1);
}

#ifdef CONFIG_ARCH_OMAP4
void __init omap4_pmic_init(const char *pmic_type,
		    struct twl4030_platform_data *pmic_data,
		    struct i2c_board_info *devices, int nr_devices)
{
	/* PMIC part*/
	unsigned int irq;

	omap_mux_init_signal("sys_nirq1", OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);
	omap_mux_init_signal("fref_clk0_out.sys_drm_msecure", OMAP_PIN_OUTPUT);
	irq = omap4_xlate_irq(7 + OMAP44XX_IRQ_GIC_START);
	omap_pmic_init(1, 400, pmic_type, irq, pmic_data);

	/* Register additional devices on i2c1 bus if needed */
	if (devices)
		i2c_register_board_info(1, devices, nr_devices);
}
#endif

void __init omap_pmic_late_init(void)
{
	/* Init the OMAP TWL parameters (if PMIC has been registerd) */