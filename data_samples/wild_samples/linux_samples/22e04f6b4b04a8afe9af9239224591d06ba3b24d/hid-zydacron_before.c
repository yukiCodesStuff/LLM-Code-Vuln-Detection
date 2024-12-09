{
	int ret;
	struct zc_device *zc;

	zc = kzalloc(sizeof(*zc), GFP_KERNEL);
	if (zc == NULL) {
		hid_err(hdev, "can't alloc descriptor\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}
static struct hid_driver zc_driver = {
	.name = "zydacron",
	.id_table = zc_devices,
	.report_fixup = zc_report_fixup,
	.input_mapping = zc_input_mapping,
	.raw_event = zc_raw_event,
	.probe = zc_probe,
	.remove = zc_remove,
};
module_hid_driver(zc_driver);

MODULE_LICENSE("GPL");