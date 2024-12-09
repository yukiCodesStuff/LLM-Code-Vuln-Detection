static struct w1_gpio_platform_data w1_gpio_platform_data = {
	.pin			= GPIO_ONE_WIRE,
	.is_open_drain		= 0,
	.enable_external_pullup	= w1_enable_external_pullup,
};

struct platform_device raumfeld_w1_gpio_device = {
	.name	= "w1-gpio",
	.dev	= {
		.platform_data = &w1_gpio_platform_data
	}
};

static void __init raumfeld_w1_init(void)
{
	int ret = gpio_request(GPIO_W1_PULLUP_ENABLE,
				"W1 external pullup enable");

	if (ret < 0)
		pr_warning("Unable to request GPIO_W1_PULLUP_ENABLE\n");
	else
		gpio_direction_output(GPIO_W1_PULLUP_ENABLE, 0);

	platform_device_register(&raumfeld_w1_gpio_device);
}

/**
 * Framebuffer device
 */

/* PWM controlled backlight */
static struct platform_pwm_backlight_data raumfeld_pwm_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 100,
	.dft_brightness	= 100,
	/* 10000 ns = 10 ms ^= 100 kHz */
	.pwm_period_ns	= 10000,
};

static struct platform_device raumfeld_pwm_backlight_device = {
	.name	= "pwm-backlight",
	.dev	= {
		.parent		= &pxa27x_device_pwm0.dev,
		.platform_data	= &raumfeld_pwm_backlight_data,
	}
};

/* LT3593 controlled backlight */
static struct gpio_led raumfeld_lt3593_led = {
	.name		= "backlight",
	.gpio		= mfp_to_gpio(MFP_PIN_GPIO17),
	.default_state	= LEDS_GPIO_DEFSTATE_ON,
};

static struct gpio_led_platform_data raumfeld_lt3593_platform_data = {
	.leds		= &raumfeld_lt3593_led,
	.num_leds	= 1,
};

static struct platform_device raumfeld_lt3593_device = {
	.name	= "leds-lt3593",
	.id	= -1,
	.dev	= {
		.platform_data = &raumfeld_lt3593_platform_data,
	},
};

static struct pxafb_mode_info sharp_lq043t3dx02_mode = {
	.pixclock	= 111000,
	.xres		= 480,
	.yres		= 272,
	.bpp		= 16,
	.hsync_len	= 41,
	.left_margin	= 2,
	.right_margin	= 1,
	.vsync_len	= 10,
	.upper_margin	= 3,
	.lower_margin	= 1,
	.sync		= 0,
};

static struct pxafb_mach_info raumfeld_sharp_lcd_info = {
	.modes		= &sharp_lq043t3dx02_mode,
	.num_modes	= 1,
	.video_mem_size = 0x400000,
	.lcd_conn	= LCD_COLOR_TFT_16BPP | LCD_PCLK_EDGE_FALL,
#ifdef CONFIG_PXA3XX_GCU
	.acceleration_enabled = 1,
#endif
};

static void __init raumfeld_lcd_init(void)
{
	int ret;

	ret = gpio_request(GPIO_TFT_VA_EN, "display VA enable");
	if (ret < 0)
		pr_warning("Unable to request GPIO_TFT_VA_EN\n");
	else
		gpio_direction_output(GPIO_TFT_VA_EN, 1);

	msleep(100);

	ret = gpio_request(GPIO_DISPLAY_ENABLE, "display enable");
	if (ret < 0)
		pr_warning("Unable to request GPIO_DISPLAY_ENABLE\n");
	else
		gpio_direction_output(GPIO_DISPLAY_ENABLE, 1);

	/* Hardware revision 2 has the backlight regulator controlled
	 * by an LT3593, earlier and later devices use PWM for that. */
	if ((system_rev & 0xff) == 2) {
		platform_device_register(&raumfeld_lt3593_device);
	} else {
		mfp_cfg_t raumfeld_pwm_pin_config = GPIO17_PWM0_OUT;
		pxa3xx_mfp_config(&raumfeld_pwm_pin_config, 1);
		platform_device_register(&raumfeld_pwm_backlight_device);
	}

	pxa_set_fb_info(NULL, &raumfeld_sharp_lcd_info);
	platform_device_register(&pxa3xx_device_gcu);
}

