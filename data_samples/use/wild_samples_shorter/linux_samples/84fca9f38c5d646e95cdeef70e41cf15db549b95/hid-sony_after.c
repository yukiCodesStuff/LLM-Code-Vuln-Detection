	drv_data = hid_get_drvdata(hdev);
	BUG_ON(!(drv_data->quirks & BUZZ_CONTROLLER));

	/* Validate expected report characteristics. */
	if (!hid_validate_values(hdev, HID_OUTPUT_REPORT, 0, 0, 7))
		return -ENODEV;

	buzz = kzalloc(sizeof(*buzz), GFP_KERNEL);
	if (!buzz) {
		hid_err(hdev, "Insufficient memory, cannot allocate driver data\n");
		return -ENOMEM;