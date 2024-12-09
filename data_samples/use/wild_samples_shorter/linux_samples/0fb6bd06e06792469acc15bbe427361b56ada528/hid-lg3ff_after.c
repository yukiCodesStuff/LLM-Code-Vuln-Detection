	int x, y;

/*
 * Available values in the field should always be 63, but we only use up to
 * 35. Instead, clear the entire area, however big it is.
 */
	memset(report->field[0]->value, 0,
	       sizeof(__s32) * report->field[0]->report_count);

	switch (effect->type) {
	case FF_CONSTANT:
/*
int lg3ff_init(struct hid_device *hid)
{
	struct hid_input *hidinput = list_entry(hid->inputs.next, struct hid_input, list);
	struct input_dev *dev = hidinput->input;
	const signed short *ff_bits = ff3_joystick_ac;
	int error;
	int i;

	/* Check that the report looks ok */
	if (!hid_validate_values(hid, HID_OUTPUT_REPORT, 0, 0, 35))
		return -ENODEV;

	/* Assume single fixed device G940 */
	for (i = 0; ff_bits[i] >= 0; i++)
		set_bit(ff_bits[i], dev->ffbit);