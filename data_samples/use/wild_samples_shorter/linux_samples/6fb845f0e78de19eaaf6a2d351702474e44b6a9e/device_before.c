	down_read(&lists_rwsem);
	device = __ib_device_get_by_index(index);
	if (device) {
		/* Do not return a device if unregistration has started. */
		if (!refcount_inc_not_zero(&device->refcount))
			device = NULL;
	}
	up_read(&lists_rwsem);
	return device;
}

void ib_device_put(struct ib_device *device)
{
	if (refcount_dec_and_test(&device->refcount))
		complete(&device->unreg_completion);
}

static struct ib_device *__ib_device_get_by_name(const char *name)
{
	struct ib_device *device;
	rwlock_init(&device->client_data_lock);
	INIT_LIST_HEAD(&device->client_data_list);
	INIT_LIST_HEAD(&device->port_list);
	refcount_set(&device->refcount, 1);
	init_completion(&device->unreg_completion);

	return device;
}
		goto cg_cleanup;
	}

	device->reg_state = IB_DEV_REGISTERED;

	list_for_each_entry(client, &client_list, list)
		if (!add_client_context(device, client) && client->add)