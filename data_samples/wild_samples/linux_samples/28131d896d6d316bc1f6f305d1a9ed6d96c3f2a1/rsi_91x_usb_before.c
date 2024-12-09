{
	struct rsi_91x_usbdev *dev = (struct rsi_91x_usbdev *)adapter->rsi_dev;
	int status;
	u8 *seg = dev->tx_buffer;
	int transfer;
	int ep = dev->bulkout_endpoint_addr[endpoint - 1];

	memset(seg, 0, len + RSI_USB_TX_HEAD_ROOM);
	memcpy(seg + RSI_USB_TX_HEAD_ROOM, buf, len);
	len += RSI_USB_TX_HEAD_ROOM;
	transfer = len;
	status = usb_bulk_msg(dev->usbdev,
			      usb_sndbulkpipe(dev->usbdev, ep),
			      (void *)seg,
			      (int)len,
			      &transfer,
			      HZ * 5);

	if (status < 0) {
		rsi_dbg(ERR_ZONE,
			"Card write failed with error code :%10d\n", status);
		dev->write_fail = 1;
	}