	int ret;
	struct zc_device *zc;

	zc = kzalloc(sizeof(*zc), GFP_KERNEL);
	if (zc == NULL) {
		hid_err(hdev, "can't alloc descriptor\n");
		return -ENOMEM;
	}
	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		goto err_free;
	}

	return 0;
err_free:
	kfree(zc);

	return ret;
}

static void zc_remove(struct hid_device *hdev)
{
	struct zc_device *zc = hid_get_drvdata(hdev);

	hid_hw_stop(hdev);
	kfree(zc);
}

static const struct hid_device_id zc_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_ZYDACRON, USB_DEVICE_ID_ZYDACRON_REMOTE_CONTROL) },
	.input_mapping = zc_input_mapping,
	.raw_event = zc_raw_event,
	.probe = zc_probe,
	.remove = zc_remove,
};
module_hid_driver(zc_driver);

MODULE_LICENSE("GPL");