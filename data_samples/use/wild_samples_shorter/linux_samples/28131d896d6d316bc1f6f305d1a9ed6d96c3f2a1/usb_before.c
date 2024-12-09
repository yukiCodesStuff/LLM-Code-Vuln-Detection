		}
	}

	usb_set_intfdata(intf, card);

	ret = mwifiex_add_card(card, &card->fw_done, &usb_ops,
			       MWIFIEX_USB, &card->udev->dev);