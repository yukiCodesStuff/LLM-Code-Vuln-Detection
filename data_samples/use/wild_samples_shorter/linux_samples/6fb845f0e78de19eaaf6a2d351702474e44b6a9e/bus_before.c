}
EXPORT_SYMBOL_GPL(scmi_driver_unregister);

struct scmi_device *
scmi_device_create(struct device_node *np, struct device *parent, int protocol)
{
	int id, retval;
	scmi_dev->dev.parent = parent;
	scmi_dev->dev.of_node = np;
	scmi_dev->dev.bus = &scmi_bus_type;
	dev_set_name(&scmi_dev->dev, "scmi_dev.%d", id);

	retval = device_register(&scmi_dev->dev);
	if (retval)
void scmi_device_destroy(struct scmi_device *scmi_dev)
{
	scmi_handle_put(scmi_dev->handle);
	device_unregister(&scmi_dev->dev);
	ida_simple_remove(&scmi_bus_id, scmi_dev->id);
	kfree(scmi_dev);
}

void scmi_set_handle(struct scmi_device *scmi_dev)
{