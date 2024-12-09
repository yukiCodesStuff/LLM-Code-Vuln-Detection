{
	struct hid_device *dev = container_of(psy, struct hid_device, battery);
	int ret = 0;
	__u8 buf[2] = {};

	switch (prop) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 1;
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

		if (dev->battery_min < dev->battery_max &&
		    buf[1] >= dev->battery_min &&
		    buf[1] <= dev->battery_max)
			val->intval = (100 * (buf[1] - dev->battery_min)) /
				(dev->battery_max - dev->battery_min);
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = dev->name;
		break;

	case POWER_SUPPLY_PROP_STATUS:
		val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		break;

	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = POWER_SUPPLY_SCOPE_DEVICE;
		break;

	default:
		ret = -EINVAL;
		break;
	}
	switch (prop) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 1;
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		ret = dev->hid_get_raw_report(dev, dev->battery_report_id,
					      buf, sizeof(buf),
					      dev->battery_report_type);

		if (ret != 2) {
			ret = -ENODATA;
			break;
		}
		if (ret != 2) {
			ret = -ENODATA;
			break;
		}
		ret = 0;

		if (dev->battery_min < dev->battery_max &&
		    buf[1] >= dev->battery_min &&
		    buf[1] <= dev->battery_max)
			val->intval = (100 * (buf[1] - dev->battery_min)) /
				(dev->battery_max - dev->battery_min);
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = dev->name;
		break;

	case POWER_SUPPLY_PROP_STATUS:
		val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		break;

	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = POWER_SUPPLY_SCOPE_DEVICE;
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static bool hidinput_setup_battery(struct hid_device *dev, unsigned report_type, struct hid_field *field)
{
	struct power_supply *battery = &dev->battery;
	int ret;
	unsigned quirks;
	s32 min, max;

	if (field->usage->hid != HID_DC_BATTERYSTRENGTH)
		return false;	/* no match */

	if (battery->name != NULL)
		goto out;	/* already initialized? */

	battery->name = kasprintf(GFP_KERNEL, "hid-%s-battery", dev->uniq);
	if (battery->name == NULL)
		goto out;

	battery->type = POWER_SUPPLY_TYPE_BATTERY;
	battery->properties = hidinput_battery_props;
	battery->num_properties = ARRAY_SIZE(hidinput_battery_props);
	battery->use_for_apm = 0;
	battery->get_property = hidinput_get_battery_property;

	quirks = find_battery_quirk(dev);

	hid_dbg(dev, "device %x:%x:%x %d quirks %d\n",
		dev->bus, dev->vendor, dev->product, dev->version, quirks);

	min = field->logical_minimum;
	max = field->logical_maximum;

	if (quirks & HID_BATTERY_QUIRK_PERCENT) {
		min = 0;
		max = 100;
	}

	if (quirks & HID_BATTERY_QUIRK_FEATURE)
		report_type = HID_FEATURE_REPORT;

	dev->battery_min = min;
	dev->battery_max = max;
	dev->battery_report_type = report_type;
	dev->battery_report_id = field->report->id;

	ret = power_supply_register(&dev->dev, battery);
	if (ret != 0) {
		hid_warn(dev, "can't register power supply: %d\n", ret);
		kfree(battery->name);
		battery->name = NULL;
	}

	power_supply_powers(battery, &dev->dev);

out:
	return true;
}

static void hidinput_cleanup_battery(struct hid_device *dev)
{
	if (!dev->battery.name)
		return;

	power_supply_unregister(&dev->battery);
	kfree(dev->battery.name);
	dev->battery.name = NULL;
}
#else  /* !CONFIG_HID_BATTERY_STRENGTH */
static bool hidinput_setup_battery(struct hid_device *dev, unsigned report_type,
				   struct hid_field *field)
{
	return false;
}

static void hidinput_cleanup_battery(struct hid_device *dev)
{
}
#endif	/* CONFIG_HID_BATTERY_STRENGTH */

static void hidinput_configure_usage(struct hid_input *hidinput, struct hid_field *field,
				     struct hid_usage *usage)
{
	struct input_dev *input = hidinput->input;
	struct hid_device *device = input_get_drvdata(input);
	int max = 0, code;
	unsigned long *bit = NULL;

