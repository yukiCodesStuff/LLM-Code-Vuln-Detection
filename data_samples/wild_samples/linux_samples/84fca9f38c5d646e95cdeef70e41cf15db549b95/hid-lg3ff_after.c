{
	struct hid_device *hid = input_get_drvdata(dev);
	struct list_head *report_list = &hid->report_enum[HID_OUTPUT_REPORT].report_list;
	struct hid_report *report = list_entry(report_list->next, struct hid_report, list);
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
 * Already clamped in ff_memless
 * 0 is center (different then other logitech)
 */
		x = effect->u.ramp.start_level;
		y = effect->u.ramp.end_level;

		/* send command byte */
		report->field[0]->value[0] = 0x51;

/*
 * Sign backwards from other Force3d pro
 * which get recast here in two's complement 8 bits
 */
		report->field[0]->value[1] = (unsigned char)(-x);
		report->field[0]->value[31] = (unsigned char)(-y);

		hid_hw_request(hid, report, HID_REQ_SET_REPORT);
		break;
	}
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

	error = input_ff_create_memless(dev, NULL, hid_lg3ff_play);
	if (error)
		return error;

	if (test_bit(FF_AUTOCENTER, dev->ffbit))
		dev->ff->set_autocenter = hid_lg3ff_set_autocenter;

	hid_info(hid, "Force feedback for Logitech Flight System G940 by Gary Stein <LordCnidarian@gmail.com>\n");
	return 0;
}