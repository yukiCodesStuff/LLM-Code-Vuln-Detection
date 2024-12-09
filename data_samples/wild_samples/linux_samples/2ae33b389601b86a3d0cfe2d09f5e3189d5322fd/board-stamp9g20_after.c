static struct w1_gpio_platform_data w1_gpio_pdata = {
	.pin		= AT91_PIN_PA29,
	.is_open_drain	= 1,
	.ext_pullup_enable_pin	= -EINVAL,
};

static struct platform_device w1_device = {
	.name			= "w1-gpio",
	.id			= -1,
	.dev.platform_data	= &w1_gpio_pdata,
};

void add_w1(void)
{
	at91_set_GPIO_periph(w1_gpio_pdata.pin, 1);
	at91_set_multi_drive(w1_gpio_pdata.pin, 1);
	platform_device_register(&w1_device);
}


void __init stamp9g20_board_init(void)
{
	/* Serial */
	/* DGBU on ttyS0. (Rx & Tx only) */
	at91_register_uart(0, 0, 0);
	at91_add_device_serial();
	/* NAND */
	add_device_nand();
	/* MMC */
	at91_add_device_mci(0, &mmc_data);
	/* W1 */
	add_w1();
}

static void __init portuxg20_board_init(void)
{
	/* USART0 on ttyS1. (Rx, Tx, CTS, RTS, DTR, DSR, DCD, RI) */
	at91_register_uart(AT91SAM9260_ID_US0, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
						| ATMEL_UART_DTR | ATMEL_UART_DSR
						| ATMEL_UART_DCD | ATMEL_UART_RI);

	/* USART1 on ttyS2. (Rx, Tx, CTS, RTS) */
	at91_register_uart(AT91SAM9260_ID_US1, 2, ATMEL_UART_CTS | ATMEL_UART_RTS);

	/* USART2 on ttyS3. (Rx, Tx, CTS, RTS) */
	at91_register_uart(AT91SAM9260_ID_US2, 3, ATMEL_UART_CTS | ATMEL_UART_RTS);

	/* USART4 on ttyS5. (Rx, Tx only) */
	at91_register_uart(AT91SAM9260_ID_US4, 5, 0);

	/* USART5 on ttyS6. (Rx, Tx only) */
	at91_register_uart(AT91SAM9260_ID_US5, 6, 0);
	stamp9g20_board_init();
	/* USB Host */
	at91_add_device_usbh(&usbh_data);
	/* USB Device */
	at91_add_device_udc(&portuxg20_udc_data);
	/* Ethernet */
	at91_add_device_eth(&macb_data);
	/* I2C */
	at91_add_device_i2c(NULL, 0);
	/* SPI */
	at91_add_device_spi(portuxg20_spi_devices, ARRAY_SIZE(portuxg20_spi_devices));
	/* LEDs */
	at91_gpio_leds(portuxg20_leds, ARRAY_SIZE(portuxg20_leds));
}

static void __init stamp9g20evb_board_init(void)
{
	/* USART0 on ttyS1. (Rx, Tx, CTS, RTS, DTR, DSR, DCD, RI) */
	at91_register_uart(AT91SAM9260_ID_US0, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
						| ATMEL_UART_DTR | ATMEL_UART_DSR
						| ATMEL_UART_DCD | ATMEL_UART_RI);
	stamp9g20_board_init();
	/* USB Host */
	at91_add_device_usbh(&usbh_data);
	/* USB Device */
	at91_add_device_udc(&stamp9g20evb_udc_data);
	/* Ethernet */
	at91_add_device_eth(&macb_data);
	/* I2C */
	at91_add_device_i2c(NULL, 0);
	/* LEDs */
	at91_gpio_leds(stamp9g20evb_leds, ARRAY_SIZE(stamp9g20evb_leds));
}

MACHINE_START(PORTUXG20, "taskit PortuxG20")
	/* Maintainer: taskit GmbH */
	.init_time	= at91sam926x_pit_init,
	.map_io		= at91_map_io,
	.handle_irq	= at91_aic_handle_irq,
	.init_early	= stamp9g20_init_early,
	.init_irq	= at91_init_irq_default,
	.init_machine	= portuxg20_board_init,
MACHINE_END

MACHINE_START(STAMP9G20, "taskit Stamp9G20")
	/* Maintainer: taskit GmbH */
	.init_time	= at91sam926x_pit_init,
	.map_io		= at91_map_io,
	.handle_irq	= at91_aic_handle_irq,
	.init_early	= stamp9g20_init_early,
	.init_irq	= at91_init_irq_default,
	.init_machine	= stamp9g20evb_board_init,
MACHINE_END