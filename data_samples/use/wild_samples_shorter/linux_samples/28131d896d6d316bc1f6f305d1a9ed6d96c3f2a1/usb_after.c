		}
	}

	switch (card->usb_boot_state) {
	case USB8XXX_FW_DNLD:
		/* Reject broken descriptors. */
		if (!card->rx_cmd_ep || !card->tx_cmd_ep)
			return -ENODEV;
		if (card->bulk_out_maxpktsize == 0)
			return -ENODEV;
		break;
	case USB8XXX_FW_READY:
		/* Assume the driver can handle missing endpoints for now. */
		break;
	default:
		WARN_ON(1);
		return -ENODEV;
	}

	usb_set_intfdata(intf, card);

	ret = mwifiex_add_card(card, &card->fw_done, &usb_ops,
			       MWIFIEX_USB, &card->udev->dev);