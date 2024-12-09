	list_for_each_entry(report, &report_enum->report_list, list) {
		field = report->field[0];
		if (report->maxfield && field &&
					field->physical)
			cnt++;
	}
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
	hid_hw_request(hsdev->hdev, report, HID_REQ_SET_REPORT);
	hid_hw_wait(hsdev->hdev);

done_proc:
	mutex_unlock(&data->mutex);

	return ret;
}
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
	hid_hw_wait(hsdev->hdev);
	*value = report->field[field_index]->value[0];

done_proc:
	mutex_unlock(&data->mutex);

	return ret;
}
	if (!report || (field_index >=  report->maxfield)) {
		ret = -EINVAL;
		goto done_proc;
	}
	hid_hw_request(hsdev->hdev, report, HID_REQ_GET_REPORT);
	hid_hw_wait(hsdev->hdev);
	*value = report->field[field_index]->value[0];

done_proc:
	mutex_unlock(&data->mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(sensor_hub_get_feature);


int sensor_hub_input_attr_get_raw_value(struct hid_sensor_hub_device *hsdev,
					u32 usage_id,
					u32 attr_usage_id, u32 report_id)
{
	struct sensor_hub_data *data =  hid_get_drvdata(hsdev->hdev);
	unsigned long flags;
	struct hid_report *report;
	int ret_val = 0;

	mutex_lock(&data->mutex);
	memset(&data->pending, 0, sizeof(data->pending));
	init_completion(&data->pending.ready);
	data->pending.usage_id = usage_id;
	data->pending.attr_usage_id = attr_usage_id;
	data->pending.raw_size = 0;

	spin_lock_irqsave(&data->lock, flags);
	data->pending.status = true;
	report = sensor_hub_report(report_id, hsdev->hdev, HID_INPUT_REPORT);
	if (!report) {
		spin_unlock_irqrestore(&data->lock, flags);
		goto err_free;
	}
	hid_hw_request(hsdev->hdev, report, HID_REQ_GET_REPORT);
	spin_unlock_irqrestore(&data->lock, flags);
	wait_for_completion_interruptible_timeout(&data->pending.ready, HZ*5);
	switch (data->pending.raw_size) {
	case 1:
		ret_val = *(u8 *)data->pending.raw_data;
		break;
	case 2:
		ret_val = *(u16 *)data->pending.raw_data;
		break;
	case 4:
		ret_val = *(u32 *)data->pending.raw_data;
		break;
	default:
		ret_val = 0;
	}
	kfree(data->pending.raw_data);

err_free:
	data->pending.status = false;
	mutex_unlock(&data->mutex);

	return ret_val;
}
{
	int ret = -1;
	int i, j;
	int collection_index = -1;
	struct hid_report *report;
	struct hid_field *field;
	struct hid_report_enum *report_enum;
	struct hid_device *hdev = hsdev->hdev;

	/* Initialize with defaults */
	info->usage_id = usage_id;
	info->attrib_id =  attr_usage_id;
	info->report_id = -1;
	info->index = -1;
	info->units = -1;
	info->unit_expo = -1;

	for (i = 0; i < hdev->maxcollection; ++i) {
		struct hid_collection *collection = &hdev->collection[i];
		if (usage_id == collection->usage) {
			collection_index = i;
			break;
		}
	}
				for (j = 0; j < field->maxusage; ++j) {
					if (field->usage[j].hid ==
					attr_usage_id &&
					field->usage[j].collection_index ==
					collection_index)  {
						sensor_hub_fill_attr_info(info,
							i, report->id,
							field->unit,
							field->unit_exponent,
							field->report_size);
						ret = 0;
						break;
					}
					collection_index)  {
						sensor_hub_fill_attr_info(info,
							i, report->id,
							field->unit,
							field->unit_exponent,
							field->report_size);
						ret = 0;
						break;
					}
				}
			}
			if (ret == 0)
				break;
		}
	}

err_ret:
	return ret;
}
EXPORT_SYMBOL_GPL(sensor_hub_input_get_attribute_info);

