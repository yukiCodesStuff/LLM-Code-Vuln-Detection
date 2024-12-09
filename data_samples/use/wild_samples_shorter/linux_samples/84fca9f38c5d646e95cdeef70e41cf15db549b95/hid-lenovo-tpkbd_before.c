	struct tpkbd_data_pointer *data_pointer;
	size_t name_sz = strlen(dev_name(dev)) + 16;
	char *name_mute, *name_micmute;
	int ret;

	if (sysfs_create_group(&hdev->dev.kobj,
				&tpkbd_attr_group_pointer)) {
		hid_warn(hdev, "Could not create sysfs group\n");
	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "hid_parse failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hid_hw_start failed\n");
		goto err_free;
	}

	uhdev = (struct usbhid_device *) hdev->driver_data;

	if (uhdev->ifnum == 1)
		return tpkbd_probe_tp(hdev);

	return 0;
err_free:
	return ret;
}

static void tpkbd_remove_tp(struct hid_device *hdev)