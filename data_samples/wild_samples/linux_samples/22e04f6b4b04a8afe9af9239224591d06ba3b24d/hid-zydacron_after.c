{
	int ret;
	struct zc_device *zc;

	zc = devm_kzalloc(&hdev->dev, sizeof(*zc), GFP_KERNEL);
	if (zc == NULL) {
		hid_err(hdev, "can't alloc descriptor\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}
static struct hid_driver zc_driver = {
	.name = "zydacron",
	.id_table = zc_devices,
	.report_fixup = zc_report_fixup,
	.input_mapping = zc_input_mapping,
	.raw_event = zc_raw_event,
	.probe = zc_probe,
};
module_hid_driver(zc_driver);

MODULE_LICENSE("GPL");