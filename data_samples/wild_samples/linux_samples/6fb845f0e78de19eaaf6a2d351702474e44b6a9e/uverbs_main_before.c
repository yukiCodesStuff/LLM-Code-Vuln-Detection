{
	struct ib_uverbs_file *file =
		container_of(ref, struct ib_uverbs_file, ref);
	struct ib_device *ib_dev;
	int srcu_key;

	release_ufile_idr_uobject(file);

	srcu_key = srcu_read_lock(&file->device->disassociate_srcu);
	ib_dev = srcu_dereference(file->device->ib_dev,
				  &file->device->disassociate_srcu);
	if (ib_dev && !ib_dev->ops.disassociate_ucontext)
		module_put(ib_dev->owner);
	srcu_read_unlock(&file->device->disassociate_srcu, srcu_key);

	if (atomic_dec_and_test(&file->device->refcount))
		ib_uverbs_comp_dev(file->device);

	put_device(&file->device->dev);
	kfree(file);
}

static ssize_t ib_uverbs_event_read(struct ib_uverbs_event_queue *ev_queue,
				    struct ib_uverbs_file *uverbs_file,
				    struct file *filp, char __user *buf,
				    size_t count, loff_t *pos,
				    size_t eventsz)
{
	struct ib_uverbs_event *event;
	int ret = 0;

	spin_lock_irq(&ev_queue->lock);

	while (list_empty(&ev_queue->event_list)) {
		spin_unlock_irq(&ev_queue->lock);

		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(ev_queue->poll_wait,
					     (!list_empty(&ev_queue->event_list) ||
			/* The barriers built into wait_event_interruptible()
			 * and wake_up() guarentee this will see the null set
			 * without using RCU
			 */
					     !uverbs_file->device->ib_dev)))
			return -ERESTARTSYS;

		/* If device was disassociated and no event exists set an error */
		if (list_empty(&ev_queue->event_list) &&
		    !uverbs_file->device->ib_dev)
			return -EIO;

		spin_lock_irq(&ev_queue->lock);
	}
	while (1) {
		struct mm_struct *mm = NULL;

		/* Get an arbitrary mm pointer that hasn't been cleaned yet */
		mutex_lock(&ufile->umap_lock);
		if (!list_empty(&ufile->umaps)) {
			mm = list_first_entry(&ufile->umaps,
					      struct rdma_umap_priv, list)
				     ->vma->vm_mm;
			mmget(mm);
		}
{
	struct ib_uverbs_file *file = filp->private_data;

	uverbs_destroy_ufile_hw(file, RDMA_REMOVE_CLOSE);

	mutex_lock(&file->device->lists_mutex);
	list_del_init(&file->list);
	mutex_unlock(&file->device->lists_mutex);

	if (file->async_file)
		kref_put(&file->async_file->ref,
			 ib_uverbs_release_async_event_file);

	kref_put(&file->ref, ib_uverbs_release_file);

	return 0;
}

static const struct file_operations uverbs_fops = {
	.owner	 = THIS_MODULE,
	.write	 = ib_uverbs_write,
	.open	 = ib_uverbs_open,
	.release = ib_uverbs_close,
	.llseek	 = no_llseek,
	.unlocked_ioctl = ib_uverbs_ioctl,
	.compat_ioctl = ib_uverbs_ioctl,
};

static const struct file_operations uverbs_mmap_fops = {
	.owner	 = THIS_MODULE,
	.write	 = ib_uverbs_write,
	.mmap    = ib_uverbs_mmap,
	.open	 = ib_uverbs_open,
	.release = ib_uverbs_close,
	.llseek	 = no_llseek,
	.unlocked_ioctl = ib_uverbs_ioctl,
	.compat_ioctl = ib_uverbs_ioctl,
};

static struct ib_client uverbs_client = {
	.name   = "uverbs",
	.add    = ib_uverbs_add_one,
	.remove = ib_uverbs_remove_one
};

static ssize_t ibdev_show(struct device *device, struct device_attribute *attr,
			  char *buf)
{
	struct ib_uverbs_device *dev =
			container_of(device, struct ib_uverbs_device, dev);
	int ret = -ENODEV;
	int srcu_key;
	struct ib_device *ib_dev;

	srcu_key = srcu_read_lock(&dev->disassociate_srcu);
	ib_dev = srcu_dereference(dev->ib_dev, &dev->disassociate_srcu);
	if (ib_dev)
		ret = sprintf(buf, "%s\n", dev_name(&ib_dev->dev));
	srcu_read_unlock(&dev->disassociate_srcu, srcu_key);

	return ret;
}
static DEVICE_ATTR_RO(ibdev);

static ssize_t abi_version_show(struct device *device,
				struct device_attribute *attr, char *buf)
{
	struct ib_uverbs_device *dev =
			container_of(device, struct ib_uverbs_device, dev);
	int ret = -ENODEV;
	int srcu_key;
	struct ib_device *ib_dev;

	srcu_key = srcu_read_lock(&dev->disassociate_srcu);
	ib_dev = srcu_dereference(dev->ib_dev, &dev->disassociate_srcu);
	if (ib_dev)
		ret = sprintf(buf, "%d\n", ib_dev->uverbs_abi_ver);
	srcu_read_unlock(&dev->disassociate_srcu, srcu_key);

	return ret;
}
static DEVICE_ATTR_RO(abi_version);

static struct attribute *ib_dev_attrs[] = {
	&dev_attr_abi_version.attr,
	&dev_attr_ibdev.attr,
	NULL,
};

static const struct attribute_group dev_attr_group = {
	.attrs = ib_dev_attrs,
};

static CLASS_ATTR_STRING(abi_version, S_IRUGO,
			 __stringify(IB_USER_VERBS_ABI_VERSION));

static int ib_uverbs_create_uapi(struct ib_device *device,
				 struct ib_uverbs_device *uverbs_dev)
{
	struct uverbs_api *uapi;

	uapi = uverbs_alloc_api(device);
	if (IS_ERR(uapi))
		return PTR_ERR(uapi);

	uverbs_dev->uapi = uapi;
	return 0;
}

static void ib_uverbs_add_one(struct ib_device *device)
{
	int devnum;
	dev_t base;
	struct ib_uverbs_device *uverbs_dev;
	int ret;

	if (!device->ops.alloc_ucontext)
		return;

	uverbs_dev = kzalloc(sizeof(*uverbs_dev), GFP_KERNEL);
	if (!uverbs_dev)
		return;

	ret = init_srcu_struct(&uverbs_dev->disassociate_srcu);
	if (ret) {
		kfree(uverbs_dev);
		return;
	}

	device_initialize(&uverbs_dev->dev);
	uverbs_dev->dev.class = uverbs_class;
	uverbs_dev->dev.parent = device->dev.parent;
	uverbs_dev->dev.release = ib_uverbs_release_dev;
	uverbs_dev->groups[0] = &dev_attr_group;
	uverbs_dev->dev.groups = uverbs_dev->groups;
	atomic_set(&uverbs_dev->refcount, 1);
	init_completion(&uverbs_dev->comp);
	uverbs_dev->xrcd_tree = RB_ROOT;
	mutex_init(&uverbs_dev->xrcd_tree_mutex);
	mutex_init(&uverbs_dev->lists_mutex);
	INIT_LIST_HEAD(&uverbs_dev->uverbs_file_list);
	INIT_LIST_HEAD(&uverbs_dev->uverbs_events_file_list);
	rcu_assign_pointer(uverbs_dev->ib_dev, device);
	uverbs_dev->num_comp_vectors = device->num_comp_vectors;

	devnum = ida_alloc_max(&uverbs_ida, IB_UVERBS_MAX_DEVICES - 1,
			       GFP_KERNEL);
	if (devnum < 0)
		goto err;
	uverbs_dev->devnum = devnum;
	if (devnum >= IB_UVERBS_NUM_FIXED_MINOR)
		base = dynamic_uverbs_dev + devnum - IB_UVERBS_NUM_FIXED_MINOR;
	else
		base = IB_UVERBS_BASE_DEV + devnum;

	if (ib_uverbs_create_uapi(device, uverbs_dev))
		goto err_uapi;

	uverbs_dev->dev.devt = base;
	dev_set_name(&uverbs_dev->dev, "uverbs%d", uverbs_dev->devnum);

	cdev_init(&uverbs_dev->cdev,
		  device->ops.mmap ? &uverbs_mmap_fops : &uverbs_fops);
	uverbs_dev->cdev.owner = THIS_MODULE;

	ret = cdev_device_add(&uverbs_dev->cdev, &uverbs_dev->dev);
	if (ret)
		goto err_uapi;

	ib_set_client_data(device, &uverbs_client, uverbs_dev);
	return;

err_uapi:
	ida_free(&uverbs_ida, devnum);
err:
	if (atomic_dec_and_test(&uverbs_dev->refcount))
		ib_uverbs_comp_dev(uverbs_dev);
	wait_for_completion(&uverbs_dev->comp);
	put_device(&uverbs_dev->dev);
	return;
}

static void ib_uverbs_free_hw_resources(struct ib_uverbs_device *uverbs_dev,
					struct ib_device *ib_dev)
{
	struct ib_uverbs_file *file;
	struct ib_uverbs_async_event_file *event_file;
	struct ib_event event;

	/* Pending running commands to terminate */
	uverbs_disassociate_api_pre(uverbs_dev);
	event.event = IB_EVENT_DEVICE_FATAL;
	event.element.port_num = 0;
	event.device = ib_dev;

	mutex_lock(&uverbs_dev->lists_mutex);
	while (!list_empty(&uverbs_dev->uverbs_file_list)) {
		file = list_first_entry(&uverbs_dev->uverbs_file_list,
					struct ib_uverbs_file, list);
		list_del_init(&file->list);
		kref_get(&file->ref);

		/* We must release the mutex before going ahead and calling
		 * uverbs_cleanup_ufile, as it might end up indirectly calling
		 * uverbs_close, for example due to freeing the resources (e.g
		 * mmput).
		 */
		mutex_unlock(&uverbs_dev->lists_mutex);

		ib_uverbs_event_handler(&file->event_handler, &event);
		uverbs_destroy_ufile_hw(file, RDMA_REMOVE_DRIVER_REMOVE);
		kref_put(&file->ref, ib_uverbs_release_file);

		mutex_lock(&uverbs_dev->lists_mutex);
	}

	while (!list_empty(&uverbs_dev->uverbs_events_file_list)) {
		event_file = list_first_entry(&uverbs_dev->
					      uverbs_events_file_list,
					      struct ib_uverbs_async_event_file,
					      list);
		spin_lock_irq(&event_file->ev_queue.lock);
		event_file->ev_queue.is_closed = 1;
		spin_unlock_irq(&event_file->ev_queue.lock);

		list_del(&event_file->list);
		ib_unregister_event_handler(
			&event_file->uverbs_file->event_handler);
		event_file->uverbs_file->event_handler.device =
			NULL;

		wake_up_interruptible(&event_file->ev_queue.poll_wait);
		kill_fasync(&event_file->ev_queue.async_queue, SIGIO, POLL_IN);
	}
	mutex_unlock(&uverbs_dev->lists_mutex);

	uverbs_disassociate_api(uverbs_dev->uapi);
}

static void ib_uverbs_remove_one(struct ib_device *device, void *client_data)
{
	struct ib_uverbs_device *uverbs_dev = client_data;
	int wait_clients = 1;

	if (!uverbs_dev)
		return;

	cdev_device_del(&uverbs_dev->cdev, &uverbs_dev->dev);
	ida_free(&uverbs_ida, uverbs_dev->devnum);

	if (device->ops.disassociate_ucontext) {
		/* We disassociate HW resources and immediately return.
		 * Userspace will see a EIO errno for all future access.
		 * Upon returning, ib_device may be freed internally and is not
		 * valid any more.
		 * uverbs_device is still available until all clients close
		 * their files, then the uverbs device ref count will be zero
		 * and its resources will be freed.
		 * Note: At this point no more files can be opened since the
		 * cdev was deleted, however active clients can still issue
		 * commands and close their open files.
		 */
		ib_uverbs_free_hw_resources(uverbs_dev, device);
		wait_clients = 0;
	}

	if (atomic_dec_and_test(&uverbs_dev->refcount))
		ib_uverbs_comp_dev(uverbs_dev);
	if (wait_clients)
		wait_for_completion(&uverbs_dev->comp);

	put_device(&uverbs_dev->dev);
}

static char *uverbs_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return kasprintf(GFP_KERNEL, "infiniband/%s", dev_name(dev));
}

static int __init ib_uverbs_init(void)
{
	int ret;

	ret = register_chrdev_region(IB_UVERBS_BASE_DEV,
				     IB_UVERBS_NUM_FIXED_MINOR,
				     "infiniband_verbs");
	if (ret) {
		pr_err("user_verbs: couldn't register device number\n");
		goto out;
	}

	ret = alloc_chrdev_region(&dynamic_uverbs_dev, 0,
				  IB_UVERBS_NUM_DYNAMIC_MINOR,
				  "infiniband_verbs");
	if (ret) {
		pr_err("couldn't register dynamic device number\n");
		goto out_alloc;
	}

	uverbs_class = class_create(THIS_MODULE, "infiniband_verbs");
	if (IS_ERR(uverbs_class)) {
		ret = PTR_ERR(uverbs_class);
		pr_err("user_verbs: couldn't create class infiniband_verbs\n");
		goto out_chrdev;
	}

	uverbs_class->devnode = uverbs_devnode;

	ret = class_create_file(uverbs_class, &class_attr_abi_version.attr);
	if (ret) {
		pr_err("user_verbs: couldn't create abi_version attribute\n");
		goto out_class;
	}

	ret = ib_register_client(&uverbs_client);
	if (ret) {
		pr_err("user_verbs: couldn't register client\n");
		goto out_class;
	}

	return 0;

out_class:
	class_destroy(uverbs_class);

out_chrdev:
	unregister_chrdev_region(dynamic_uverbs_dev,
				 IB_UVERBS_NUM_DYNAMIC_MINOR);

out_alloc:
	unregister_chrdev_region(IB_UVERBS_BASE_DEV,
				 IB_UVERBS_NUM_FIXED_MINOR);

out:
	return ret;
}

static void __exit ib_uverbs_cleanup(void)
{
	ib_unregister_client(&uverbs_client);
	class_destroy(uverbs_class);
	unregister_chrdev_region(IB_UVERBS_BASE_DEV,
				 IB_UVERBS_NUM_FIXED_MINOR);
	unregister_chrdev_region(dynamic_uverbs_dev,
				 IB_UVERBS_NUM_DYNAMIC_MINOR);
}

module_init(ib_uverbs_init);
module_exit(ib_uverbs_cleanup);