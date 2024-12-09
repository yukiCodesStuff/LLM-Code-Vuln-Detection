{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_mode_key temp_buf;
	unsigned long state;
	int retval;

	retval = kstrtoul(buf, 10, &state);
	if (retval)
		return retval;

	temp_buf.command = ARVO_COMMAND_MODE_KEY;
	temp_buf.state = state;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_MODE_KEY,
			&temp_buf, sizeof(struct arvo_mode_key));
	mutex_unlock(&arvo->arvo_lock);
	if (retval)
		return retval;

	return size;
}
static DEVICE_ATTR(mode_key, 0660,
		   arvo_sysfs_show_mode_key, arvo_sysfs_set_mode_key);

static ssize_t arvo_sysfs_show_key_mask(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_key_mask temp_buf;
	int retval;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_receive(usb_dev, ARVO_COMMAND_KEY_MASK,
			&temp_buf, sizeof(struct arvo_key_mask));
	mutex_unlock(&arvo->arvo_lock);
	if (retval)
		return retval;

	return snprintf(buf, PAGE_SIZE, "%d\n", temp_buf.key_mask);
}

static ssize_t arvo_sysfs_set_key_mask(struct device *dev,
		struct device_attribute *attr, char const *buf, size_t size)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_key_mask temp_buf;
	unsigned long key_mask;
	int retval;

	retval = kstrtoul(buf, 10, &key_mask);
	if (retval)
		return retval;

	temp_buf.command = ARVO_COMMAND_KEY_MASK;
	temp_buf.key_mask = key_mask;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_KEY_MASK,
			&temp_buf, sizeof(struct arvo_key_mask));
	mutex_unlock(&arvo->arvo_lock);
	if (retval)
		return retval;

	return size;
}
static DEVICE_ATTR(key_mask, 0660,
		   arvo_sysfs_show_key_mask, arvo_sysfs_set_key_mask);

/* retval is 1-5 on success, < 0 on error */
static int arvo_get_actual_profile(struct usb_device *usb_dev)
{
	struct arvo_actual_profile temp_buf;
	int retval;

	retval = roccat_common2_receive(usb_dev, ARVO_COMMAND_ACTUAL_PROFILE,
			&temp_buf, sizeof(struct arvo_actual_profile));

	if (retval)
		return retval;

	return temp_buf.actual_profile;
}

static ssize_t arvo_sysfs_show_actual_profile(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));

	return snprintf(buf, PAGE_SIZE, "%d\n", arvo->actual_profile);
}

static ssize_t arvo_sysfs_set_actual_profile(struct device *dev,
		struct device_attribute *attr, char const *buf, size_t size)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_actual_profile temp_buf;
	unsigned long profile;
	int retval;

	retval = kstrtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile < 1 || profile > 5)
		return -EINVAL;

	temp_buf.command = ARVO_COMMAND_ACTUAL_PROFILE;
	temp_buf.actual_profile = profile;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_ACTUAL_PROFILE,
			&temp_buf, sizeof(struct arvo_actual_profile));
	if (!retval) {
		arvo->actual_profile = profile;
		retval = size;
	}
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_key_mask temp_buf;
	unsigned long key_mask;
	int retval;

	retval = kstrtoul(buf, 10, &key_mask);
	if (retval)
		return retval;

	temp_buf.command = ARVO_COMMAND_KEY_MASK;
	temp_buf.key_mask = key_mask;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_KEY_MASK,
			&temp_buf, sizeof(struct arvo_key_mask));
	mutex_unlock(&arvo->arvo_lock);
	if (retval)
		return retval;

	return size;
}
static DEVICE_ATTR(key_mask, 0660,
		   arvo_sysfs_show_key_mask, arvo_sysfs_set_key_mask);

/* retval is 1-5 on success, < 0 on error */
static int arvo_get_actual_profile(struct usb_device *usb_dev)
{
	struct arvo_actual_profile temp_buf;
	int retval;

	retval = roccat_common2_receive(usb_dev, ARVO_COMMAND_ACTUAL_PROFILE,
			&temp_buf, sizeof(struct arvo_actual_profile));

	if (retval)
		return retval;

	return temp_buf.actual_profile;
}

static ssize_t arvo_sysfs_show_actual_profile(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));

	return snprintf(buf, PAGE_SIZE, "%d\n", arvo->actual_profile);
}

static ssize_t arvo_sysfs_set_actual_profile(struct device *dev,
		struct device_attribute *attr, char const *buf, size_t size)
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_actual_profile temp_buf;
	unsigned long profile;
	int retval;

	retval = kstrtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile < 1 || profile > 5)
		return -EINVAL;

	temp_buf.command = ARVO_COMMAND_ACTUAL_PROFILE;
	temp_buf.actual_profile = profile;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_ACTUAL_PROFILE,
			&temp_buf, sizeof(struct arvo_actual_profile));
	if (!retval) {
		arvo->actual_profile = profile;
		retval = size;
	}
{
	struct arvo_device *arvo =
			hid_get_drvdata(dev_get_drvdata(dev->parent->parent));
	struct usb_device *usb_dev =
			interface_to_usbdev(to_usb_interface(dev->parent->parent));
	struct arvo_actual_profile temp_buf;
	unsigned long profile;
	int retval;

	retval = kstrtoul(buf, 10, &profile);
	if (retval)
		return retval;

	if (profile < 1 || profile > 5)
		return -EINVAL;

	temp_buf.command = ARVO_COMMAND_ACTUAL_PROFILE;
	temp_buf.actual_profile = profile;

	mutex_lock(&arvo->arvo_lock);
	retval = roccat_common2_send(usb_dev, ARVO_COMMAND_ACTUAL_PROFILE,
			&temp_buf, sizeof(struct arvo_actual_profile));
	if (!retval) {
		arvo->actual_profile = profile;
		retval = size;
	}