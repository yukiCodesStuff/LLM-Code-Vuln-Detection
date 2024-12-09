	__u8 *buf;
	int ret = 0;

	if (!hidraw_table[minor]) {
		ret = -ENODEV;
		goto out;
	}

	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list;
	int err = 0;

	if (!(list = kzalloc(sizeof(struct hidraw_list), GFP_KERNEL))) {
		err = -ENOMEM;
	}

	mutex_lock(&minors_lock);
	if (!hidraw_table[minor]) {
		err = -ENODEV;
		goto out_unlock;
	}

	list->hidraw = hidraw_table[minor];
	mutex_init(&list->read_mutex);
	list_add_tail(&list->node, &hidraw_table[minor]->list);
	file->private_data = list;

	dev = hidraw_table[minor];
	if (!dev->open++) {
		err = hid_hw_power(dev->hid, PM_HINT_FULLON);
		if (err < 0) {
		if (err < 0) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			dev->open--;
		}
	}

out_unlock:
	mutex_unlock(&minors_lock);
out:
	if (err < 0)
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
	}

	for (i = 0; i < HIDRAW_BUFFER_SIZE; ++i)
		kfree(list->buffer[i].value);
	kfree(list);
	ret = 0;
unlock:
	mutex_unlock(&minors_lock);

	return ret;
}

static long hidraw_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
	struct hidraw *dev = hid->hidraw;
	struct hidraw_list *list;
	int ret = 0;

	list_for_each_entry(list, &dev->list, node) {
		int new_head = (list->head + 1) & (HIDRAW_BUFFER_SIZE - 1);

		if (new_head == list->tail)
		list->head = new_head;
		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	}

	wake_up_interruptible(&dev->wait);
	return ret;
}
	}

	init_waitqueue_head(&dev->wait);
	INIT_LIST_HEAD(&dev->list);

	dev->hid = hid;
	dev->minor = minor;
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
EXPORT_SYMBOL_GPL(hidraw_disconnect);
