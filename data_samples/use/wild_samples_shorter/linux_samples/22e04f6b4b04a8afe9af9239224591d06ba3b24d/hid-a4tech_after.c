	struct a4tech_sc *a4;
	int ret;

	a4 = devm_kzalloc(&hdev->dev, sizeof(*a4), GFP_KERNEL);
	if (a4 == NULL) {
		hid_err(hdev, "can't alloc device descriptor\n");
		return -ENOMEM;
	}

	a4->quirks = id->driver_data;

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

static const struct hid_device_id a4_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_A4TECH, USB_DEVICE_ID_A4TECH_WCP32PU),
	.input_mapped = a4_input_mapped,
	.event = a4_event,
	.probe = a4_probe,
};
module_hid_driver(a4_driver);

MODULE_LICENSE("GPL");