#ifdef CONFIG_PM
static int sensor_hub_suspend(struct hid_device *hdev, pm_message_t message)
{
	struct sensor_hub_data *pdata =  hid_get_drvdata(hdev);
	struct hid_sensor_hub_callbacks_list *callback;

	hid_dbg(hdev, " sensor_hub_suspend\n");
	spin_lock(&pdata->dyn_callback_lock);
	list_for_each_entry(callback, &pdata->dyn_callback_list, list) {
		if (callback->usage_callback->suspend)
			callback->usage_callback->suspend(
					pdata->hsdev, callback->priv);
	}
	spin_unlock(&pdata->dyn_callback_lock);

	return 0;
}
	list_for_each_entry(callback, &pdata->dyn_callback_list, list) {
		if (callback->usage_callback->suspend)
			callback->usage_callback->suspend(
					pdata->hsdev, callback->priv);
	}
	spin_unlock(&pdata->dyn_callback_lock);

	return 0;
}

static int sensor_hub_resume(struct hid_device *hdev)
{
	struct sensor_hub_data *pdata =  hid_get_drvdata(hdev);
	struct hid_sensor_hub_callbacks_list *callback;

	hid_dbg(hdev, " sensor_hub_resume\n");
	spin_lock(&pdata->dyn_callback_lock);
	list_for_each_entry(callback, &pdata->dyn_callback_list, list) {
		if (callback->usage_callback->resume)
			callback->usage_callback->resume(
					pdata->hsdev, callback->priv);
	}
	spin_unlock(&pdata->dyn_callback_lock);

	return 0;
}
{
	return 0;
}
#endif
/*
 * Handle raw report as sent by device
 */
static int sensor_hub_raw_event(struct hid_device *hdev,
		struct hid_report *report, u8 *raw_data, int size)
{
	int i;
	u8 *ptr;
	int sz;
	struct sensor_hub_data *pdata = hid_get_drvdata(hdev);
	unsigned long flags;
	struct hid_sensor_hub_callbacks *callback = NULL;
	struct hid_collection *collection = NULL;
	void *priv = NULL;

	hid_dbg(hdev, "sensor_hub_raw_event report id:0x%x size:%d type:%d\n",
			 report->id, size, report->type);
	hid_dbg(hdev, "maxfield:%d\n", report->maxfield);
	if (report->type != HID_INPUT_REPORT)
		return 1;

	ptr = raw_data;
	ptr++; /*Skip report id*/

	spin_lock_irqsave(&pdata->lock, flags);

