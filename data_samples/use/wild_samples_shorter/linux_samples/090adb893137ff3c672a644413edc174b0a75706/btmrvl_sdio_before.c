	.io_port_2 = 0x7a,
};

static const struct btmrvl_sdio_device btmrvl_sdio_sd8688 = {
	.helper		= "mrvl/sd8688_helper.bin",
	.firmware	= "mrvl/sd8688.bin",
	.reg		= &btmrvl_reg_8688,
	.sd_blksz_fw_dl	= 256,
};

static const struct sdio_device_id btmrvl_sdio_ids[] = {
	/* Marvell SD8688 Bluetooth device */
	{ SDIO_DEVICE(SDIO_VENDOR_ID_MARVELL, 0x9105),
			.driver_data = (unsigned long) &btmrvl_sdio_sd8688 },
	/* Marvell SD8797 Bluetooth device */
	{ SDIO_DEVICE(SDIO_VENDOR_ID_MARVELL, 0x912A),
			.driver_data = (unsigned long) &btmrvl_sdio_sd8797 },

	{ }	/* Terminating entry */
};

MODULE_FIRMWARE("mrvl/sd8688.bin");
MODULE_FIRMWARE("mrvl/sd8787_uapsta.bin");
MODULE_FIRMWARE("mrvl/sd8797_uapsta.bin");