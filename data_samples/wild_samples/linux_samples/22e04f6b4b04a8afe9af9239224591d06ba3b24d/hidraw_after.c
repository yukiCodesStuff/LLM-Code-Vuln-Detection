{
	unsigned int minor = iminor(file_inode(file));
	struct hid_device *dev;
	__u8 *buf;
	int ret = 0;

	if (!hidraw_table[minor] || !hidraw_table[minor]->exist) {
		ret = -ENODEV;
		goto out;
	}
{
	unsigned int minor = iminor(inode);
	struct hidraw *dev;
	struct hidraw_list *list;
	unsigned long flags;
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
	if (!hidraw_table[minor] || !hidraw_table[minor]->exist) {
		err = -ENODEV;
		goto out_unlock;
	}
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
		kfree(list);
	return err;

}

static int hidraw_fasync(int fd, struct file *file, int on)
{
	struct hidraw_list *list = file->private_data;

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
{
	struct inode *inode = file_inode(file);
	unsigned int minor = iminor(inode);
	long ret = 0;
	struct hidraw *dev;
	void __user *user_arg = (void __user*) arg;

	mutex_lock(&minors_lock);
	dev = hidraw_table[minor];
	if (!dev) {
		ret = -ENODEV;
		goto out;
	}

	switch (cmd) {
		case HIDIOCGRDESCSIZE:
			if (put_user(dev->hid->rsize, (int __user *)arg))
				ret = -EFAULT;
			break;

		case HIDIOCGRDESC:
			{
				__u32 len;

				if (get_user(len, (int __user *)arg))
					ret = -EFAULT;
				else if (len > HID_MAX_DESCRIPTOR_SIZE - 1)
					ret = -EINVAL;
				else if (copy_to_user(user_arg + offsetof(
					struct hidraw_report_descriptor,
					value[0]),
					dev->hid->rdesc,
					min(dev->hid->rsize, len)))
					ret = -EFAULT;
				break;
			}
{
	struct hidraw_list *list = file->private_data;

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
{
	struct hidraw *dev = hid->hidraw;
	struct hidraw_list *list;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&dev->list_lock, flags);
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
	spin_unlock_irqrestore(&dev->list_lock, flags);

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
	spin_lock_init(&dev->list_lock);
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

	drop_ref(hidraw, 1);

	mutex_unlock(&minors_lock);
}
EXPORT_SYMBOL_GPL(hidraw_disconnect);

int __init hidraw_init(void)
{
	int result;
	dev_t dev_id;

	result = alloc_chrdev_region(&dev_id, HIDRAW_FIRST_MINOR,
			HIDRAW_MAX_DEVICES, "hidraw");

	hidraw_major = MAJOR(dev_id);

	if (result < 0) {
		pr_warn("can't get major number\n");
		goto out;
	}

	hidraw_class = class_create(THIS_MODULE, "hidraw");
	if (IS_ERR(hidraw_class)) {
		result = PTR_ERR(hidraw_class);
		goto error_cdev;
	}

        cdev_init(&hidraw_cdev, &hidraw_ops);
	result = cdev_add(&hidraw_cdev, dev_id, HIDRAW_MAX_DEVICES);
	if (result < 0)
		goto error_class;

	printk(KERN_INFO "hidraw: raw HID events driver (C) Jiri Kosina\n");
out:
	return result;

error_class:
	class_destroy(hidraw_class);
error_cdev:
	unregister_chrdev_region(dev_id, HIDRAW_MAX_DEVICES);
	goto out;
}

void hidraw_exit(void)
{
	dev_t dev_id = MKDEV(hidraw_major, 0);

	cdev_del(&hidraw_cdev);
	class_destroy(hidraw_class);
	unregister_chrdev_region(dev_id, HIDRAW_MAX_DEVICES);

}
	if (IS_ERR(dev->dev)) {
		hidraw_table[minor] = NULL;
		mutex_unlock(&minors_lock);
		result = PTR_ERR(dev->dev);
		kfree(dev);
		goto out;
	}

	init_waitqueue_head(&dev->wait);
	spin_lock_init(&dev->list_lock);
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

	drop_ref(hidraw, 1);

	mutex_unlock(&minors_lock);
}
EXPORT_SYMBOL_GPL(hidraw_disconnect);

int __init hidraw_init(void)
{
	int result;
	dev_t dev_id;

	result = alloc_chrdev_region(&dev_id, HIDRAW_FIRST_MINOR,
			HIDRAW_MAX_DEVICES, "hidraw");

	hidraw_major = MAJOR(dev_id);

	if (result < 0) {
		pr_warn("can't get major number\n");
		goto out;
	}
{
	struct hidraw *hidraw = hid->hidraw;

	mutex_lock(&minors_lock);

	drop_ref(hidraw, 1);

	mutex_unlock(&minors_lock);
}
EXPORT_SYMBOL_GPL(hidraw_disconnect);

int __init hidraw_init(void)
{
	int result;
	dev_t dev_id;

	result = alloc_chrdev_region(&dev_id, HIDRAW_FIRST_MINOR,
			HIDRAW_MAX_DEVICES, "hidraw");

	hidraw_major = MAJOR(dev_id);

	if (result < 0) {
		pr_warn("can't get major number\n");
		goto out;
	}