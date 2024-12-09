{
	struct hid_device *dev = container_of(psy, struct hid_device, battery);
	int ret = 0;
	__u8 buf[2] = {};

	switch (prop) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		ret = dev->hid_get_raw_report(dev, dev->battery_report_id,
					      buf, sizeof(buf),
					      dev->battery_report_type);

		if (ret != 2) {
			ret = -ENODATA;
			break;
		}
		ret = 0;

		    buf[1] <= dev->battery_max)
			val->intval = (100 * (buf[1] - dev->battery_min)) /
				(dev->battery_max - dev->battery_min);
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = dev->name;
}
EXPORT_SYMBOL_GPL(hidinput_count_leds);

static int hidinput_open(struct input_dev *dev)
{
	struct hid_device *hid = input_get_drvdata(dev);

	}

	input_set_drvdata(input_dev, hid);
	input_dev->event = hid->ll_driver->hidinput_input_event;
	input_dev->open = hidinput_open;
	input_dev->close = hidinput_close;
	input_dev->setkeycode = hidinput_setkeycode;
	input_dev->getkeycode = hidinput_getkeycode;
	int i, j, k;

	INIT_LIST_HEAD(&hid->inputs);

	if (!force) {
		for (i = 0; i < hid->maxcollection; i++) {
			struct hid_collection *col = &hid->collection[i];
		input_unregister_device(hidinput->input);
		kfree(hidinput);
	}
}
EXPORT_SYMBOL_GPL(hidinput_disconnect);
