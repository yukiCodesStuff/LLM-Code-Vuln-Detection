{
	struct kone_device *kone;
	struct usb_device *usb_dev;
	int retval;
	unsigned long state;

	dev = dev->parent->parent;
	kone = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = kstrtoul(buf, 10, &state);
	if (retval)
		return retval;

	if (state != 0 && state != 1)
		return -EINVAL;

	mutex_lock(&kone->kone_lock);

	if (state == 1) { /* state activate */
		retval = kone_tcu_command(usb_dev, 1);
		if (retval)
			goto exit_unlock;
		retval = kone_tcu_command(usb_dev, 2);
		if (retval)
			goto exit_unlock;
		ssleep(5); /* tcu needs this time for calibration */
		retval = kone_tcu_command(usb_dev, 3);
		if (retval)
			goto exit_unlock;
		retval = kone_tcu_command(usb_dev, 0);
		if (retval)
			goto exit_unlock;
		retval = kone_tcu_command(usb_dev, 4);
		if (retval)
			goto exit_unlock;
		/*
		 * Kone needs this time to settle things.
		 * Reading settings too early will result in invalid data.
		 * Roccat's driver waits 1 sec, maybe this time could be
		 * shortened.
		 */
		ssleep(1);
	}

	/* calibration changes values in settings, so reread */
	retval = kone_get_settings(usb_dev, &kone->settings);
	if (retval)
		goto exit_no_settings;

	/* only write settings back if activation state is different */
	if (kone->settings.tcu != state) {
		kone->settings.tcu = state;
		kone_set_settings_checksum(&kone->settings);

		retval = kone_set_settings(usb_dev, &kone->settings);
		if (retval) {
			dev_err(&usb_dev->dev, "couldn't set tcu state\n");
			/*
			 * try to reread valid settings into buffer overwriting
			 * first error code
			 */
			retval = kone_get_settings(usb_dev, &kone->settings);
			if (retval)
				goto exit_no_settings;
			goto exit_unlock;
		}
{
	struct kone_device *kone;
	struct usb_device *usb_dev;
	int retval;
	unsigned long new_startup_profile;

	dev = dev->parent->parent;
	kone = hid_get_drvdata(dev_get_drvdata(dev));
	usb_dev = interface_to_usbdev(to_usb_interface(dev));

	retval = kstrtoul(buf, 10, &new_startup_profile);
	if (retval)
		return retval;

	if (new_startup_profile  < 1 || new_startup_profile > 5)
		return -EINVAL;

	mutex_lock(&kone->kone_lock);

	kone->settings.startup_profile = new_startup_profile;
	kone_set_settings_checksum(&kone->settings);

	retval = kone_set_settings(usb_dev, &kone->settings);
	if (retval) {
		mutex_unlock(&kone->kone_lock);
		return retval;
	}