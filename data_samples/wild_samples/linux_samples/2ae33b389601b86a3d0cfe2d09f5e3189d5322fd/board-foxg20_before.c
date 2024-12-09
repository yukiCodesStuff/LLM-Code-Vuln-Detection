static struct w1_gpio_platform_data w1_gpio_pdata = {
	/* If you choose to use a pin other than PB16 it needs to be 3.3V */
	.pin		= AT91_PIN_PB16,
	.is_open_drain  = 1,
};

static struct platform_device w1_device = {
	.name			= "w1-gpio",
	.id			= -1,
	.dev.platform_data	= &w1_gpio_pdata,
};

static void __init at91_add_device_w1(void)
{
	at91_set_GPIO_periph(w1_gpio_pdata.pin, 1);
	at91_set_multi_drive(w1_gpio_pdata.pin, 1);
	platform_device_register(&w1_device);
}

#endif


static struct i2c_board_info __initdata foxg20_i2c_devices[] = {
	{
		I2C_BOARD_INFO("24c512", 0x50),
	},
};


static void __init foxg20_board_init(void)
{
	/* Serial */
	/* DBGU on ttyS0. (Rx & Tx only) */
	at91_register_uart(0, 0, 0);

	/* USART0 on ttyS1. (Rx, Tx, CTS, RTS, DTR, DSR, DCD, RI) */
	at91_register_uart(AT91SAM9260_ID_US0, 1,
				ATMEL_UART_CTS
				| ATMEL_UART_RTS
				| ATMEL_UART_DTR
				| ATMEL_UART_DSR
				| ATMEL_UART_DCD
				| ATMEL_UART_RI);

	/* USART1 on ttyS2. (Rx, Tx, RTS, CTS) */
	at91_register_uart(AT91SAM9260_ID_US1, 2,
		ATMEL_UART_CTS
		| ATMEL_UART_RTS);

	/* USART2 on ttyS3. (Rx & Tx only) */
	at91_register_uart(AT91SAM9260_ID_US2, 3, 0);

	/* USART3 on ttyS4. (Rx, Tx, RTS, CTS) */
	at91_register_uart(AT91SAM9260_ID_US3, 4,
		ATMEL_UART_CTS
		| ATMEL_UART_RTS);

	/* USART4 on ttyS5. (Rx & Tx only) */
	at91_register_uart(AT91SAM9260_ID_US4, 5, 0);

	/* USART5 on ttyS6. (Rx & Tx only) */
	at91_register_uart(AT91SAM9260_ID_US5, 6, 0);

	/* Set the internal pull-up resistor on DRXD */
	at91_set_A_periph(AT91_PIN_PB14, 1);
	at91_add_device_serial();
	/* USB Host */
	at91_add_device_usbh(&foxg20_usbh_data);
	/* USB Device */
	at91_add_device_udc(&foxg20_udc_data);
	/* SPI */
	at91_add_device_spi(foxg20_spi_devices, ARRAY_SIZE(foxg20_spi_devices));
	/* Ethernet */
	at91_add_device_eth(&foxg20_macb_data);
	/* MMC */
	at91_add_device_mci(0, &foxg20_mci0_data);
	/* I2C */
	at91_add_device_i2c(foxg20_i2c_devices, ARRAY_SIZE(foxg20_i2c_devices));
	/* LEDs */
	at91_gpio_leds(foxg20_leds, ARRAY_SIZE(foxg20_leds));
	/* Push Buttons */
	foxg20_add_device_buttons();
#if defined(CONFIG_W1_MASTER_GPIO) || defined(CONFIG_W1_MASTER_GPIO_MODULE)
	at91_add_device_w1();
#endif
}

MACHINE_START(ACMENETUSFOXG20, "Acme Systems srl FOX Board G20")
	/* Maintainer: Sergio Tanzilli */
	.init_time	= at91sam926x_pit_init,
	.map_io		= at91_map_io,
	.handle_irq	= at91_aic_handle_irq,
	.init_early	= foxg20_init_early,
	.init_irq	= at91_init_irq_default,
	.init_machine	= foxg20_board_init,
MACHINE_END