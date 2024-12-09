	__u8 *buf;
	int ret = 0;

	if (!hidraw_table[minor] || !hidraw_table[minor]->exist) {
		ret = -ENODEV;
		goto out;
	}

	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list;
	unsigned long flags;
	int err = 0;

	if (!(list = kzalloc(sizeof(struct hidraw_list), GFP_KERNEL))) {
		err = -ENOMEM;
	}

	mutex_lock(&minors_lock);
	if (!hidraw_table[minor] || !hidraw_table[minor]->exist) {
		err = -ENODEV;
		goto out_unlock;
	}

	dev = hidraw_table[minor];
	if (!dev->open++) {
		err = hid_hw_power(dev->hid, PM_HINT_FULLON);
		if (err < 0) {
		if (err < 0) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			dev->open--;
			goto out_unlock;
		}
	}

	list->hidraw = hidraw_table[minor];
	mutex_init(&list->read_mutex);
	spin_lock_irqsave(&hidraw_table[minor]->list_lock, flags);
	list_add_tail(&list->node, &hidraw_table[minor]->list);
	spin_unlock_irqrestore(&hidraw_table[minor]->list_lock, flags);
	file->private_data = list;
out_unlock:
	mutex_unlock(&minors_lock);
out:
	if (err < 0)
	return fasync_helper(fd, file, on, &list->fasync);
}

static void drop_ref(struct hidraw *hidraw, int exists_bit)
{
	if (exists_bit) {
		hid_hw_close(hidraw->hid);
		hidraw->exist = 0;
		if (hidraw->open)
			wake_up_interruptible(&hidraw->wait);
	} else {
		--hidraw->open;
	}

	if (!hidraw->open && !hidraw->exist) {
		device_destroy(hidraw_class, MKDEV(hidraw_major, hidraw->minor));
		hidraw_table[hidraw->minor] = NULL;
		kfree(hidraw);
	}
}

static int hidraw_release(struct inode * inode, struct file * file)
{
	unsigned int minor = iminor(inode);
	struct hidraw_list *list = file->private_data;
	unsigned long flags;

	mutex_lock(&minors_lock);

	spin_lock_irqsave(&hidraw_table[minor]->list_lock, flags);
	list_del(&list->node);
	spin_unlock_irqrestore(&hidraw_table[minor]->list_lock, flags);
	kfree(list);

	drop_ref(hidraw_table[minor], 0);

	mutex_unlock(&minors_lock);
	return 0;
}

static long hidraw_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
	struct hidraw *dev = hid->hidraw;
	struct hidraw_list *list;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&dev->list_lock, flags);
	list_for_each_entry(list, &dev->list, node) {
		int new_head = (list->head + 1) & (HIDRAW_BUFFER_SIZE - 1);

		if (new_head == list->tail)
		list->head = new_head;
		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	}
	spin_unlock_irqrestore(&dev->list_lock, flags);

	wake_up_interruptible(&dev->wait);
	return ret;
}
	}

	init_waitqueue_head(&dev->wait);
	spin_lock_init(&dev->list_lock);
	INIT_LIST_HEAD(&dev->list);

	dev->hid = hid;
	dev->minor = minor;
	struct hidraw *hidraw = hid->hidraw;

	mutex_lock(&minors_lock);

	drop_ref(hidraw, 1);

	mutex_unlock(&minors_lock);
}
EXPORT_SYMBOL_GPL(hidraw_disconnect);
