static struct w1_gpio_platform_data vulcan_w1_gpio_pdata = {
	.pin			= 14,
};

static struct platform_device vulcan_w1_gpio = {
	.name			= "w1-gpio",
	.id			= 0,
	.dev			= {
		.platform_data	= &vulcan_w1_gpio_pdata,
	},
};

static struct platform_device *vulcan_devices[] __initdata = {
	&vulcan_uart,
	&vulcan_flash,
	&vulcan_sram,
	&vulcan_max6369,
	&vulcan_eth[0],
	&vulcan_eth[1],
	&vulcan_w1_gpio,
};

static void __init vulcan_init(void)
{
	ixp4xx_sys_init();

	/* Flash is spread over both CS0 and CS1 */
	vulcan_flash_resource.start	 = IXP4XX_EXP_BUS_BASE(0);
	vulcan_flash_resource.end	 = IXP4XX_EXP_BUS_BASE(0) + SZ_32M - 1;
	*IXP4XX_EXP_CS0 = IXP4XX_EXP_BUS_CS_EN		|
			  IXP4XX_EXP_BUS_STROBE_T(3)	|
			  IXP4XX_EXP_BUS_SIZE(0xF)	|
			  IXP4XX_EXP_BUS_BYTE_RD16	|
			  IXP4XX_EXP_BUS_WR_EN;
	*IXP4XX_EXP_CS1 = *IXP4XX_EXP_CS0;

	/* SRAM on CS2, (256kB, 8bit, writable) */
	vulcan_sram_resource.start	= IXP4XX_EXP_BUS_BASE(2);
	vulcan_sram_resource.end	= IXP4XX_EXP_BUS_BASE(2) + SZ_256K - 1;
	*IXP4XX_EXP_CS2 = IXP4XX_EXP_BUS_CS_EN		|
			  IXP4XX_EXP_BUS_STROBE_T(1)	|
			  IXP4XX_EXP_BUS_HOLD_T(2)	|
			  IXP4XX_EXP_BUS_SIZE(9)	|
			  IXP4XX_EXP_BUS_SPLT_EN	|
			  IXP4XX_EXP_BUS_WR_EN		|
			  IXP4XX_EXP_BUS_BYTE_EN;

	/* XR16L2551 on CS3 (Moto style, 512 bytes, 8bits, writable) */
	vulcan_uart_resources[2].start	= IXP4XX_EXP_BUS_BASE(3);
	vulcan_uart_resources[2].end	= IXP4XX_EXP_BUS_BASE(3) + 16 - 1;
	vulcan_uart_data[2].mapbase	= vulcan_uart_resources[2].start;
	vulcan_uart_data[3].mapbase	= vulcan_uart_data[2].mapbase + 8;
	*IXP4XX_EXP_CS3 = IXP4XX_EXP_BUS_CS_EN		|
			  IXP4XX_EXP_BUS_STROBE_T(3)	|
			  IXP4XX_EXP_BUS_CYCLES(IXP4XX_EXP_BUS_CYCLES_MOTOROLA)|
			  IXP4XX_EXP_BUS_WR_EN		|
			  IXP4XX_EXP_BUS_BYTE_EN;

	/* GPIOS on CS4 (512 bytes, 8bits, writable) */
	*IXP4XX_EXP_CS4 = IXP4XX_EXP_BUS_CS_EN		|
			  IXP4XX_EXP_BUS_WR_EN		|
			  IXP4XX_EXP_BUS_BYTE_EN;

	/* max6369 on CS5 (512 bytes, 8bits, writable) */
	vulcan_max6369_resource.start	= IXP4XX_EXP_BUS_BASE(5);
	vulcan_max6369_resource.end	= IXP4XX_EXP_BUS_BASE(5);
	*IXP4XX_EXP_CS5 = IXP4XX_EXP_BUS_CS_EN		|
			  IXP4XX_EXP_BUS_WR_EN		|
			  IXP4XX_EXP_BUS_BYTE_EN;

	platform_add_devices(vulcan_devices, ARRAY_SIZE(vulcan_devices));
}

MACHINE_START(ARCOM_VULCAN, "Arcom/Eurotech Vulcan")
	/* Maintainer: Marc Zyngier <maz@misterjones.org> */
	.map_io		= ixp4xx_map_io,
	.init_early	= ixp4xx_init_early,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.atag_offset	= 0x100,
	.init_machine	= vulcan_init,
#if defined(CONFIG_PCI)
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END