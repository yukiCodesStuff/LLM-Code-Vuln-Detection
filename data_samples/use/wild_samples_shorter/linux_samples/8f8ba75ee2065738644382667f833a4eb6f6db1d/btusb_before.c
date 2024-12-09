	{ USB_DEVICE(0x0a5c, 0x21e6) },
	{ USB_DEVICE(0x0a5c, 0x21e8) },
	{ USB_DEVICE(0x0a5c, 0x21f3) },
	{ USB_DEVICE(0x413c, 0x8197) },

	/* Foxconn - Hon Hai */
	{ USB_DEVICE(0x0489, 0xe033) },
	{ USB_DEVICE(0x13d3, 0x3362), .driver_info = BTUSB_ATH3012 },
	{ USB_DEVICE(0x0cf3, 0xe004), .driver_info = BTUSB_ATH3012 },
	{ USB_DEVICE(0x0930, 0x0219), .driver_info = BTUSB_ATH3012 },

	/* Atheros AR5BBU12 with sflash firmware */
	{ USB_DEVICE(0x0489, 0xe02c), .driver_info = BTUSB_IGNORE },