	for (i = 0; i < report->maxfield; ++i) {

		hid_dbg(hdev, "%d collection_index:%x hid:%x sz:%x\n",
				i, report->field[i]->usage->collection_index,
				report->field[i]->usage->hid,
				report->field[i]->report_size/8);

		sz = report->field[i]->report_size/8;
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
				report->field[i]->usage->collection_index];
		hid_dbg(hdev, "collection->usage %x\n",
					collection->usage);
		callback = sensor_hub_get_callback(pdata->hsdev->hdev,
						report->field[i]->physical,
							&priv);
		if (callback && callback->capture_sample) {
			if (report->field[i]->logical)
				callback->capture_sample(pdata->hsdev,
					report->field[i]->logical, sz, ptr,
					callback->pdev);
			else
				callback->capture_sample(pdata->hsdev,
					report->field[i]->usage->hid, sz, ptr,
					callback->pdev);
		}
		ptr += sz;
	}
	if (callback && collection && callback->send_event)
		callback->send_event(pdata->hsdev, collection->usage,
				callback->pdev);
	spin_unlock_irqrestore(&pdata->lock, flags);

	return 1;
}
{
	int i;
	u8 *ptr;
	int sz;
	struct sensor_hub_data *pdata = hid_get_drvdata(hdev);
	unsigned long flags;
	struct hid_sensor_hub_callbacks *callback = NULL;
	struct hid_collection *collection = NULL;
	void *priv = NULL;

	hid_dbg(hdev, "sensor_hub_raw_event report id:0x%x size:%d type:%d\n",
			 report->id, size, report->type);
	hid_dbg(hdev, "maxfield:%d\n", report->maxfield);
	if (report->type != HID_INPUT_REPORT)
		return 1;

	ptr = raw_data;
	ptr++; /*Skip report id*/

	spin_lock_irqsave(&pdata->lock, flags);

	for (i = 0; i < report->maxfield; ++i) {

		hid_dbg(hdev, "%d collection_index:%x hid:%x sz:%x\n",
				i, report->field[i]->usage->collection_index,
				report->field[i]->usage->hid,
				report->field[i]->report_size/8);

		sz = report->field[i]->report_size/8;
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
				report->field[i]->usage->collection_index];
		hid_dbg(hdev, "collection->usage %x\n",
					collection->usage);
		callback = sensor_hub_get_callback(pdata->hsdev->hdev,
						report->field[i]->physical,
							&priv);
		if (callback && callback->capture_sample) {
			if (report->field[i]->logical)
				callback->capture_sample(pdata->hsdev,
					report->field[i]->logical, sz, ptr,
					callback->pdev);
			else
				callback->capture_sample(pdata->hsdev,
					report->field[i]->usage->hid, sz, ptr,
					callback->pdev);
		}
		ptr += sz;
	}
	if (callback && collection && callback->send_event)
		callback->send_event(pdata->hsdev, collection->usage,
				callback->pdev);
	spin_unlock_irqrestore(&pdata->lock, flags);

	return 1;
}
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
				report->field[i]->usage->collection_index];
		hid_dbg(hdev, "collection->usage %x\n",
					collection->usage);
		callback = sensor_hub_get_callback(pdata->hsdev->hdev,
						report->field[i]->physical,
							&priv);
		if (callback && callback->capture_sample) {
			if (report->field[i]->logical)
				callback->capture_sample(pdata->hsdev,
					report->field[i]->logical, sz, ptr,
					callback->pdev);
			else
				callback->capture_sample(pdata->hsdev,
					report->field[i]->usage->hid, sz, ptr,
					callback->pdev);
		}
		ptr += sz;
	}
{
	int ret;
	struct sensor_hub_data *sd;
	int i;
	char *name;
	struct hid_report *report;
	struct hid_report_enum *report_enum;
	struct hid_field *field;
	int dev_cnt;

	sd = kzalloc(sizeof(struct sensor_hub_data), GFP_KERNEL);
	if (!sd) {
		hid_err(hdev, "cannot allocate Sensor data\n");
		return -ENOMEM;
	}
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}
					field->physical) {
			name = kasprintf(GFP_KERNEL, "HID-SENSOR-%x",
						field->physical);
			if (name  == NULL) {
				hid_err(hdev, "Failed MFD device name\n");
					ret = -ENOMEM;
					goto err_free_names;
			}
			if (name  == NULL) {
				hid_err(hdev, "Failed MFD device name\n");
					ret = -ENOMEM;
					goto err_free_names;
			}
			sd->hid_sensor_hub_client_devs[
				sd->hid_sensor_client_cnt].name = name;
			sd->hid_sensor_hub_client_devs[
				sd->hid_sensor_client_cnt].platform_data =
						sd->hsdev;
			sd->hid_sensor_hub_client_devs[
				sd->hid_sensor_client_cnt].pdata_size =
						sizeof(*sd->hsdev);
			hid_dbg(hdev, "Adding %s:%p\n", name, sd);
			sd->hid_sensor_client_cnt++;
		}
	}
	ret = mfd_add_devices(&hdev->dev, 0, sd->hid_sensor_hub_client_devs,
		sd->hid_sensor_client_cnt, NULL, 0, NULL);
	if (ret < 0)
		goto err_free_names;

	return ret;

err_free_names:
	for (i = 0; i < sd->hid_sensor_client_cnt ; ++i)
		kfree(sd->hid_sensor_hub_client_devs[i].name);
	kfree(sd->hid_sensor_hub_client_devs);
err_close:
	hid_hw_close(hdev);
err_stop_hw:
	hid_hw_stop(hdev);
err_free:
	kfree(sd->hsdev);
err_free_hub:
	kfree(sd);

	return ret;
}

