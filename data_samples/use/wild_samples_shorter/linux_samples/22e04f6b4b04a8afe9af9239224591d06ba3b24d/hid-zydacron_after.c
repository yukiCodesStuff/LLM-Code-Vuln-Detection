	int ret;
	struct zc_device *zc;

	zc = devm_kzalloc(&hdev->dev, sizeof(*zc), GFP_KERNEL);
	if (zc == NULL) {
		hid_err(hdev, "can't alloc descriptor\n");
		return -ENOMEM;
	}
	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}

	return 0;
}

static const struct hid_device_id zc_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_ZYDACRON, USB_DEVICE_ID_ZYDACRON_REMOTE_CONTROL) },
	.input_mapping = zc_input_mapping,
	.raw_event = zc_raw_event,
	.probe = zc_probe,
};
module_hid_driver(zc_driver);

MODULE_LICENSE("GPL");