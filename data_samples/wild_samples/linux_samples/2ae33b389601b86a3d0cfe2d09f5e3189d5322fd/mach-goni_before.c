	{
		.mux_id		= 0,
		.flags		= V4L2_MBUS_PCLK_SAMPLE_FALLING |
				  V4L2_MBUS_VSYNC_ACTIVE_LOW,
		.bus_type	= FIMC_BUS_TYPE_ITU_601,
		.board_info	= &noon010pc30_board_info,
		.i2c_bus_num	= 0,
		.clk_frequency	= 16000000UL,
	},
};

static struct s5p_platform_fimc goni_fimc_md_platdata __initdata = {
	.source_info	= goni_camera_sensors,
	.num_clients	= ARRAY_SIZE(goni_camera_sensors),
};

/* Audio device */
static struct platform_device goni_device_audio = {
	.name = "smdk-audio",
	.id = -1,
};

static struct platform_device *goni_devices[] __initdata = {
	&s3c_device_fb,
	&s5p_device_onenand,
	&goni_spi_gpio,
	&goni_i2c_gpio_pmic,
	&goni_i2c_gpio5,
	&goni_device_audio,
	&mmc2_fixed_voltage,
	&goni_device_gpiokeys,
	&s5p_device_mfc,
	&s5p_device_mfc_l,
	&s5p_device_mfc_r,
	&s5p_device_mixer,
	&s5p_device_sdo,
	&s3c_device_i2c0,
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc_md,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s5pv210_device_iis0,
	&s3c_device_usb_hsotg,
	&samsung_device_keypad,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&wm8994_fixed_voltage0,
	&wm8994_fixed_voltage1,
};

static void __init goni_sound_init(void)
{
	/* Ths main clock of WM8994 codec uses the output of CLKOUT pin.
	 * The CLKOUT[9:8] set to 0x3(XUSBXTI) of 0xE010E000(OTHERS)
	 * because it needs 24MHz clock to operate WM8994 codec.
	 */
	__raw_writel(__raw_readl(S5P_OTHERS) | (0x3 << 8), S5P_OTHERS);
}

static void __init goni_map_io(void)
{
	s5pv210_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(goni_uartcfgs, ARRAY_SIZE(goni_uartcfgs));
	s5p_set_timer_source(S5P_PWM3, S5P_PWM4);
}

static void __init goni_reserve(void)
{
	s5p_mfc_reserve_mem(0x43000000, 8 << 20, 0x51000000, 8 << 20);
}

static void __init goni_machine_init(void)
{
	/* Radio: call before I2C 1 registeration */
	goni_radio_init();

	/* I2C0 */
	s3c_i2c0_set_platdata(NULL);

	/* I2C1 */
	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, i2c1_devs, ARRAY_SIZE(i2c1_devs));

	/* TSP: call before I2C 2 registeration */
	goni_tsp_init();

	/* I2C2 */
	s3c_i2c2_set_platdata(&i2c2_data);
	i2c_register_board_info(2, i2c2_devs, ARRAY_SIZE(i2c2_devs));

	/* PMIC */
	goni_pmic_init();
	i2c_register_board_info(AP_I2C_GPIO_PMIC_BUS_4, i2c_gpio_pmic_devs,
			ARRAY_SIZE(i2c_gpio_pmic_devs));
	/* SDHCI */
	goni_setup_sdhci();

	/* SOUND */
	goni_sound_init();
	i2c_register_board_info(AP_I2C_GPIO_BUS_5, i2c_gpio5_devs,
			ARRAY_SIZE(i2c_gpio5_devs));

	/* FB */
	s3c_fb_set_platdata(&goni_lcd_pdata);

	/* FIMC */
	s3c_set_platdata(&goni_fimc_md_platdata, sizeof(goni_fimc_md_platdata),
			 &s5p_device_fimc_md);

	s3c_hsotg_set_platdata(&goni_hsotg_pdata);

	goni_camera_init();

	/* SPI */
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));

	/* KEYPAD */
	samsung_keypad_set_platdata(&keypad_data);

	platform_add_devices(goni_devices, ARRAY_SIZE(goni_devices));
}

MACHINE_START(GONI, "GONI")
	/* Maintainers: Kyungmin Park <kyungmin.park@samsung.com> */
	.atag_offset	= 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io		= goni_map_io,
	.init_machine	= goni_machine_init,
	.init_time	= s5p_timer_init,
	.reserve	= &goni_reserve,
	.restart	= s5pv210_restart,
MACHINE_END