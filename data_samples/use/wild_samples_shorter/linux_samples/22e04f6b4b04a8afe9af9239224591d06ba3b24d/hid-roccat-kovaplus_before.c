	kovaplus = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = strict_strtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile >= 5)