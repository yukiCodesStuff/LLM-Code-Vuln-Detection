	if (field->flags & HID_MAIN_ITEM_CONSTANT)
		goto ignore;

	/* Ignore if report count is out of bounds. */
	if (field->report_count < 1)
		goto ignore;

	/* only LED usages are supported in output fields */
	if (field->report_type == HID_OUTPUT_REPORT &&
			(usage->hid & HID_USAGE_PAGE) != HID_UP_LED) {
		goto ignore;

	rep_enum = &hid->report_enum[HID_FEATURE_REPORT];
	list_for_each_entry(rep, &rep_enum->report_list, list)
		for (i = 0; i < rep->maxfield; i++) {
			/* Ignore if report count is out of bounds. */
			if (rep->field[i]->report_count < 1)
				continue;

			for (j = 0; j < rep->field[i]->maxusage; j++) {
				/* Verify if Battery Strength feature is available */
				hidinput_setup_battery(hid, HID_FEATURE_REPORT, rep->field[i]);

					drv->feature_mapping(hid, rep->field[i],
							     rep->field[i]->usage + j);
			}
		}
}

static struct hid_input *hidinput_allocate(struct hid_device *hid)
{