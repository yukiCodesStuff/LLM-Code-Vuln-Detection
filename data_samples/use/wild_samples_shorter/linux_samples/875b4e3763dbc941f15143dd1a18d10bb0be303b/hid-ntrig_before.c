	struct hid_report *report = hdev->report_enum[HID_FEATURE_REPORT].
				    report_id_hash[0x0d];

	if (!report)
		return -EINVAL;

	hid_hw_request(hdev, report, HID_REQ_GET_REPORT);
	hid_hw_wait(hdev);