{
	struct kovaplus_device *kovaplus;
	struct usb_device *usb_dev;
	unsigned long profile;
	int retval;
	struct kovaplus_roccat_report roccat_report;

	dev = dev->parent->parent;
	kovaplus = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = strict_strtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile >= 5)
		return -EINVAL;

	mutex_lock(&kovaplus->kovaplus_lock);
	retval = kovaplus_set_actual_profile(usb_dev, profile);
	if (retval) {
		mutex_unlock(&kovaplus->kovaplus_lock);
		return retval;
	}