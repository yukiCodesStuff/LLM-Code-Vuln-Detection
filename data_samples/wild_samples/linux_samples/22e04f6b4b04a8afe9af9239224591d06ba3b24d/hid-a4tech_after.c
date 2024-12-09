{
	struct a4tech_sc *a4;
	int ret;

	a4 = devm_kzalloc(&hdev->dev, sizeof(*a4), GFP_KERNEL);
	if (a4 == NULL) {
		hid_err(hdev, "can't alloc device descriptor\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}
static struct hid_driver a4_driver = {
	.name = "a4tech",
	.id_table = a4_devices,
	.input_mapped = a4_input_mapped,
	.event = a4_event,
	.probe = a4_probe,
};
module_hid_driver(a4_driver);

MODULE_LICENSE("GPL");