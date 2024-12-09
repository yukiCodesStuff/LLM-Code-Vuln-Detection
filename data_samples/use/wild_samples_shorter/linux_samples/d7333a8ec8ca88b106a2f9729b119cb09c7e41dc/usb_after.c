				   le16_to_cpu(endpoint->wMaxPacketSize),
				   endpoint->bInterval);
		}

		/* Ignore broken descriptors. */
		if (usb_endpoint_maxp(endpoint) == 0)
			continue;

		urbcount = 0;

		pipe_num =
		    ath6kl_usb_get_logical_pipe_num(ar_usb,
				 req,
				 USB_DIR_IN | USB_TYPE_VENDOR |
				 USB_RECIP_DEVICE, value, index, buf,
				 size, 2000);

	if (ret < 0) {
		ath6kl_warn("Failed to read usb control message: %d\n", ret);
		kfree(buf);