/**
 * SPI devices
 */

struct spi_gpio_platform_data raumfeld_spi_platform_data = {
	.sck		= GPIO_SPI_CLK,
	.mosi		= GPIO_SPI_MOSI,
	.miso		= GPIO_SPI_MISO,
	.num_chipselect	= 3,
};

static struct platform_device raumfeld_spi_device = {
	.name	= "spi_gpio",
	.id	= 0,
	.dev 	= {
		.platform_data	= &raumfeld_spi_platform_data,
	}
};

static struct lis3lv02d_platform_data lis3_pdata = {
	.click_flags 	= LIS3_CLICK_SINGLE_X |
			  LIS3_CLICK_SINGLE_Y |
			  LIS3_CLICK_SINGLE_Z,
	.irq_cfg	= LIS3_IRQ1_CLICK | LIS3_IRQ2_CLICK,
	.wakeup_flags	= LIS3_WAKEUP_X_LO | LIS3_WAKEUP_X_HI |
			  LIS3_WAKEUP_Y_LO | LIS3_WAKEUP_Y_HI |
			  LIS3_WAKEUP_Z_LO | LIS3_WAKEUP_Z_HI,
	.wakeup_thresh	= 10,
	.click_thresh_x = 10,
	.click_thresh_y = 10,
	.click_thresh_z = 10,
};

#define SPI_AK4104	\
{			\
	.modalias	= "ak4104-codec",	\
	.max_speed_hz	= 10000,		\
	.bus_num	= 0,			\
	.chip_select	= 0,			\
	.controller_data = (void *) GPIO_SPDIF_CS,	\
}

#define SPI_LIS3	\
{			\
	.modalias	= "lis3lv02d_spi",	\
	.max_speed_hz	= 1000000,		\
	.bus_num	= 0,			\
	.chip_select	= 1,			\
	.controller_data = (void *) GPIO_ACCEL_CS,	\
	.platform_data	= &lis3_pdata,		\
	.irq		= PXA_GPIO_TO_IRQ(GPIO_ACCEL_IRQ),	\
}

#define SPI_DAC7512	\
{	\
	.modalias	= "dac7512",		\
	.max_speed_hz	= 1000000,		\
	.bus_num	= 0,			\
	.chip_select	= 2,			\
	.controller_data = (void *) GPIO_MCLK_DAC_CS,	\
}

static struct spi_board_info connector_spi_devices[] __initdata = {
	SPI_AK4104,
	SPI_DAC7512,
};

static struct spi_board_info speaker_spi_devices[] __initdata = {
	SPI_DAC7512,
};

static struct spi_board_info controller_spi_devices[] __initdata = {
	SPI_LIS3,
};

/**
 * MMC for Marvell Libertas 8688 via SDIO
 */

static int raumfeld_mci_init(struct device *dev, irq_handler_t isr, void *data)
{
	gpio_set_value(GPIO_W2W_RESET, 1);
	gpio_set_value(GPIO_W2W_PDN, 1);

	return 0;
}

static void raumfeld_mci_exit(struct device *dev, void *data)
{
	gpio_set_value(GPIO_W2W_RESET, 0);
	gpio_set_value(GPIO_W2W_PDN, 0);
}

static struct pxamci_platform_data raumfeld_mci_platform_data = {
	.init			= raumfeld_mci_init,
	.exit			= raumfeld_mci_exit,
	.detect_delay_ms	= 200,
	.gpio_card_detect	= -1,
	.gpio_card_ro		= -1,
	.gpio_power		= -1,
};

/*
 * External power / charge logic
 */

static int power_supply_init(struct device *dev)
{
	return 0;
}

static void power_supply_exit(struct device *dev)
{
}

static int raumfeld_is_ac_online(void)
{
	return !gpio_get_value(GPIO_CHARGE_DC_OK);
}

static int raumfeld_is_usb_online(void)
{
	return 0;
}

static char *raumfeld_power_supplicants[] = { "ds2760-battery.0" };

static void raumfeld_power_signal_charged(void)
{
	struct power_supply *psy =
		power_supply_get_by_name(raumfeld_power_supplicants[0]);

	if (psy)
		power_supply_set_battery_charged(psy);
}

