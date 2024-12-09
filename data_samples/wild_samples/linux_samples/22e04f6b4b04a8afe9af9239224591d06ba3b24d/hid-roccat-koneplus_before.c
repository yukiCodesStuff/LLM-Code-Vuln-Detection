{
	struct koneplus_device *koneplus;
	struct usb_device *usb_dev;
	unsigned long profile;
	int retval;
	struct koneplus_roccat_report roccat_report;

	dev = dev->parent->parent;
	koneplus = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = strict_strtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile > 4)
		return -EINVAL;

	mutex_lock(&koneplus->koneplus_lock);

	retval = koneplus_set_actual_profile(usb_dev, profile);
	if (retval) {
		mutex_unlock(&koneplus->koneplus_lock);
		return retval;
	}