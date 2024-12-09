	down_read(&lists_rwsem);
	device = __ib_device_get_by_index(index);
	if (device) {
		if (!ib_device_try_get(device))
			device = NULL;
	}
	up_read(&lists_rwsem);
	return device;
}

/**
 * ib_device_put - Release IB device reference
 * @device: device whose reference to be released
 *
 * ib_device_put() releases reference to the IB device to allow it to be
 * unregistered and eventually free.
 */
void ib_device_put(struct ib_device *device)
{
	if (refcount_dec_and_test(&device->refcount))
		complete(&device->unreg_completion);
}
EXPORT_SYMBOL(ib_device_put);

static struct ib_device *__ib_device_get_by_name(const char *name)
{
	struct ib_device *device;
	rwlock_init(&device->client_data_lock);
	INIT_LIST_HEAD(&device->client_data_list);
	INIT_LIST_HEAD(&device->port_list);
	init_completion(&device->unreg_completion);

	return device;
}
		goto cg_cleanup;
	}

	refcount_set(&device->refcount, 1);
	device->reg_state = IB_DEV_REGISTERED;

	list_for_each_entry(client, &client_list, list)
		if (!add_client_context(device, client) && client->add)