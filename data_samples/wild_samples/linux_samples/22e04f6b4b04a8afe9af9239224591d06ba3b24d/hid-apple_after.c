{
	unsigned long quirks = id->driver_data;
	struct apple_sc *asc;
	unsigned int connect_mask = HID_CONNECT_DEFAULT;
	int ret;

	asc = devm_kzalloc(&hdev->dev, sizeof(*asc), GFP_KERNEL);
	if (asc == NULL) {
		hid_err(hdev, "can't alloc apple descriptor\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}
static struct hid_driver apple_driver = {
	.name = "apple",
	.id_table = apple_devices,
	.report_fixup = apple_report_fixup,
	.probe = apple_probe,
	.event = apple_event,
	.input_mapping = apple_input_mapping,
	.input_mapped = apple_input_mapped,
};
module_hid_driver(apple_driver);

MODULE_LICENSE("GPL");