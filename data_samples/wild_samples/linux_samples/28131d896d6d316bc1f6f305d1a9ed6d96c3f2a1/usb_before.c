		     usb_endpoint_xfer_int(epd))) {
			card->tx_cmd_ep_type = usb_endpoint_type(epd);
			card->tx_cmd_interval = epd->bInterval;
			pr_debug("info: bulk OUT: max pkt size: %d, addr: %d\n",
				 le16_to_cpu(epd->wMaxPacketSize),
				 epd->bEndpointAddress);
			pr_debug("info: Tx CMD:: max pkt size: %d, addr: %d, ep_type: %d\n",
				 le16_to_cpu(epd->wMaxPacketSize),
				 epd->bEndpointAddress, card->tx_cmd_ep_type);
			card->tx_cmd_ep = usb_endpoint_num(epd);
			atomic_set(&card->tx_cmd_urb_pending, 0);
			card->bulk_out_maxpktsize =
					le16_to_cpu(epd->wMaxPacketSize);
		}
	}

	usb_set_intfdata(intf, card);

	ret = mwifiex_add_card(card, &card->fw_done, &usb_ops,
			       MWIFIEX_USB, &card->udev->dev);
	if (ret) {
		pr_err("%s: mwifiex_add_card failed: %d\n", __func__, ret);
		usb_reset_device(udev);
		return ret;
	}

	usb_get_dev(udev);

	return 0;
}

/* Kernel needs to suspend all functions separately. Therefore all
 * registered functions must have drivers with suspend and resume
 * methods. Failing that the kernel simply removes the whole card.
 *
 * If already not suspended, this function allocates and sends a
 * 'host sleep activate' request to the firmware and turns off the traffic.
 */
static int mwifiex_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usb_card_rec *card = usb_get_intfdata(intf);
	struct mwifiex_adapter *adapter;
	struct usb_tx_data_port *port;
	int i, j;

	/* Might still be loading firmware */
	wait_for_completion(&card->fw_done);

	adapter = card->adapter;
	if (!adapter) {
		dev_err(&intf->dev, "card is not valid\n");
		return 0;
	}

	if (unlikely(test_bit(MWIFIEX_IS_SUSPENDED, &adapter->work_flags)))
		mwifiex_dbg(adapter, WARN,
			    "Device already suspended\n");

	/* Enable the Host Sleep */
	if (!mwifiex_enable_hs(adapter)) {
		mwifiex_dbg(adapter, ERROR,
			    "cmd: failed to suspend\n");
		clear_bit(MWIFIEX_IS_HS_ENABLING, &adapter->work_flags);
		return -EFAULT;
	}


	/* 'MWIFIEX_IS_SUSPENDED' bit indicates device is suspended.
	 * It must be set here before the usb_kill_urb() calls. Reason
	 * is in the complete handlers, urb->status(= -ENOENT) and
	 * this flag is used in combination to distinguish between a
	 * 'suspended' state and a 'disconnect' one.
	 */
	set_bit(MWIFIEX_IS_SUSPENDED, &adapter->work_flags);
	clear_bit(MWIFIEX_IS_HS_ENABLING, &adapter->work_flags);

	if (atomic_read(&card->rx_cmd_urb_pending) && card->rx_cmd.urb)
		usb_kill_urb(card->rx_cmd.urb);

	if (atomic_read(&card->rx_data_urb_pending))
		for (i = 0; i < MWIFIEX_RX_DATA_URB; i++)
			if (card->rx_data_list[i].urb)
				usb_kill_urb(card->rx_data_list[i].urb);

	for (i = 0; i < MWIFIEX_TX_DATA_PORT; i++) {
		port = &card->port[i];
		for (j = 0; j < MWIFIEX_TX_DATA_URB; j++) {
			if (port->tx_data_list[j].urb)
				usb_kill_urb(port->tx_data_list[j].urb);
		}