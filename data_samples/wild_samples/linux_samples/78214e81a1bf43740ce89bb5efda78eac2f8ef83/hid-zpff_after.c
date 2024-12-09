{
	struct zpff_device *zpff;
	struct hid_report *report;
	struct hid_input *hidinput = list_entry(hid->inputs.next,
						struct hid_input, list);
	struct input_dev *dev = hidinput->input;
	int i, error;

	for (i = 0; i < 4; i++) {
		report = hid_validate_values(hid, HID_OUTPUT_REPORT, 0, i, 1);
		if (!report)
			return -ENODEV;
	}