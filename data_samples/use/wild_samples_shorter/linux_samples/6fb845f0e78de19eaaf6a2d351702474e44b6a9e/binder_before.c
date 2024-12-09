static int __init binder_init(void)
{
	int ret;
	char *device_name, *device_names, *device_tmp;
	struct binder_device *device;
	struct hlist_node *tmp;

	ret = binder_alloc_shrinker_init();
	if (ret)
		return ret;
				    &transaction_log_fops);
	}

	/*
	 * Copy the module_parameter string, because we don't want to
	 * tokenize it in-place.
	 */
	device_names = kstrdup(binder_devices_param, GFP_KERNEL);
	if (!device_names) {
		ret = -ENOMEM;
		goto err_alloc_device_names_failed;
	}

	device_tmp = device_names;
	while ((device_name = strsep(&device_tmp, ","))) {
		ret = init_binder_device(device_name);
		if (ret)
			goto err_init_binder_device_failed;
	}

	return ret;

err_init_binder_device_failed:
	hlist_for_each_entry_safe(device, tmp, &binder_devices, hlist) {