static void sensor_hub_remove(struct hid_device *hdev)
{
	struct sensor_hub_data *data = hid_get_drvdata(hdev);
	unsigned long flags;
	int i;

	hid_dbg(hdev, " hardware removed\n");
	hid_hw_close(hdev);
	hid_hw_stop(hdev);
	spin_lock_irqsave(&data->lock, flags);
	if (data->pending.status)
		complete(&data->pending.ready);
	spin_unlock_irqrestore(&data->lock, flags);
	mfd_remove_devices(&hdev->dev);
	for (i = 0; i < data->hid_sensor_client_cnt ; ++i)
		kfree(data->hid_sensor_hub_client_devs[i].name);
	kfree(data->hid_sensor_hub_client_devs);
	hid_set_drvdata(hdev, NULL);
	mutex_destroy(&data->mutex);
	kfree(data->hsdev);
	kfree(data);
}

static const struct hid_device_id sensor_hub_devices[] = {
	{ HID_DEVICE(HID_BUS_ANY, HID_GROUP_SENSOR_HUB, HID_ANY_ID,
		     HID_ANY_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, sensor_hub_devices);

static struct hid_driver sensor_hub_driver = {
	.name = "hid-sensor-hub",
	.id_table = sensor_hub_devices,
	.probe = sensor_hub_probe,
	.remove = sensor_hub_remove,
	.raw_event = sensor_hub_raw_event,
#ifdef CONFIG_PM
	.suspend = sensor_hub_suspend,
	.resume =  sensor_hub_resume,
	.reset_resume =  sensor_hub_reset_resume,
#endif
};
module_hid_driver(sensor_hub_driver);

MODULE_DESCRIPTION("HID Sensor Hub driver");
MODULE_AUTHOR("Srinivas Pandruvada <srinivas.pandruvada@intel.com>");
MODULE_LICENSE("GPL");
{
	struct sensor_hub_data *data = hid_get_drvdata(hdev);
	unsigned long flags;
	int i;

	hid_dbg(hdev, " hardware removed\n");
	hid_hw_close(hdev);
	hid_hw_stop(hdev);
	spin_lock_irqsave(&data->lock, flags);
	if (data->pending.status)
		complete(&data->pending.ready);
	spin_unlock_irqrestore(&data->lock, flags);
	mfd_remove_devices(&hdev->dev);
	for (i = 0; i < data->hid_sensor_client_cnt ; ++i)
		kfree(data->hid_sensor_hub_client_devs[i].name);
	kfree(data->hid_sensor_hub_client_devs);
	hid_set_drvdata(hdev, NULL);
	mutex_destroy(&data->mutex);
	kfree(data->hsdev);
	kfree(data);
}

static const struct hid_device_id sensor_hub_devices[] = {
	{ HID_DEVICE(HID_BUS_ANY, HID_GROUP_SENSOR_HUB, HID_ANY_ID,
		     HID_ANY_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, sensor_hub_devices);

static struct hid_driver sensor_hub_driver = {
	.name = "hid-sensor-hub",
	.id_table = sensor_hub_devices,
	.probe = sensor_hub_probe,
	.remove = sensor_hub_remove,
	.raw_event = sensor_hub_raw_event,
#ifdef CONFIG_PM
	.suspend = sensor_hub_suspend,
	.resume =  sensor_hub_resume,
	.reset_resume =  sensor_hub_reset_resume,
#endif
};
module_hid_driver(sensor_hub_driver);

MODULE_DESCRIPTION("HID Sensor Hub driver");
MODULE_AUTHOR("Srinivas Pandruvada <srinivas.pandruvada@intel.com>");
MODULE_LICENSE("GPL");
static struct hid_driver sensor_hub_driver = {
	.name = "hid-sensor-hub",
	.id_table = sensor_hub_devices,
	.probe = sensor_hub_probe,
	.remove = sensor_hub_remove,
	.raw_event = sensor_hub_raw_event,
#ifdef CONFIG_PM
	.suspend = sensor_hub_suspend,
	.resume =  sensor_hub_resume,
	.reset_resume =  sensor_hub_reset_resume,
#endif
};
module_hid_driver(sensor_hub_driver);

MODULE_DESCRIPTION("HID Sensor Hub driver");
MODULE_AUTHOR("Srinivas Pandruvada <srinivas.pandruvada@intel.com>");
MODULE_LICENSE("GPL");