{
	struct a4tech_sc *a4;
	int ret;

	a4 = kzalloc(sizeof(*a4), GFP_KERNEL);
	if (a4 == NULL) {
		hid_err(hdev, "can't alloc device descriptor\n");
		ret = -ENOMEM;
		goto err_free;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}
static struct hid_driver a4_driver = {
	.name = "a4tech",
	.id_table = a4_devices,
	.input_mapped = a4_input_mapped,
	.event = a4_event,
	.probe = a4_probe,
	.remove = a4_remove,
};
module_hid_driver(a4_driver);

MODULE_LICENSE("GPL");