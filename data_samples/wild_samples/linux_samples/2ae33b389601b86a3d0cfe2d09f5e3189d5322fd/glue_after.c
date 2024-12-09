{
	if (acpi_disabled)
		return -ENODEV;
	if (type && type->match && type->find_device) {
		down_write(&bus_type_sem);
		list_add_tail(&type->list, &bus_type_list);
		up_write(&bus_type_sem);
		printk(KERN_INFO PREFIX "bus type %s registered\n", type->name);
		return 0;
	}
	if (type) {
		down_write(&bus_type_sem);
		list_del_init(&type->list);
		up_write(&bus_type_sem);
		printk(KERN_INFO PREFIX "bus type %s unregistered\n",
		       type->name);
		return 0;
	}
	return -ENODEV;
}
EXPORT_SYMBOL_GPL(unregister_acpi_bus_type);

static struct acpi_bus_type *acpi_get_bus_type(struct device *dev)
{
	struct acpi_bus_type *tmp, *ret = NULL;

	down_read(&bus_type_sem);
	list_for_each_entry(tmp, &bus_type_list, list) {
		if (tmp->match(dev)) {
			ret = tmp;
			break;
		}
		if (tmp->match(dev)) {
			ret = tmp;
			break;
		}
	}
	up_read(&bus_type_sem);
	return ret;
}

static acpi_status do_acpi_find_child(acpi_handle handle, u32 lvl_not_used,
				      void *addr_p, void **ret_p)
{
	unsigned long long addr;
	acpi_status status;

	status = acpi_evaluate_integer(handle, METHOD_NAME__ADR, NULL, &addr);
	if (ACPI_SUCCESS(status) && addr == *((u64 *)addr_p)) {
		*ret_p = handle;
		return AE_CTRL_TERMINATE;
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
	struct acpi_bus_type *type = acpi_get_bus_type(dev);
	acpi_handle handle;
	int ret;

	ret = acpi_bind_one(dev, NULL);
	if (ret && type) {
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

	type = acpi_get_bus_type(dev);
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

	type = acpi_get_bus_type(dev);
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