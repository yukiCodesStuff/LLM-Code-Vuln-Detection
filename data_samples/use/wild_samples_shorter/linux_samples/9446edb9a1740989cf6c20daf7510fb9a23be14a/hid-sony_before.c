	drv_data = hid_get_drvdata(hdev);
	BUG_ON(!(drv_data->quirks & BUZZ_CONTROLLER));

	buzz = kzalloc(sizeof(*buzz), GFP_KERNEL);
	if (!buzz) {
		hid_err(hdev, "Insufficient memory, cannot allocate driver data\n");
		return -ENOMEM;