static int raumfeld_power_resume(void)
{
	/* check if GPIO_CHARGE_DONE went low while we were sleeping */
	if (!gpio_get_value(GPIO_CHARGE_DONE))
		raumfeld_power_signal_charged();

	return 0;
}

static struct pda_power_pdata power_supply_info = {
	.init			= power_supply_init,
	.is_ac_online		= raumfeld_is_ac_online,
	.is_usb_online		= raumfeld_is_usb_online,
	.exit			= power_supply_exit,
	.supplied_to		= raumfeld_power_supplicants,
	.num_supplicants	= ARRAY_SIZE(raumfeld_power_supplicants),
	.resume			= raumfeld_power_resume,
};

static struct resource power_supply_resources[] = {
	{
		.name  = "ac",
		.flags = IORESOURCE_IRQ |
			 IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_LOWEDGE,
		.start = GPIO_CHARGE_DC_OK,
		.end   = GPIO_CHARGE_DC_OK,
	},
};

static irqreturn_t charge_done_irq(int irq, void *dev_id)
{
	raumfeld_power_signal_charged();
	return IRQ_HANDLED;
}

static struct platform_device raumfeld_power_supply = {
	.name = "pda-power",
	.id   = -1,
	.dev  = {
		.platform_data = &power_supply_info,
	},
	.resource      = power_supply_resources,
	.num_resources = ARRAY_SIZE(power_supply_resources),
};

static void __init raumfeld_power_init(void)
{
	int ret;

	/* Set PEN2 high to enable maximum charge current */
	ret = gpio_request(GPIO_CHRG_PEN2, "CHRG_PEN2");
	if (ret < 0)
		pr_warning("Unable to request GPIO_CHRG_PEN2\n");
	else
		gpio_direction_output(GPIO_CHRG_PEN2, 1);

	ret = gpio_request(GPIO_CHARGE_DC_OK, "CABLE_DC_OK");
	if (ret < 0)
		pr_warning("Unable to request GPIO_CHARGE_DC_OK\n");

	ret = gpio_request(GPIO_CHARGE_USB_SUSP, "CHARGE_USB_SUSP");
	if (ret < 0)
		pr_warning("Unable to request GPIO_CHARGE_USB_SUSP\n");
	else
		gpio_direction_output(GPIO_CHARGE_USB_SUSP, 0);

	power_supply_resources[0].start = gpio_to_irq(GPIO_CHARGE_DC_OK);
	power_supply_resources[0].end = gpio_to_irq(GPIO_CHARGE_DC_OK);

	ret = request_irq(gpio_to_irq(GPIO_CHARGE_DONE),
			&charge_done_irq, IORESOURCE_IRQ_LOWEDGE,
			"charge_done", NULL);

	if (ret < 0)
		printk(KERN_ERR "%s: unable to register irq %d\n", __func__,
			GPIO_CHARGE_DONE);
	else
		platform_device_register(&raumfeld_power_supply);
}

/* Fixed regulator for AUDIO_VA, 0-0048 maps to the cs4270 codec device */

static struct regulator_consumer_supply audio_va_consumer_supply =
	REGULATOR_SUPPLY("va", "0-0048");

struct regulator_init_data audio_va_initdata = {
	.consumer_supplies = &audio_va_consumer_supply,
	.num_consumer_supplies = 1,
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
};

static struct fixed_voltage_config audio_va_config = {
	.supply_name		= "audio_va",
	.microvolts		= 5000000,
	.gpio			= GPIO_AUDIO_VA_ENABLE,
	.enable_high		= 1,
	.enabled_at_boot	= 0,
	.init_data		= &audio_va_initdata,
};

static struct platform_device audio_va_device = {
	.name	= "reg-fixed-voltage",
	.id	= 0,
	.dev	= {
		.platform_data = &audio_va_config,
	},
};

/* Dummy supplies for Codec's VD/VLC */

static struct regulator_consumer_supply audio_dummy_supplies[] = {
	REGULATOR_SUPPLY("vd", "0-0048"),
	REGULATOR_SUPPLY("vlc", "0-0048"),
};

struct regulator_init_data audio_dummy_initdata = {
	.consumer_supplies = audio_dummy_supplies,
	.num_consumer_supplies = ARRAY_SIZE(audio_dummy_supplies),
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
};

