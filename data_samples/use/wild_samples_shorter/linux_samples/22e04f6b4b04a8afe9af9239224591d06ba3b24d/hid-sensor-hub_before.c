
	list_for_each_entry(report, &report_enum->report_list, list) {
		field = report->field[0];
		if (report->maxfield && field &&
					field->physical)
			cnt++;
	}

	return cnt;
				u32 field_index, s32 value)
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
	hid_set_field(report->field[field_index], 0, value);
				u32 field_index, s32 *value)
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
	hid_hw_request(hsdev->hdev, report, HID_REQ_GET_REPORT);
					u32 usage_id,
					u32 attr_usage_id, u32 report_id)
{
	struct sensor_hub_data *data =  hid_get_drvdata(hsdev->hdev);
	unsigned long flags;
	struct hid_report *report;
	int ret_val = 0;


	/* Initialize with defaults */
	info->usage_id = usage_id;
	info->attrib_id =  attr_usage_id;
	info->report_id = -1;
	info->index = -1;
	info->units = -1;
	info->unit_expo = -1;
					if (field->usage[j].hid ==
					attr_usage_id &&
					field->usage[j].collection_index ==
					collection_index)  {
						sensor_hub_fill_attr_info(info,
							i, report->id,
							field->unit,
							field->unit_exponent,
#ifdef CONFIG_PM
static int sensor_hub_suspend(struct hid_device *hdev, pm_message_t message)
{
	struct sensor_hub_data *pdata =  hid_get_drvdata(hdev);
	struct hid_sensor_hub_callbacks_list *callback;

	hid_dbg(hdev, " sensor_hub_suspend\n");
	spin_lock(&pdata->dyn_callback_lock);

static int sensor_hub_resume(struct hid_device *hdev)
{
	struct sensor_hub_data *pdata =  hid_get_drvdata(hdev);
	struct hid_sensor_hub_callbacks_list *callback;

	hid_dbg(hdev, " sensor_hub_resume\n");
	spin_lock(&pdata->dyn_callback_lock);
	return 0;
}
#endif
/*
 * Handle raw report as sent by device
 */
static int sensor_hub_raw_event(struct hid_device *hdev,
		return 1;

	ptr = raw_data;
	ptr++; /*Skip report id*/

	spin_lock_irqsave(&pdata->lock, flags);

	for (i = 0; i < report->maxfield; ++i) {

		hid_dbg(hdev, "%d collection_index:%x hid:%x sz:%x\n",
				i, report->field[i]->usage->collection_index,
				report->field[i]->usage->hid,
				report->field[i]->report_size/8);
		if (pdata->pending.status && pdata->pending.attr_usage_id ==
				report->field[i]->usage->hid) {
			hid_dbg(hdev, "data was pending ...\n");
			pdata->pending.raw_data = kmalloc(sz, GFP_ATOMIC);
			if (pdata->pending.raw_data) {
				memcpy(pdata->pending.raw_data, ptr, sz);
				pdata->pending.raw_size  = sz;
			} else
				pdata->pending.raw_size = 0;
			complete(&pdata->pending.ready);
		}
		collection = &hdev->collection[
	struct hid_field *field;
	int dev_cnt;

	sd = kzalloc(sizeof(struct sensor_hub_data), GFP_KERNEL);
	if (!sd) {
		hid_err(hdev, "cannot allocate Sensor data\n");
		return -ENOMEM;
	}
	sd->hsdev = kzalloc(sizeof(struct hid_sensor_hub_device), GFP_KERNEL);
	if (!sd->hsdev) {
		hid_err(hdev, "cannot allocate hid_sensor_hub_device\n");
		ret = -ENOMEM;
		goto err_free_hub;
	}
	hid_set_drvdata(hdev, sd);
	sd->hsdev->hdev = hdev;
	sd->hsdev->vendor_id = hdev->vendor;
	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}
	INIT_LIST_HEAD(&hdev->inputs);

	ret = hid_hw_start(hdev, 0);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		goto err_free;
	}
	ret = hid_hw_open(hdev);
	if (ret) {
		hid_err(hdev, "failed to open input interrupt pipe\n");
					field->physical) {
			name = kasprintf(GFP_KERNEL, "HID-SENSOR-%x",
						field->physical);
			if (name  == NULL) {
				hid_err(hdev, "Failed MFD device name\n");
					ret = -ENOMEM;
					goto err_free_names;
			}
	hid_hw_close(hdev);
err_stop_hw:
	hid_hw_stop(hdev);
err_free:
	kfree(sd->hsdev);
err_free_hub:
	kfree(sd);

	return ret;
}

	kfree(data->hid_sensor_hub_client_devs);
	hid_set_drvdata(hdev, NULL);
	mutex_destroy(&data->mutex);
	kfree(data->hsdev);
	kfree(data);
}

static const struct hid_device_id sensor_hub_devices[] = {
	{ HID_DEVICE(HID_BUS_ANY, HID_GROUP_SENSOR_HUB, HID_ANY_ID,
	.raw_event = sensor_hub_raw_event,
#ifdef CONFIG_PM
	.suspend = sensor_hub_suspend,
	.resume =  sensor_hub_resume,
	.reset_resume =  sensor_hub_reset_resume,
#endif
};
module_hid_driver(sensor_hub_driver);
