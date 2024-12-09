	struct hid_report *report = hdev->report_enum[HID_FEATURE_REPORT].
				    report_id_hash[0x0d];

	if (!report)
		return -EINVAL;

	hid_hw_request(hdev, report, HID_REQ_GET_REPORT);
	hid_hw_wait(hdev);

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	if (val > nd->sensor_physical_width)
		return -EINVAL;

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	if (val > nd->sensor_physical_height)
		return -EINVAL;

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	if (val > 0x7f)
		return -EINVAL;

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	if (val > nd->sensor_physical_width)
		return -EINVAL;

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	if (val > nd->sensor_physical_height)
		return -EINVAL;

	unsigned long val;

	if (strict_strtoul(buf, 0, &val))
		return -EINVAL;

	/*
	 * No more than 8 terminal frames have been observed so far