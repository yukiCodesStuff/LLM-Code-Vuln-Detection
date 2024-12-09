	struct a4tech_sc *a4;
	int ret;

	a4 = kzalloc(sizeof(*a4), GFP_KERNEL);
	if (a4 == NULL) {
		hid_err(hdev, "can't alloc device descriptor\n");
		ret = -ENOMEM;
		goto err_free;
	}

	a4->quirks = id->driver_data;

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
	kfree(a4);
	return ret;
}

static void a4_remove(struct hid_device *hdev)
{
	struct a4tech_sc *a4 = hid_get_drvdata(hdev);

	hid_hw_stop(hdev);
	kfree(a4);
}

static const struct hid_device_id a4_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_A4TECH, USB_DEVICE_ID_A4TECH_WCP32PU),
	.input_mapped = a4_input_mapped,
	.event = a4_event,
	.probe = a4_probe,
	.remove = a4_remove,
};
module_hid_driver(a4_driver);

MODULE_LICENSE("GPL");