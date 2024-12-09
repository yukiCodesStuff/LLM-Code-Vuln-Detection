	unsigned last_slot_field;	/* the last field of a slot */
	unsigned mt_report_id;	/* the report ID of the multitouch device */
	unsigned pen_report_id;	/* the report ID of the pen device */
	__s8 inputmode;		/* InputMode HID feature, -1 if non-existent */
	__s8 inputmode_index;	/* InputMode HID feature index in the report */
	__s8 maxcontact_report_id;	/* Maximum Contact Number HID feature,
				   -1 if non-existent */
	__u8 num_received;	/* how many contacts we received */
	__u8 num_expected;	/* expected last contact index */
	__u8 maxcontacts;
		struct hid_field *field, struct hid_usage *usage)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	int i;

	switch (usage->hid) {
	case HID_DG_INPUTMODE:
		td->inputmode = field->report->id;
		td->inputmode_index = 0; /* has to be updated below */

		for (i=0; i < field->maxusage; i++) {
			if (field->usage[i].hid == usage->hid) {
				td->inputmode_index = i;
				break;
			}
		}

		break;
	case HID_DG_CONTACTMAX:
		td->maxcontact_report_id = field->report->id;
		td->maxcontacts = field->value[0];
			mt_store_field(usage, td, hi);
			return 1;
		case HID_DG_CONTACTCOUNT:
			td->cc_index = field->index;
			td->cc_value_index = usage->usage_index;
			return 1;
		case HID_DG_CONTACTMAX: