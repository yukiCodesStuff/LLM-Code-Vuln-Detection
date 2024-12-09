{
	struct ib_device *device;

	down_read(&lists_rwsem);
	device = __ib_device_get_by_index(index);
	if (device) {
		if (!ib_device_try_get(device))
			device = NULL;
	}
{
	struct ib_device *device;

	if (WARN_ON(size < sizeof(struct ib_device)))
		return NULL;

	device = kzalloc(size, GFP_KERNEL);
	if (!device)
		return NULL;

	rdma_restrack_init(&device->res);

	device->dev.class = &ib_class;
	device_initialize(&device->dev);

	dev_set_drvdata(&device->dev, device);

	INIT_LIST_HEAD(&device->event_handler_list);
	spin_lock_init(&device->event_handler_lock);
	rwlock_init(&device->client_data_lock);
	INIT_LIST_HEAD(&device->client_data_list);
	INIT_LIST_HEAD(&device->port_list);
	init_completion(&device->unreg_completion);

	return device;
}
EXPORT_SYMBOL(ib_alloc_device);

/**
 * ib_dealloc_device - free an IB device struct
 * @device:structure to free
 *
 * Free a structure allocated with ib_alloc_device().
 */
void ib_dealloc_device(struct ib_device *device)
{
	WARN_ON(!list_empty(&device->client_data_list));
	WARN_ON(device->reg_state != IB_DEV_UNREGISTERED &&
		device->reg_state != IB_DEV_UNINITIALIZED);
	rdma_restrack_clean(&device->res);
	put_device(&device->dev);
}
EXPORT_SYMBOL(ib_dealloc_device);

static int add_client_context(struct ib_device *device, struct ib_client *client)
{
	struct ib_client_data *context;

	context = kmalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return -ENOMEM;

	context->client = client;
	context->data   = NULL;
	context->going_down = false;

	down_write(&lists_rwsem);
	write_lock_irq(&device->client_data_lock);
	list_add(&context->list, &device->client_data_list);
	write_unlock_irq(&device->client_data_lock);
	up_write(&lists_rwsem);

	return 0;
}

static int verify_immutable(const struct ib_device *dev, u8 port)
{
	return WARN_ON(!rdma_cap_ib_mad(dev, port) &&
			    rdma_max_mad_size(dev, port) != 0);
}

static int read_port_immutable(struct ib_device *device)
{
	int ret;
	u8 start_port = rdma_start_port(device);
	u8 end_port = rdma_end_port(device);
	u8 port;

	/**
	 * device->port_immutable is indexed directly by the port number to make
	 * access to this data as efficient as possible.
	 *
	 * Therefore port_immutable is declared as a 1 based array with
	 * potential empty slots at the beginning.
	 */
	device->port_immutable = kcalloc(end_port + 1,
					 sizeof(*device->port_immutable),
					 GFP_KERNEL);
	if (!device->port_immutable)
		return -ENOMEM;

	for (port = start_port; port <= end_port; ++port) {
		ret = device->ops.get_port_immutable(
			device, port, &device->port_immutable[port]);
		if (ret)
			return ret;

		if (verify_immutable(device, port))
			return -EINVAL;
	}
	if (ret) {
		dev_warn(&device->dev,
			 "Couldn't register device with driver model\n");
		goto cg_cleanup;
	}

	refcount_set(&device->refcount, 1);
	device->reg_state = IB_DEV_REGISTERED;

	list_for_each_entry(client, &client_list, list)
		if (!add_client_context(device, client) && client->add)
			client->add(device);

	down_write(&lists_rwsem);
	list_add_tail(&device->core_list, &device_list);
	up_write(&lists_rwsem);
	mutex_unlock(&device_mutex);
	return 0;

cg_cleanup:
	ib_device_unregister_rdmacg(device);
dev_cleanup:
	cleanup_device(device);
out:
	mutex_unlock(&device_mutex);
	return ret;
}
EXPORT_SYMBOL(ib_register_device);

/**
 * ib_unregister_device - Unregister an IB device
 * @device:Device to unregister
 *
 * Unregister an IB device.  All clients will receive a remove callback.
 */
void ib_unregister_device(struct ib_device *device)
{
	struct ib_client_data *context, *tmp;
	unsigned long flags;

	/*
	 * Wait for all netlink command callers to finish working on the
	 * device.
	 */
	ib_device_put(device);
	wait_for_completion(&device->unreg_completion);

	mutex_lock(&device_mutex);

	down_write(&lists_rwsem);
	list_del(&device->core_list);
	write_lock_irq(&device->client_data_lock);
	list_for_each_entry(context, &device->client_data_list, list)
		context->going_down = true;
	write_unlock_irq(&device->client_data_lock);
	downgrade_write(&lists_rwsem);

	list_for_each_entry(context, &device->client_data_list, list) {
		if (context->client->remove)
			context->client->remove(device, context->data);
	}
	up_read(&lists_rwsem);

	ib_device_unregister_sysfs(device);
	ib_device_unregister_rdmacg(device);

	mutex_unlock(&device_mutex);

	ib_cache_cleanup_one(device);

	ib_security_destroy_port_pkey_list(device);
	kfree(device->port_pkey_list);

	down_write(&lists_rwsem);
	write_lock_irqsave(&device->client_data_lock, flags);
	list_for_each_entry_safe(context, tmp, &device->client_data_list,
				 list) {
		list_del(&context->list);
		kfree(context);
	}
	write_unlock_irqrestore(&device->client_data_lock, flags);
	up_write(&lists_rwsem);

	device->reg_state = IB_DEV_UNREGISTERED;
}
EXPORT_SYMBOL(ib_unregister_device);

/**
 * ib_register_client - Register an IB client
 * @client:Client to register
 *
 * Upper level users of the IB drivers can use ib_register_client() to
 * register callbacks for IB device addition and removal.  When an IB
 * device is added, each registered client's add method will be called
 * (in the order the clients were registered), and when a device is
 * removed, each client's remove method will be called (in the reverse
 * order that clients were registered).  In addition, when
 * ib_register_client() is called, the client will receive an add
 * callback for all devices already registered.
 */
int ib_register_client(struct ib_client *client)
{
	struct ib_device *device;

	mutex_lock(&device_mutex);

	list_for_each_entry(device, &device_list, core_list)
		if (!add_client_context(device, client) && client->add)
			client->add(device);

	down_write(&lists_rwsem);
	list_add_tail(&client->list, &client_list);
	up_write(&lists_rwsem);

	mutex_unlock(&device_mutex);

	return 0;
}
EXPORT_SYMBOL(ib_register_client);

/**
 * ib_unregister_client - Unregister an IB client
 * @client:Client to unregister
 *
 * Upper level users use ib_unregister_client() to remove their client
 * registration.  When ib_unregister_client() is called, the client
 * will receive a remove callback for each IB device still registered.
 */
void ib_unregister_client(struct ib_client *client)
{
	struct ib_client_data *context;
	struct ib_device *device;

	mutex_lock(&device_mutex);

	down_write(&lists_rwsem);
	list_del(&client->list);
	up_write(&lists_rwsem);

	list_for_each_entry(device, &device_list, core_list) {
		struct ib_client_data *found_context = NULL;

		down_write(&lists_rwsem);
		write_lock_irq(&device->client_data_lock);
		list_for_each_entry(context, &device->client_data_list, list)
			if (context->client == client) {
				context->going_down = true;
				found_context = context;
				break;
			}