{
	struct isku_device *isku;
	struct usb_device *usb_dev;
	unsigned long profile;
	int retval;
	struct isku_roccat_report roccat_report;

	dev = dev->parent->parent;
	isku = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = strict_strtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile > 4)
		return -EINVAL;

	mutex_lock(&isku->isku_lock);

	retval = isku_set_actual_profile(usb_dev, profile);
	if (retval) {
		mutex_unlock(&isku->isku_lock);
		return retval;
	}