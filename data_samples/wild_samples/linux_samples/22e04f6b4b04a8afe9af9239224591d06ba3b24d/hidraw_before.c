{
	unsigned int minor = iminor(file_inode(file));
	struct hid_device *dev;
	__u8 *buf;
	int ret = 0;

	if (!hidraw_table[minor]) {
		ret = -ENODEV;
		goto out;
	}
{
	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list;
	int err = 0;

	if (!(list = kzalloc(sizeof(struct hidraw_list), GFP_KERNEL))) {
		err = -ENOMEM;
		goto out;
	}
	if (!(list = kzalloc(sizeof(struct hidraw_list), GFP_KERNEL))) {
		err = -ENOMEM;
		goto out;
	}

	mutex_lock(&minors_lock);
	if (!hidraw_table[minor]) {
		err = -ENODEV;
		goto out_unlock;
	}
		if (err < 0) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			dev->open--;
		}
	}

out_unlock:
	mutex_unlock(&minors_lock);
out:
	if (err < 0)
		kfree(list);
	return err;

}

static int hidraw_fasync(int fd, struct file *file, int on)
{
	struct hidraw_list *list = file->private_data;

	return fasync_helper(fd, file, on, &list->fasync);
}

static int hidraw_release(struct inode * inode, struct file * file)
{
	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list = file->private_data;
	int ret;
	int i;

	mutex_lock(&minors_lock);
	if (!hidraw_table[minor]) {
		ret = -ENODEV;
		goto unlock;
	}

	list_del(&list->node);
	dev = hidraw_table[minor];
	if (!--dev->open) {
		if (list->hidraw->exist) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			hid_hw_close(dev->hid);
		} else {
			kfree(list->hidraw);
		}
{
	struct hidraw_list *list = file->private_data;

	return fasync_helper(fd, file, on, &list->fasync);
}

static int hidraw_release(struct inode * inode, struct file * file)
{
	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list = file->private_data;
	int ret;
	int i;

	mutex_lock(&minors_lock);
	if (!hidraw_table[minor]) {
		ret = -ENODEV;
		goto unlock;
	}
{
	struct hidraw *dev = hid->hidraw;
	struct hidraw_list *list;
	int ret = 0;

	list_for_each_entry(list, &dev->list, node) {
		int new_head = (list->head + 1) & (HIDRAW_BUFFER_SIZE - 1);

		if (new_head == list->tail)
			continue;

		if (!(list->buffer[list->head].value = kmemdup(data, len, GFP_ATOMIC))) {
			ret = -ENOMEM;
			break;
		}
		list->buffer[list->head].len = len;
		list->head = new_head;
		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	}
		if (!(list->buffer[list->head].value = kmemdup(data, len, GFP_ATOMIC))) {
			ret = -ENOMEM;
			break;
		}
		list->buffer[list->head].len = len;
		list->head = new_head;
		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	}

	wake_up_interruptible(&dev->wait);
	return ret;
}
EXPORT_SYMBOL_GPL(hidraw_report_event);

int hidraw_connect(struct hid_device *hid)
{
	int minor, result;
	struct hidraw *dev;

	/* we accept any HID device, no matter the applications */

	dev = kzalloc(sizeof(struct hidraw), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	result = -EINVAL;

	mutex_lock(&minors_lock);

	for (minor = 0; minor < HIDRAW_MAX_DEVICES; minor++) {
		if (hidraw_table[minor])
			continue;
		hidraw_table[minor] = dev;
		result = 0;
		break;
	}

	if (result) {
		mutex_unlock(&minors_lock);
		kfree(dev);
		goto out;
	}

	dev->dev = device_create(hidraw_class, &hid->dev, MKDEV(hidraw_major, minor),
				 NULL, "%s%d", "hidraw", minor);

	if (IS_ERR(dev->dev)) {
		hidraw_table[minor] = NULL;
		mutex_unlock(&minors_lock);
		result = PTR_ERR(dev->dev);
		kfree(dev);
		goto out;
	}

	init_waitqueue_head(&dev->wait);
	INIT_LIST_HEAD(&dev->list);

	dev->hid = hid;
	dev->minor = minor;

	dev->exist = 1;
	hid->hidraw = dev;

	mutex_unlock(&minors_lock);
out:
	return result;

}
EXPORT_SYMBOL_GPL(hidraw_connect);

void hidraw_disconnect(struct hid_device *hid)
{
	struct hidraw *hidraw = hid->hidraw;

	mutex_lock(&minors_lock);
	hidraw->exist = 0;

	device_destroy(hidraw_class, MKDEV(hidraw_major, hidraw->minor));

	hidraw_table[hidraw->minor] = NULL;

	if (hidraw->open) {
		hid_hw_close(hid);
		wake_up_interruptible(&hidraw->wait);
	} else {
		kfree(hidraw);
	}
	if (IS_ERR(dev->dev)) {
		hidraw_table[minor] = NULL;
		mutex_unlock(&minors_lock);
		result = PTR_ERR(dev->dev);
		kfree(dev);
		goto out;
	}

	init_waitqueue_head(&dev->wait);
	INIT_LIST_HEAD(&dev->list);

	dev->hid = hid;
	dev->minor = minor;

	dev->exist = 1;
	hid->hidraw = dev;

	mutex_unlock(&minors_lock);
out:
	return result;

}
EXPORT_SYMBOL_GPL(hidraw_connect);

void hidraw_disconnect(struct hid_device *hid)
{
	struct hidraw *hidraw = hid->hidraw;

	mutex_lock(&minors_lock);
	hidraw->exist = 0;

	device_destroy(hidraw_class, MKDEV(hidraw_major, hidraw->minor));

	hidraw_table[hidraw->minor] = NULL;

	if (hidraw->open) {
		hid_hw_close(hid);
		wake_up_interruptible(&hidraw->wait);
	} else {
		kfree(hidraw);
	}
	mutex_unlock(&minors_lock);
}
{
	struct hidraw *hidraw = hid->hidraw;

	mutex_lock(&minors_lock);
	hidraw->exist = 0;

	device_destroy(hidraw_class, MKDEV(hidraw_major, hidraw->minor));

	hidraw_table[hidraw->minor] = NULL;

	if (hidraw->open) {
		hid_hw_close(hid);
		wake_up_interruptible(&hidraw->wait);
	} else {
		kfree(hidraw);
	}
	mutex_unlock(&minors_lock);
}