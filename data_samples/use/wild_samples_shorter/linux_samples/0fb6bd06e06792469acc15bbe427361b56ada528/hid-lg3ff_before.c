	int x, y;

/*
 * Maxusage should always be 63 (maximum fields)
 * likely a better way to ensure this data is clean
 */
	memset(report->field[0]->value, 0, sizeof(__s32)*report->field[0]->maxusage);

	switch (effect->type) {
	case FF_CONSTANT:
/*
int lg3ff_init(struct hid_device *hid)
{
	struct hid_input *hidinput = list_entry(hid->inputs.next, struct hid_input, list);
	struct list_head *report_list = &hid->report_enum[HID_OUTPUT_REPORT].report_list;
	struct input_dev *dev = hidinput->input;
	struct hid_report *report;
	struct hid_field *field;
	const signed short *ff_bits = ff3_joystick_ac;
	int error;
	int i;

	/* Find the report to use */
	if (list_empty(report_list)) {
		hid_err(hid, "No output report found\n");
		return -1;
	}

	/* Check that the report looks ok */
	report = list_entry(report_list->next, struct hid_report, list);
	if (!report) {
		hid_err(hid, "NULL output report\n");
		return -1;
	}

	field = report->field[0];
	if (!field) {
		hid_err(hid, "NULL field\n");
		return -1;
	}

	/* Assume single fixed device G940 */
	for (i = 0; ff_bits[i] >= 0; i++)
		set_bit(ff_bits[i], dev->ffbit);