static struct fixed_voltage_config audio_dummy_config = {
	.supply_name		= "audio_vd",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &audio_dummy_initdata,
};

static struct platform_device audio_supply_dummy_device = {
	.name	= "reg-fixed-voltage",
	.id	= 1,
	.dev	= {
		.platform_data = &audio_dummy_config,
	},
};

static struct platform_device *audio_regulator_devices[] = {
	&audio_va_device,
	&audio_supply_dummy_device,
};

/**
 * Regulator support via MAX8660
 */

static struct regulator_consumer_supply vcc_mmc_supply =
	REGULATOR_SUPPLY("vmmc", "pxa2xx-mci.0");

static struct regulator_init_data vcc_mmc_init_data = {
	.constraints = {
		.min_uV			= 3300000,
		.max_uV			= 3300000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL,
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS |
					  REGULATOR_CHANGE_VOLTAGE |
					  REGULATOR_CHANGE_MODE,
	},
	.consumer_supplies = &vcc_mmc_supply,
	.num_consumer_supplies = 1,
};

struct max8660_subdev_data max8660_v6_subdev_data = {
	.id		= MAX8660_V6,
	.name		= "vmmc",
	.platform_data	= &vcc_mmc_init_data,
};

static struct max8660_platform_data max8660_pdata = {
	.subdevs = &max8660_v6_subdev_data,
	.num_subdevs = 1,
};

/**
 * I2C devices
 */

static struct i2c_board_info raumfeld_pwri2c_board_info = {
	.type		= "max8660",
	.addr		= 0x34,
	.platform_data	= &max8660_pdata,
};

static struct i2c_board_info raumfeld_connector_i2c_board_info __initdata = {
	.type	= "cs4270",
	.addr	= 0x48,
};

static struct eeti_ts_platform_data eeti_ts_pdata = {
	.irq_active_high = 1,
	.irq_gpio = GPIO_TOUCH_IRQ,
};

static struct i2c_board_info raumfeld_controller_i2c_board_info __initdata = {
	.type	= "eeti_ts",
	.addr	= 0x0a,
	.platform_data = &eeti_ts_pdata,
};

static struct platform_device *raumfeld_common_devices[] = {
	&raumfeld_gpio_keys_device,
	&raumfeld_led_device,
	&raumfeld_spi_device,
};

static void __init raumfeld_audio_init(void)
{
	int ret;

	ret = gpio_request(GPIO_CODEC_RESET, "cs4270 reset");
	if (ret < 0)
		pr_warning("unable to request GPIO_CODEC_RESET\n");
	else
		gpio_direction_output(GPIO_CODEC_RESET, 1);

	ret = gpio_request(GPIO_SPDIF_RESET, "ak4104 s/pdif reset");
	if (ret < 0)
		pr_warning("unable to request GPIO_SPDIF_RESET\n");
	else
		gpio_direction_output(GPIO_SPDIF_RESET, 1);

	ret = gpio_request(GPIO_MCLK_RESET, "MCLK reset");
	if (ret < 0)
		pr_warning("unable to request GPIO_MCLK_RESET\n");
	else
		gpio_direction_output(GPIO_MCLK_RESET, 1);

	platform_add_devices(ARRAY_AND_SIZE(audio_regulator_devices));
}

static void __init raumfeld_common_init(void)
{
	int ret;

	/* The on/off button polarity has changed after revision 1 */
	if ((system_rev & 0xff) > 1) {
		int i;

		for (i = 0; i < ARRAY_SIZE(gpio_keys_button); i++)
			if (!strcmp(gpio_keys_button[i].desc, "on_off button"))
				gpio_keys_button[i].active_low = 1;
	}

	enable_irq_wake(IRQ_WAKEUP0);

	pxa3xx_set_nand_info(&raumfeld_nand_info);
	pxa3xx_set_i2c_power_info(NULL);
	pxa_set_ohci_info(&raumfeld_ohci_info);
	pxa_set_mci_info(&raumfeld_mci_platform_data);
	pxa_set_i2c_info(NULL);
	pxa_set_ffuart_info(NULL);

	ret = gpio_request(GPIO_W2W_RESET, "Wi2Wi reset");
	if (ret < 0)
		pr_warning("Unable to request GPIO_W2W_RESET\n");
	else
		gpio_direction_output(GPIO_W2W_RESET, 0);

	ret = gpio_request(GPIO_W2W_PDN, "Wi2Wi powerup");
	if (ret < 0)
		pr_warning("Unable to request GPIO_W2W_PDN\n");
	else
		gpio_direction_output(GPIO_W2W_PDN, 0);

	/* this can be used to switch off the device */
	ret = gpio_request(GPIO_SHUTDOWN_SUPPLY, "supply shutdown");
	if (ret < 0)
		pr_warning("Unable to request GPIO_SHUTDOWN_SUPPLY\n");
	else
		gpio_direction_output(GPIO_SHUTDOWN_SUPPLY, 0);

	platform_add_devices(ARRAY_AND_SIZE(raumfeld_common_devices));
	i2c_register_board_info(1, &raumfeld_pwri2c_board_info, 1);
}