	field->hidinput = hidinput;

	if (field->flags & HID_MAIN_ITEM_CONSTANT)
		goto ignore;

	/* only LED usages are supported in output fields */
	if (field->report_type == HID_OUTPUT_REPORT &&
			(usage->hid & HID_USAGE_PAGE) != HID_UP_LED) {
		goto ignore;
	}

	if (device->driver->input_mapping) {
		int ret = device->driver->input_mapping(device, hidinput, field,
				usage, &bit, &max);
		if (ret > 0)
			goto mapped;
		if (ret < 0)
			goto ignore;
	}

	switch (usage->hid & HID_USAGE_PAGE) {
	case HID_UP_UNDEFINED:
		goto ignore;

	case HID_UP_KEYBOARD:
		set_bit(EV_REP, input->evbit);

		if ((usage->hid & HID_USAGE) < 256) {
			if (!hid_keyboard[usage->hid & HID_USAGE]) goto ignore;
			map_key_clear(hid_keyboard[usage->hid & HID_USAGE]);
		} else
			map_key(KEY_UNKNOWN);

		break;

	case HID_UP_BUTTON:
		code = ((usage->hid - 1) & HID_USAGE);

		switch (field->application) {
		case HID_GD_MOUSE:
		case HID_GD_POINTER:  code += BTN_MOUSE; break;
		case HID_GD_JOYSTICK:
				if (code <= 0xf)
					code += BTN_JOYSTICK;
				else
					code += BTN_TRIGGER_HAPPY - 0x10;
				break;
		case HID_GD_GAMEPAD:
				if (code <= 0xf)
					code += BTN_GAMEPAD;
				else
					code += BTN_TRIGGER_HAPPY - 0x10;
				break;
		default:
			switch (field->physical) {
			case HID_GD_MOUSE:
			case HID_GD_POINTER:  code += BTN_MOUSE; break;
			case HID_GD_JOYSTICK: code += BTN_JOYSTICK; break;
			case HID_GD_GAMEPAD:  code += BTN_GAMEPAD; break;
			default:              code += BTN_MISC;
			}
		}

		map_key(code);
		break;

	case HID_UP_SIMULATION:
		switch (usage->hid & 0xffff) {
		case 0xba: map_abs(ABS_RUDDER);   break;
		case 0xbb: map_abs(ABS_THROTTLE); break;
		case 0xc4: map_abs(ABS_GAS);      break;
		case 0xc5: map_abs(ABS_BRAKE);    break;
		case 0xc8: map_abs(ABS_WHEEL);    break;
		default:   goto ignore;
		}
		break;

	case HID_UP_GENDESK:
		if ((usage->hid & 0xf0) == 0x80) {	/* SystemControl */
			switch (usage->hid & 0xf) {
			case 0x1: map_key_clear(KEY_POWER);  break;
			case 0x2: map_key_clear(KEY_SLEEP);  break;
			case 0x3: map_key_clear(KEY_WAKEUP); break;
			case 0x4: map_key_clear(KEY_CONTEXT_MENU); break;
			case 0x5: map_key_clear(KEY_MENU); break;
			case 0x6: map_key_clear(KEY_PROG1); break;
			case 0x7: map_key_clear(KEY_HELP); break;
			case 0x8: map_key_clear(KEY_EXIT); break;
			case 0x9: map_key_clear(KEY_SELECT); break;
			case 0xa: map_key_clear(KEY_RIGHT); break;
			case 0xb: map_key_clear(KEY_LEFT); break;
			case 0xc: map_key_clear(KEY_UP); break;
			case 0xd: map_key_clear(KEY_DOWN); break;
			case 0xe: map_key_clear(KEY_POWER2); break;
			case 0xf: map_key_clear(KEY_RESTART); break;
			default: goto unknown;
			}
			break;
		}
		for (i = 0; i < report->maxfield; i++) {
			field = report->field[i];
			for (j = 0; j < field->maxusage; j++)
				if (field->usage[j].type == EV_LED &&
				    field->value[j])
					count += 1;
		}
	}
	return count;
}
EXPORT_SYMBOL_GPL(hidinput_count_leds);

