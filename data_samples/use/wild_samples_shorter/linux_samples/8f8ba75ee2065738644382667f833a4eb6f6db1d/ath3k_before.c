	{ USB_DEVICE(0x13d3, 0x3362) },
	{ USB_DEVICE(0x0CF3, 0xE004) },
	{ USB_DEVICE(0x0930, 0x0219) },

	/* Atheros AR5BBU12 with sflash firmware */
	{ USB_DEVICE(0x0489, 0xE02C) },

	{ USB_DEVICE(0x13d3, 0x3362), .driver_info = BTUSB_ATH3012 },
	{ USB_DEVICE(0x0cf3, 0xe004), .driver_info = BTUSB_ATH3012 },
	{ USB_DEVICE(0x0930, 0x0219), .driver_info = BTUSB_ATH3012 },

	/* Atheros AR5BBU22 with sflash firmware */
	{ USB_DEVICE(0x0489, 0xE03C), .driver_info = BTUSB_ATH3012 },
