int lgff_init(struct hid_device* hid)
{
	struct hid_input *hidinput = list_entry(hid->inputs.next, struct hid_input, list);
	struct list_head *report_list = &hid->report_enum[HID_OUTPUT_REPORT].report_list;
	struct input_dev *dev = hidinput->input;
	struct hid_report *report;
	struct hid_field *field;
	const signed short *ff_bits = ff_joystick;
	int error;
	int i;

	/* Find the report to use */
	if (list_empty(report_list)) {
		hid_err(hid, "No output report found\n");
		return -1;
	}

	/* Check that the report looks ok */
	report = list_entry(report_list->next, struct hid_report, list);
	field = report->field[0];
	if (!field) {
		hid_err(hid, "NULL field\n");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(devices); i++) {
		if (dev->id.vendor == devices[i].idVendor &&
		    dev->id.product == devices[i].idProduct) {