	if (rsi_usb_init_rx(adapter)) {
		rsi_dbg(ERR_ZONE, "Failed to init RX handle\n");
		status = -ENOMEM;
		goto fail_rx;
	}

	rsi_dev->tx_blk_size = 252;
	adapter->block_size = rsi_dev->tx_blk_size;

	/* Initializing function callbacks */
	adapter->check_hw_queue_status = rsi_usb_check_queue_status;
	adapter->determine_event_timeout = rsi_usb_event_timeout;
	adapter->rsi_host_intf = RSI_HOST_INTF_USB;
	adapter->host_intf_ops = &usb_host_intf_ops;

#ifdef CONFIG_RSI_DEBUGFS
	/* In USB, one less than the MAX_DEBUGFS_ENTRIES entries is required */
	adapter->num_debugfs_entries = (MAX_DEBUGFS_ENTRIES - 1);
#endif

	rsi_dbg(INIT_ZONE, "%s: Enabled the interface\n", __func__);
	return 0;

fail_rx:
	kfree(rsi_dev->tx_buffer);

fail_eps:
	kfree(rsi_dev);

	return status;
}

static int usb_ulp_read_write(struct rsi_hw *adapter, u16 addr, u32 data,
			      u16 len_in_bits)
{
	int ret;

	ret = rsi_usb_master_reg_write
			(adapter, RSI_GSPI_DATA_REG1,
			 ((addr << 6) | ((data >> 16) & 0xffff)), 2);
	if (ret < 0)
		return ret;

	ret = rsi_usb_master_reg_write(adapter, RSI_GSPI_DATA_REG0,
				       (data & 0xffff), 2);
	if (ret < 0)
		return ret;

	/* Initializing GSPI for ULP read/writes */
	rsi_usb_master_reg_write(adapter, RSI_GSPI_CTRL_REG0,
				 RSI_GSPI_CTRL_REG0_VALUE, 2);

	ret = rsi_usb_master_reg_write(adapter, RSI_GSPI_CTRL_REG1,
				       ((len_in_bits - 1) | RSI_GSPI_TRIG), 2);
	if (ret < 0)
		return ret;

	msleep(20);

	return 0;
}

static int rsi_reset_card(struct rsi_hw *adapter)
{
	int ret;

	rsi_dbg(INFO_ZONE, "Resetting Card...\n");
	rsi_usb_master_reg_write(adapter, RSI_TA_HOLD_REG, 0xE, 4);

	/* This msleep will ensure Thread-Arch processor to go to hold
	 * and any pending dma transfers to rf in device to finish.
	 */
	msleep(100);

	ret = rsi_usb_master_reg_write(adapter, SWBL_REGOUT,
				       RSI_FW_WDT_DISABLE_REQ,
				       RSI_COMMON_REG_SIZE);
	if (ret < 0) {
		rsi_dbg(ERR_ZONE, "Disabling firmware watchdog timer failed\n");
		goto fail;
	}