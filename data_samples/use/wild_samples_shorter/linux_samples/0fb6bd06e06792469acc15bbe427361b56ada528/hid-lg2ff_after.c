	struct hid_report *report;
	struct hid_input *hidinput = list_entry(hid->inputs.next,
						struct hid_input, list);
	struct input_dev *dev = hidinput->input;
	int error;

	/* Check that the report looks ok */
	report = hid_validate_values(hid, HID_OUTPUT_REPORT, 0, 0, 7);
	if (!report)
		return -ENODEV;

	lg2ff = kmalloc(sizeof(struct lg2ff_device), GFP_KERNEL);
	if (!lg2ff)
		return -ENOMEM;