static int hidinput_open(struct input_dev *dev)
{
	struct hid_device *hid = input_get_drvdata(dev);

	return hid_hw_open(hid);
}
	if (!hidinput || !input_dev) {
		kfree(hidinput);
		input_free_device(input_dev);
		hid_err(hid, "Out of memory during hid input probe\n");
		return NULL;
	}

	input_set_drvdata(input_dev, hid);
	input_dev->event = hid->ll_driver->hidinput_input_event;
	input_dev->open = hidinput_open;
	input_dev->close = hidinput_close;
	input_dev->setkeycode = hidinput_setkeycode;
	input_dev->getkeycode = hidinput_getkeycode;

	input_dev->name = hid->name;
	input_dev->phys = hid->phys;
	input_dev->uniq = hid->uniq;
	input_dev->id.bustype = hid->bus;
	input_dev->id.vendor  = hid->vendor;
	input_dev->id.product = hid->product;
	input_dev->id.version = hid->version;
	input_dev->dev.parent = hid->dev.parent;
	hidinput->input = input_dev;
	list_add_tail(&hidinput->list, &hid->inputs);

	return hidinput;
}

static bool hidinput_has_been_populated(struct hid_input *hidinput)
{
	int i;
	unsigned long r = 0;

	for (i = 0; i < BITS_TO_LONGS(EV_CNT); i++)
		r |= hidinput->input->evbit[i];

	for (i = 0; i < BITS_TO_LONGS(KEY_CNT); i++)
		r |= hidinput->input->keybit[i];

	for (i = 0; i < BITS_TO_LONGS(REL_CNT); i++)
		r |= hidinput->input->relbit[i];

	for (i = 0; i < BITS_TO_LONGS(ABS_CNT); i++)
		r |= hidinput->input->absbit[i];

	for (i = 0; i < BITS_TO_LONGS(MSC_CNT); i++)
		r |= hidinput->input->mscbit[i];

	for (i = 0; i < BITS_TO_LONGS(LED_CNT); i++)
		r |= hidinput->input->ledbit[i];

	for (i = 0; i < BITS_TO_LONGS(SND_CNT); i++)
		r |= hidinput->input->sndbit[i];

	for (i = 0; i < BITS_TO_LONGS(FF_CNT); i++)
		r |= hidinput->input->ffbit[i];

	for (i = 0; i < BITS_TO_LONGS(SW_CNT); i++)
		r |= hidinput->input->swbit[i];

	return !!r;
}

static void hidinput_cleanup_hidinput(struct hid_device *hid,
		struct hid_input *hidinput)
{
	struct hid_report *report;
	int i, k;

	list_del(&hidinput->list);
	input_free_device(hidinput->input);

	for (k = HID_INPUT_REPORT; k <= HID_OUTPUT_REPORT; k++) {
		if (k == HID_OUTPUT_REPORT &&
			hid->quirks & HID_QUIRK_SKIP_OUTPUT_REPORTS)
			continue;

		list_for_each_entry(report, &hid->report_enum[k].report_list,
				    list) {

			for (i = 0; i < report->maxfield; i++)
				if (report->field[i]->hidinput == hidinput)
					report->field[i]->hidinput = NULL;
		}
	}
{
	struct hid_driver *drv = hid->driver;
	struct hid_report *report;
	struct hid_input *hidinput = NULL;
	int i, j, k;

	INIT_LIST_HEAD(&hid->inputs);

	if (!force) {
		for (i = 0; i < hid->maxcollection; i++) {
			struct hid_collection *col = &hid->collection[i];
			if (col->type == HID_COLLECTION_APPLICATION ||
					col->type == HID_COLLECTION_PHYSICAL)
				if (IS_INPUT_APPLICATION(col->usage))
					break;
		}

		if (i == hid->maxcollection)
			return -1;
	}
	list_for_each_entry_safe(hidinput, next, &hid->inputs, list) {
		list_del(&hidinput->list);
		input_unregister_device(hidinput->input);
		kfree(hidinput);
	}
}
EXPORT_SYMBOL_GPL(hidinput_disconnect);
