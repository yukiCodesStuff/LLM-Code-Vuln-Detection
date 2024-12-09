{
	struct hid_report *report;
	struct sensor_hub_data *data =  hid_get_drvdata(hsdev->hdev);
	int ret = 0;

	mutex_lock(&data->mutex);
	report = sensor_hub_report(report_id, hsdev->hdev, HID_FEATURE_REPORT);
	if (!report || (field_index >=  report->maxfield)) {
		ret = -EINVAL;
		goto done_proc;
	}