	} else if (cnt >= 10 && strncmp("bootloader", buf, 10) == 0) {
		if (!(data->status & PICOLCD_BOOTLOADER))
			report = picolcd_out_report(REPORT_EXIT_KEYBOARD, data->hdev);
		buf += 10;
		cnt -= 10;
	}
	if (!report)
		return -EINVAL;

	while (cnt > 0 && (buf[cnt-1] == '\n' || buf[cnt-1] == '\r'))
		cnt--;
	if (cnt != 0)
		return -EINVAL;

	spin_lock_irqsave(&data->lock, flags);
	hid_set_field(report->field[0], 0, timeout & 0xff);
	hid_set_field(report->field[0], 1, (timeout >> 8) & 0xff);
	hid_hw_request(data->hdev, report, HID_REQ_SET_REPORT);
	spin_unlock_irqrestore(&data->lock, flags);
	return count;
}

static DEVICE_ATTR(operation_mode, 0644, picolcd_operation_mode_show,
		picolcd_operation_mode_store);

/*
 * The "operation_mode_delay" sysfs attribute
 */
static ssize_t picolcd_operation_mode_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct picolcd_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%hu\n", data->opmode_delay);
}

static ssize_t picolcd_operation_mode_delay_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct picolcd_data *data = dev_get_drvdata(dev);
	unsigned u;
	if (sscanf(buf, "%u", &u) != 1)
		return -EINVAL;
	if (u > 30000)
		return -EINVAL;
	else
		data->opmode_delay = u;
	return count;
}

static DEVICE_ATTR(operation_mode_delay, 0644, picolcd_operation_mode_delay_show,
		picolcd_operation_mode_delay_store);

/*
 * Handle raw report as sent by device
 */
static int picolcd_raw_event(struct hid_device *hdev,
		struct hid_report *report, u8 *raw_data, int size)
{
	struct picolcd_data *data = hid_get_drvdata(hdev);
	unsigned long flags;
	int ret = 0;

	if (!data)
		return 1;

	if (report->id == REPORT_KEY_STATE) {
		if (data->input_keys)
			ret = picolcd_raw_keypad(data, report, raw_data+1, size-1);
	} else if (report->id == REPORT_IR_DATA) {
		ret = picolcd_raw_cir(data, report, raw_data+1, size-1);
	} else {
		spin_lock_irqsave(&data->lock, flags);
		/*
		 * We let the caller of picolcd_send_and_wait() check if the
		 * report we got is one of the expected ones or not.
		 */
		if (data->pending) {
			memcpy(data->pending->raw_data, raw_data+1, size-1);
			data->pending->raw_size  = size-1;
			data->pending->in_report = report;
			complete(&data->pending->ready);
		}
		spin_unlock_irqrestore(&data->lock, flags);
	}

	picolcd_debug_raw_event(data, hdev, report, raw_data, size);
	return 1;
}