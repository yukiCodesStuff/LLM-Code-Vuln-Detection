{
	struct input_dev *input = hidinput->input;
	struct hid_device *device = input_get_drvdata(input);
	int max = 0, code;
	unsigned long *bit = NULL;

	field->hidinput = hidinput;

	if (field->flags & HID_MAIN_ITEM_CONSTANT)
		goto ignore;

	/* Ignore if report count is out of bounds. */
	if (field->report_count < 1)
		goto ignore;

	/* only LED usages are supported in output fields */
	if (field->report_type == HID_OUTPUT_REPORT &&
			(usage->hid & HID_USAGE_PAGE) != HID_UP_LED) {
		goto ignore;
	}
{
	struct hid_driver *drv = hid->driver;
	struct hid_report_enum *rep_enum;
	struct hid_report *rep;
	int i, j;

	rep_enum = &hid->report_enum[HID_FEATURE_REPORT];
	list_for_each_entry(rep, &rep_enum->report_list, list)
		for (i = 0; i < rep->maxfield; i++) {
			/* Ignore if report count is out of bounds. */
			if (rep->field[i]->report_count < 1)
				continue;

			for (j = 0; j < rep->field[i]->maxusage; j++) {
				/* Verify if Battery Strength feature is available */
				hidinput_setup_battery(hid, HID_FEATURE_REPORT, rep->field[i]);

				if (drv->feature_mapping)
					drv->feature_mapping(hid, rep->field[i],
							     rep->field[i]->usage + j);
			}
		}
			for (j = 0; j < rep->field[i]->maxusage; j++) {
				/* Verify if Battery Strength feature is available */
				hidinput_setup_battery(hid, HID_FEATURE_REPORT, rep->field[i]);

				if (drv->feature_mapping)
					drv->feature_mapping(hid, rep->field[i],
							     rep->field[i]->usage + j);
			}
		}
}

static struct hid_input *hidinput_allocate(struct hid_device *hid)
{
	struct hid_input *hidinput = kzalloc(sizeof(*hidinput), GFP_KERNEL);
	struct input_dev *input_dev = input_allocate_device();
	if (!hidinput || !input_dev) {
		kfree(hidinput);
		input_free_device(input_dev);
		hid_err(hid, "Out of memory during hid input probe\n");
		return NULL;
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

	kfree(hidinput);
}

/*
 * Register the input device; print a message.
 * Configure the input layer interface
 * Read all reports and initialize the absolute field values.
 */

int hidinput_connect(struct hid_device *hid, unsigned int force)
{
	struct hid_driver *drv = hid->driver;
	struct hid_report *report;
	struct hid_input *hidinput = NULL;
	int i, j, k;

	INIT_LIST_HEAD(&hid->inputs);
	INIT_WORK(&hid->led_work, hidinput_led_worker);

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

	report_features(hid);

	for (k = HID_INPUT_REPORT; k <= HID_OUTPUT_REPORT; k++) {
		if (k == HID_OUTPUT_REPORT &&
			hid->quirks & HID_QUIRK_SKIP_OUTPUT_REPORTS)
			continue;

		list_for_each_entry(report, &hid->report_enum[k].report_list, list) {

			if (!report->maxfield)
				continue;

			if (!hidinput) {
				hidinput = hidinput_allocate(hid);
				if (!hidinput)
					goto out_unwind;
			}