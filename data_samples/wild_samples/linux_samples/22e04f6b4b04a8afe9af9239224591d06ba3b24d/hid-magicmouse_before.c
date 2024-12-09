static int param_set_scroll_speed(const char *val, struct kernel_param *kp) {
	unsigned long speed;
	if (!val || strict_strtoul(val, 0, &speed) || speed > 63)
		return -EINVAL;
	scroll_speed = speed;
	return 0;
}
{
	__u8 feature[] = { 0xd7, 0x01 };
	struct magicmouse_sc *msc;
	struct hid_report *report;
	int ret;

	msc = kzalloc(sizeof(*msc), GFP_KERNEL);
	if (msc == NULL) {
		hid_err(hdev, "can't alloc magicmouse descriptor\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "magicmouse hid parse failed\n");
		goto err_free;
	}
	if (ret != -EIO && ret != sizeof(feature)) {
		hid_err(hdev, "unable to request touch data (%d)\n", ret);
		goto err_stop_hw;
	}

	return 0;
err_stop_hw:
	hid_hw_stop(hdev);
err_free:
	kfree(msc);
	return ret;
}

static void magicmouse_remove(struct hid_device *hdev)
{
	struct magicmouse_sc *msc = hid_get_drvdata(hdev);

	hid_hw_stop(hdev);
	kfree(msc);
}

static const struct hid_device_id magic_mice[] = {
	{ HID_BLUETOOTH_DEVICE(USB_VENDOR_ID_APPLE,
		USB_DEVICE_ID_APPLE_MAGICMOUSE), .driver_data = 0 },
	{ HID_BLUETOOTH_DEVICE(USB_VENDOR_ID_APPLE,
		USB_DEVICE_ID_APPLE_MAGICTRACKPAD), .driver_data = 0 },
	{ }
};
MODULE_DEVICE_TABLE(hid, magic_mice);

static struct hid_driver magicmouse_driver = {
	.name = "magicmouse",
	.id_table = magic_mice,
	.probe = magicmouse_probe,
	.remove = magicmouse_remove,
	.raw_event = magicmouse_raw_event,
	.input_mapping = magicmouse_input_mapping,
	.input_configured = magicmouse_input_configured,
};
module_hid_driver(magicmouse_driver);

MODULE_LICENSE("GPL");
static struct hid_driver magicmouse_driver = {
	.name = "magicmouse",
	.id_table = magic_mice,
	.probe = magicmouse_probe,
	.remove = magicmouse_remove,
	.raw_event = magicmouse_raw_event,
	.input_mapping = magicmouse_input_mapping,
	.input_configured = magicmouse_input_configured,
};
module_hid_driver(magicmouse_driver);

MODULE_LICENSE("GPL");