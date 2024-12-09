{
	driver_unregister(&driver->driver);
}
EXPORT_SYMBOL_GPL(scmi_driver_unregister);

struct scmi_device *
scmi_device_create(struct device_node *np, struct device *parent, int protocol)
{
	int id, retval;
	struct scmi_device *scmi_dev;

	scmi_dev = kzalloc(sizeof(*scmi_dev), GFP_KERNEL);
	if (!scmi_dev)
		return NULL;

	id = ida_simple_get(&scmi_bus_id, 1, 0, GFP_KERNEL);
	if (id < 0)
		goto free_mem;

	scmi_dev->id = id;
	scmi_dev->protocol_id = protocol;
	scmi_dev->dev.parent = parent;
	scmi_dev->dev.of_node = np;
	scmi_dev->dev.bus = &scmi_bus_type;
	dev_set_name(&scmi_dev->dev, "scmi_dev.%d", id);

	retval = device_register(&scmi_dev->dev);
	if (retval)
		goto put_dev;

	return scmi_dev;
put_dev:
	put_device(&scmi_dev->dev);
	ida_simple_remove(&scmi_bus_id, id);
free_mem:
	kfree(scmi_dev);
	return NULL;
}
{
	int id, retval;
	struct scmi_device *scmi_dev;

	scmi_dev = kzalloc(sizeof(*scmi_dev), GFP_KERNEL);
	if (!scmi_dev)
		return NULL;

	id = ida_simple_get(&scmi_bus_id, 1, 0, GFP_KERNEL);
	if (id < 0)
		goto free_mem;

	scmi_dev->id = id;
	scmi_dev->protocol_id = protocol;
	scmi_dev->dev.parent = parent;
	scmi_dev->dev.of_node = np;
	scmi_dev->dev.bus = &scmi_bus_type;
	dev_set_name(&scmi_dev->dev, "scmi_dev.%d", id);

	retval = device_register(&scmi_dev->dev);
	if (retval)
		goto put_dev;

	return scmi_dev;
put_dev:
	put_device(&scmi_dev->dev);
	ida_simple_remove(&scmi_bus_id, id);
free_mem:
	kfree(scmi_dev);
	return NULL;
}

void scmi_device_destroy(struct scmi_device *scmi_dev)
{
	scmi_handle_put(scmi_dev->handle);
	device_unregister(&scmi_dev->dev);
	ida_simple_remove(&scmi_bus_id, scmi_dev->id);
	kfree(scmi_dev);
}

void scmi_set_handle(struct scmi_device *scmi_dev)
{
	scmi_dev->handle = scmi_handle_get(&scmi_dev->dev);
}

int scmi_protocol_register(int protocol_id, scmi_prot_init_fn_t fn)
{
	int ret;

	spin_lock(&protocol_lock);
	ret = idr_alloc(&scmi_protocols, fn, protocol_id, protocol_id + 1,
			GFP_ATOMIC);
	spin_unlock(&protocol_lock);
	if (ret != protocol_id)
		pr_err("unable to allocate SCMI idr slot, err %d\n", ret);

	return ret;
}
EXPORT_SYMBOL_GPL(scmi_protocol_register);

void scmi_protocol_unregister(int protocol_id)
{
	spin_lock(&protocol_lock);
	idr_remove(&scmi_protocols, protocol_id);
	spin_unlock(&protocol_lock);
}
EXPORT_SYMBOL_GPL(scmi_protocol_unregister);

static int __scmi_devices_unregister(struct device *dev, void *data)
{
	struct scmi_device *scmi_dev = to_scmi_dev(dev);

	scmi_device_destroy(scmi_dev);
	return 0;
}

static void scmi_devices_unregister(void)
{
	bus_for_each_dev(&scmi_bus_type, NULL, NULL, __scmi_devices_unregister);
}

static int __init scmi_bus_init(void)
{
	int retval;

	retval = bus_register(&scmi_bus_type);
	if (retval)
		pr_err("scmi protocol bus register failed (%d)\n", retval);

	return retval;
}
subsys_initcall(scmi_bus_init);

static void __exit scmi_bus_exit(void)
{
	scmi_devices_unregister();
	bus_unregister(&scmi_bus_type);
	ida_destroy(&scmi_bus_id);
}
module_exit(scmi_bus_exit);
{
	scmi_handle_put(scmi_dev->handle);
	device_unregister(&scmi_dev->dev);
	ida_simple_remove(&scmi_bus_id, scmi_dev->id);
	kfree(scmi_dev);
}