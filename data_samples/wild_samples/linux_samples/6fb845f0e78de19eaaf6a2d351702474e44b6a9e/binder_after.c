{
	int ret;
	char *device_name, *device_tmp;
	struct binder_device *device;
	struct hlist_node *tmp;
	char *device_names = NULL;

	ret = binder_alloc_shrinker_init();
	if (ret)
		return ret;

	atomic_set(&binder_transaction_log.cur, ~0U);
	atomic_set(&binder_transaction_log_failed.cur, ~0U);

	binder_debugfs_dir_entry_root = debugfs_create_dir("binder", NULL);
	if (binder_debugfs_dir_entry_root)
		binder_debugfs_dir_entry_proc = debugfs_create_dir("proc",
						 binder_debugfs_dir_entry_root);

	if (binder_debugfs_dir_entry_root) {
		debugfs_create_file("state",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &state_fops);
		debugfs_create_file("stats",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &stats_fops);
		debugfs_create_file("transactions",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &transactions_fops);
		debugfs_create_file("transaction_log",
				    0444,
				    binder_debugfs_dir_entry_root,
				    &binder_transaction_log,
				    &transaction_log_fops);
		debugfs_create_file("failed_transaction_log",
				    0444,
				    binder_debugfs_dir_entry_root,
				    &binder_transaction_log_failed,
				    &transaction_log_fops);
	}

	if (strcmp(binder_devices_param, "") != 0) {
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
	}

	ret = init_binderfs();
	if (ret)
		goto err_init_binder_device_failed;

	return ret;

err_init_binder_device_failed:
	hlist_for_each_entry_safe(device, tmp, &binder_devices, hlist) {
		misc_deregister(&device->miscdev);
		hlist_del(&device->hlist);
		kfree(device);
	}

	kfree(device_names);

err_alloc_device_names_failed:
	debugfs_remove_recursive(binder_debugfs_dir_entry_root);

	return ret;
}
	if (binder_debugfs_dir_entry_root) {
		debugfs_create_file("state",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &state_fops);
		debugfs_create_file("stats",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &stats_fops);
		debugfs_create_file("transactions",
				    0444,
				    binder_debugfs_dir_entry_root,
				    NULL,
				    &transactions_fops);
		debugfs_create_file("transaction_log",
				    0444,
				    binder_debugfs_dir_entry_root,
				    &binder_transaction_log,
				    &transaction_log_fops);
		debugfs_create_file("failed_transaction_log",
				    0444,
				    binder_debugfs_dir_entry_root,
				    &binder_transaction_log_failed,
				    &transaction_log_fops);
	}

	if (strcmp(binder_devices_param, "") != 0) {
		/*
		* Copy the module_parameter string, because we don't want to
		* tokenize it in-place.
		 */
		device_names = kstrdup(binder_devices_param, GFP_KERNEL);
		if (!device_names) {
			ret = -ENOMEM;
			goto err_alloc_device_names_failed;
		}