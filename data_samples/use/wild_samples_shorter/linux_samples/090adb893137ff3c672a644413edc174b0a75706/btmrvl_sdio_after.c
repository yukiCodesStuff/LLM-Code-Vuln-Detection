	.io_port_2 = 0x7a,
};

static const struct btmrvl_sdio_card_reg btmrvl_reg_88xx = {
	.cfg = 0x00,
	.host_int_mask = 0x02,
	.host_intstatus = 0x03,
	.card_status = 0x50,
	.sq_read_base_addr_a0 = 0x60,
	.sq_read_base_addr_a1 = 0x61,
	.card_revision = 0xbc,
	.card_fw_status0 = 0xc0,
	.card_fw_status1 = 0xc1,
	.card_rx_len = 0xc2,
	.card_rx_unit = 0xc3,
	.io_port_0 = 0xd8,
	.io_port_1 = 0xd9,
	.io_port_2 = 0xda,
};

static const struct btmrvl_sdio_device btmrvl_sdio_sd8688 = {
	.helper		= "mrvl/sd8688_helper.bin",
	.firmware	= "mrvl/sd8688.bin",
	.reg		= &btmrvl_reg_8688,
	.sd_blksz_fw_dl	= 256,
};

static const struct btmrvl_sdio_device btmrvl_sdio_sd8897 = {
	.helper		= NULL,
	.firmware	= "mrvl/sd8897_uapsta.bin",
	.reg		= &btmrvl_reg_88xx,
	.sd_blksz_fw_dl	= 256,
};

static const struct sdio_device_id btmrvl_sdio_ids[] = {
	/* Marvell SD8688 Bluetooth device */
	{ SDIO_DEVICE(SDIO_VENDOR_ID_MARVELL, 0x9105),
			.driver_data = (unsigned long) &btmrvl_sdio_sd8688 },
	/* Marvell SD8797 Bluetooth device */
	{ SDIO_DEVICE(SDIO_VENDOR_ID_MARVELL, 0x912A),
			.driver_data = (unsigned long) &btmrvl_sdio_sd8797 },
	/* Marvell SD8897 Bluetooth device */
	{ SDIO_DEVICE(SDIO_VENDOR_ID_MARVELL, 0x912E),
			.driver_data = (unsigned long) &btmrvl_sdio_sd8897 },

	{ }	/* Terminating entry */
};

MODULE_FIRMWARE("mrvl/sd8688.bin");
MODULE_FIRMWARE("mrvl/sd8787_uapsta.bin");
MODULE_FIRMWARE("mrvl/sd8797_uapsta.bin");
MODULE_FIRMWARE("mrvl/sd8897_uapsta.bin");