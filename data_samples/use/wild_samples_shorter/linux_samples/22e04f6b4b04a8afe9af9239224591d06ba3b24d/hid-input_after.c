{
	struct hid_device *dev = container_of(psy, struct hid_device, battery);
	int ret = 0;
	__u8 *buf;

	switch (prop) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		break;

	case POWER_SUPPLY_PROP_CAPACITY:

		buf = kmalloc(2 * sizeof(__u8), GFP_KERNEL);
		if (!buf) {
			ret = -ENOMEM;
			break;
		}
		ret = dev->hid_get_raw_report(dev, dev->battery_report_id,
					      buf, 2,
					      dev->battery_report_type);

		if (ret != 2) {
			ret = -ENODATA;
			kfree(buf);
			break;
		}
		ret = 0;

		    buf[1] <= dev->battery_max)
			val->intval = (100 * (buf[1] - dev->battery_min)) /
				(dev->battery_max - dev->battery_min);
		kfree(buf);
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = dev->name;
}
EXPORT_SYMBOL_GPL(hidinput_count_leds);

static void hidinput_led_worker(struct work_struct *work)
{
	struct hid_device *hid = container_of(work, struct hid_device,
					      led_work);
	struct hid_field *field;
	struct hid_report *report;
	int len;
	__u8 *buf;

	field = hidinput_get_led_field(hid);
	if (!field)
		return;

	/*
	 * field->report is accessed unlocked regarding HID core. So there might
	 * be another incoming SET-LED request from user-space, which changes
	 * the LED state while we assemble our outgoing buffer. However, this
	 * doesn't matter as hid_output_report() correctly converts it into a
	 * boolean value no matter what information is currently set on the LED
	 * field (even garbage). So the remote device will always get a valid
	 * request.
	 * And in case we send a wrong value, a next led worker is spawned
	 * for every SET-LED request so the following worker will send the
	 * correct value, guaranteed!
	 */

	report = field->report;

	/* use custom SET_REPORT request if possible (asynchronous) */
	if (hid->ll_driver->request)
		return hid->ll_driver->request(hid, report, HID_REQ_SET_REPORT);

	/* fall back to generic raw-output-report */
	len = ((report->size - 1) >> 3) + 1 + (report->id > 0);
	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return;

	hid_output_report(report, buf);
	/* synchronous output report */
	hid->hid_output_raw_report(hid, buf, len, HID_OUTPUT_REPORT);
	kfree(buf);
}

static int hidinput_input_event(struct input_dev *dev, unsigned int type,
				unsigned int code, int value)
{
	struct hid_device *hid = input_get_drvdata(dev);
	struct hid_field *field;
	int offset;

	if (type == EV_FF)
		return input_ff_event(dev, type, code, value);

	if (type != EV_LED)
		return -1;

	if ((offset = hidinput_find_field(hid, type, code, &field)) == -1) {
		hid_warn(dev, "event field not found\n");
		return -1;
	}

	hid_set_field(field, offset, value);

	schedule_work(&hid->led_work);
	return 0;
}

static int hidinput_open(struct input_dev *dev)
{
	struct hid_device *hid = input_get_drvdata(dev);

	}

	input_set_drvdata(input_dev, hid);
	if (hid->ll_driver->hidinput_input_event)
		input_dev->event = hid->ll_driver->hidinput_input_event;
	else if (hid->ll_driver->request || hid->hid_output_raw_report)
		input_dev->event = hidinput_input_event;
	input_dev->open = hidinput_open;
	input_dev->close = hidinput_close;
	input_dev->setkeycode = hidinput_setkeycode;
	input_dev->getkeycode = hidinput_getkeycode;
	int i, j, k;

	INIT_LIST_HEAD(&hid->inputs);
	INIT_WORK(&hid->led_work, hidinput_led_worker);

	if (!force) {
		for (i = 0; i < hid->maxcollection; i++) {
			struct hid_collection *col = &hid->collection[i];
		input_unregister_device(hidinput->input);
		kfree(hidinput);
	}

	/* led_work is spawned by input_dev callbacks, but doesn't access the
	 * parent input_dev at all. Once all input devices are removed, we
	 * know that led_work will never get restarted, so we can cancel it
	 * synchronously and are safe. */
	cancel_work_sync(&hid->led_work);
}
EXPORT_SYMBOL_GPL(hidinput_disconnect);