static void __init raumfeld_controller_init(void)
{
	int ret;

	pxa3xx_mfp_config(ARRAY_AND_SIZE(raumfeld_controller_pin_config));
	platform_device_register(&rotary_encoder_device);
	spi_register_board_info(ARRAY_AND_SIZE(controller_spi_devices));
	i2c_register_board_info(0, &raumfeld_controller_i2c_board_info, 1);

	ret = gpio_request(GPIO_SHUTDOWN_BATT, "battery shutdown");
	if (ret < 0)
		pr_warning("Unable to request GPIO_SHUTDOWN_BATT\n");
	else
		gpio_direction_output(GPIO_SHUTDOWN_BATT, 0);

	raumfeld_common_init();
	raumfeld_power_init();
	raumfeld_lcd_init();
	raumfeld_w1_init();
}

static void __init raumfeld_connector_init(void)
{
	pxa3xx_mfp_config(ARRAY_AND_SIZE(raumfeld_connector_pin_config));
	spi_register_board_info(ARRAY_AND_SIZE(connector_spi_devices));
	i2c_register_board_info(0, &raumfeld_connector_i2c_board_info, 1);

	platform_device_register(&smc91x_device);

	raumfeld_audio_init();
	raumfeld_common_init();
}

static void __init raumfeld_speaker_init(void)
{
	pxa3xx_mfp_config(ARRAY_AND_SIZE(raumfeld_speaker_pin_config));
	spi_register_board_info(ARRAY_AND_SIZE(speaker_spi_devices));
	i2c_register_board_info(0, &raumfeld_connector_i2c_board_info, 1);

	platform_device_register(&smc91x_device);
	platform_device_register(&rotary_encoder_device);

	raumfeld_audio_init();
	raumfeld_common_init();
}

/* physical memory regions */
#define	RAUMFELD_SDRAM_BASE	0xa0000000	/* SDRAM region */

#ifdef CONFIG_MACH_RAUMFELD_RC
MACHINE_START(RAUMFELD_RC, "Raumfeld Controller")
	.atag_offset	= 0x100,
	.init_machine	= raumfeld_controller_init,
	.map_io		= pxa3xx_map_io,
	.nr_irqs	= PXA_NR_IRQS,
	.init_irq	= pxa3xx_init_irq,
	.handle_irq	= pxa3xx_handle_irq,
	.init_time	= pxa_timer_init,
	.restart	= pxa_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_RAUMFELD_CONNECTOR
MACHINE_START(RAUMFELD_CONNECTOR, "Raumfeld Connector")
	.atag_offset	= 0x100,
	.init_machine	= raumfeld_connector_init,
	.map_io		= pxa3xx_map_io,
	.nr_irqs	= PXA_NR_IRQS,
	.init_irq	= pxa3xx_init_irq,
	.handle_irq	= pxa3xx_handle_irq,
	.init_time	= pxa_timer_init,
	.restart	= pxa_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_RAUMFELD_SPEAKER
MACHINE_START(RAUMFELD_SPEAKER, "Raumfeld Speaker")
	.atag_offset	= 0x100,
	.init_machine	= raumfeld_speaker_init,
	.map_io		= pxa3xx_map_io,
	.nr_irqs	= PXA_NR_IRQS,
	.init_irq	= pxa3xx_init_irq,
	.handle_irq	= pxa3xx_handle_irq,
	.init_time	= pxa_timer_init,
	.restart	= pxa_restart,
MACHINE_END
#endif