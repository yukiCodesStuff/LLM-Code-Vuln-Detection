#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/usb/musb.h>
#include <linux/platform_data/spi-omap2-mcspi.h>

#include <asm/mach-types.h>
	sdrc_params = nokia_get_sdram_timings();
	omap_sdrc_init(sdrc_params, sdrc_params);

	usb_musb_init(&musb_board_data);
	rx51_peripherals_init();

	/* Ensure SDRC pins are mux'd for self-refresh */