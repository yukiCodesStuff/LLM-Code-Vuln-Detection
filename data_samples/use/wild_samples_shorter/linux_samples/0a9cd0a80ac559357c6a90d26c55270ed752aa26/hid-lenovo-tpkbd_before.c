	struct tpkbd_data_pointer *data_pointer;
	size_t name_sz = strlen(dev_name(dev)) + 16;
	char *name_mute, *name_micmute;
	int ret;

	if (sysfs_create_group(&hdev->dev.kobj,
				&tpkbd_attr_group_pointer)) {
		hid_warn(hdev, "Could not create sysfs group\n");