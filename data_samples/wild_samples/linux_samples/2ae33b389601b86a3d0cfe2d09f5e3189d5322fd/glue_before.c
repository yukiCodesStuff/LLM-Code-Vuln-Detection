{
	if (acpi_disabled)
		return -ENODEV;
	if (type && type->bus && type->find_device) {
		down_write(&bus_type_sem);
		list_add_tail(&type->list, &bus_type_list);
		up_write(&bus_type_sem);
		printk(KERN_INFO PREFIX "bus type %s registered\n",
		       type->bus->name);
		return 0;
	}
	if (type) {
		down_write(&bus_type_sem);
		list_del_init(&type->list);
		up_write(&bus_type_sem);
		printk(KERN_INFO PREFIX "ACPI bus type %s unregistered\n",
		       type->bus->name);
		return 0;
	}
	return -ENODEV;
}
EXPORT_SYMBOL_GPL(unregister_acpi_bus_type);

static struct acpi_bus_type *acpi_get_bus_type(struct bus_type *type)
{
	struct acpi_bus_type *tmp, *ret = NULL;

	if (!type)
		return NULL;

	down_read(&bus_type_sem);
	list_for_each_entry(tmp, &bus_type_list, list) {
		if (tmp->bus == type) {
			ret = tmp;
			break;
		}
		if (tmp->bus == type) {
			ret = tmp;
			break;
		}
	}
	up_read(&bus_type_sem);
	return ret;
}

static int acpi_find_bridge_device(struct device *dev, acpi_handle * handle)
{
	struct acpi_bus_type *tmp;
	int ret = -ENODEV;

	down_read(&bus_type_sem);
	list_for_each_entry(tmp, &bus_type_list, list) {
		if (tmp->find_bridge && !tmp->find_bridge(dev, handle)) {
			ret = 0;
			break;
		}
	}
	list_for_each_safe(node, next, &acpi_dev->physical_node_list) {
		char physical_node_name[sizeof(PHYSICAL_NODE_STRING) + 2];

		entry = list_entry(node, struct acpi_device_physical_node,
			node);
		if (entry->dev != dev)
			continue;

		list_del(node);
		clear_bit(entry->node_id, acpi_dev->physical_node_id_bitmap);

		acpi_dev->physical_node_count--;

		if (!entry->node_id)
			strcpy(physical_node_name, PHYSICAL_NODE_STRING);
		else
			sprintf(physical_node_name,
				"physical_node%d", entry->node_id);

		sysfs_remove_link(&acpi_dev->dev.kobj, physical_node_name);
		sysfs_remove_link(&dev->kobj, "firmware_node");
		ACPI_HANDLE_SET(dev, NULL);
		/* acpi_bind_one increase refcnt by one */
		put_device(dev);
		kfree(entry);
	}
	mutex_unlock(&acpi_dev->physical_node_lock);

	return 0;

err:
	dev_err(dev, "Oops, 'acpi_handle' corrupt\n");
	return -EINVAL;
}

static int acpi_platform_notify(struct device *dev)
{
	struct acpi_bus_type *type;
	acpi_handle handle;
	int ret;

	ret = acpi_bind_one(dev, NULL);
	if (ret && (!dev->bus || !dev->parent)) {
		/* bridge devices genernally haven't bus or parent */
		ret = acpi_find_bridge_device(dev, &handle);
		if (!ret) {
			ret = acpi_bind_one(dev, handle);
			if (ret)
				goto out;
		}
	}

	type = acpi_get_bus_type(dev->bus);
	if (ret) {
		if (!type || !type->find_device) {
			DBG("No ACPI bus support for %s\n", dev_name(dev));
			ret = -EINVAL;
			goto out;
		}

		ret = type->find_device(dev, &handle);
		if (ret) {
			DBG("Unable to get handle for %s\n", dev_name(dev));
			goto out;
		}
		ret = acpi_bind_one(dev, handle);
		if (ret)
			goto out;
	}

	if (type && type->setup)
		type->setup(dev);

 out:
#if ACPI_GLUE_DEBUG
	if (!ret) {
		struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

		acpi_get_name(ACPI_HANDLE(dev), ACPI_FULL_PATHNAME, &buffer);
		DBG("Device %s -> %s\n", dev_name(dev), (char *)buffer.pointer);
		kfree(buffer.pointer);
	} else
		DBG("Device %s -> No ACPI support\n", dev_name(dev));
#endif

	return ret;
}

static int acpi_platform_notify_remove(struct device *dev)
{
	struct acpi_bus_type *type;

	type = acpi_get_bus_type(dev->bus);
	if (type && type->cleanup)
		type->cleanup(dev);

	acpi_unbind_one(dev);
	return 0;
}

int __init init_acpi_device_notify(void)
{
	if (platform_notify || platform_notify_remove) {
		printk(KERN_ERR PREFIX "Can't use platform_notify\n");
		return 0;
	}
	platform_notify = acpi_platform_notify;
	platform_notify_remove = acpi_platform_notify_remove;
	return 0;
}
{
	struct acpi_bus_type *type;

	type = acpi_get_bus_type(dev->bus);
	if (type && type->cleanup)
		type->cleanup(dev);

	acpi_unbind_one(dev);
	return 0;
}

int __init init_acpi_device_notify(void)
{
	if (platform_notify || platform_notify_remove) {
		printk(KERN_ERR PREFIX "Can't use platform_notify\n");
		return 0;
	}