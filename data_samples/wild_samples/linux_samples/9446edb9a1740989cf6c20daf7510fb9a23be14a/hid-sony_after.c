{
	struct sony_sc *drv_data;
	struct buzz_extra *buzz;
	int n, ret = 0;
	struct led_classdev *led;
	size_t name_sz;
	char *name;

	drv_data = hid_get_drvdata(hdev);
	BUG_ON(!(drv_data->quirks & BUZZ_CONTROLLER));

	/* Validate expected report characteristics. */
	if (!hid_validate_values(hdev, HID_OUTPUT_REPORT, 0, 0, 7))
		return -ENODEV;

	buzz = kzalloc(sizeof(*buzz), GFP_KERNEL);
	if (!buzz) {
		hid_err(hdev, "Insufficient memory, cannot allocate driver data\n");
		return -ENOMEM;
	}