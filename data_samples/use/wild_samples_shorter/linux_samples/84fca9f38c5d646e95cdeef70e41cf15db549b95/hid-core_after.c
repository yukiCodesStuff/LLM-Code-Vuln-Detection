static struct hid_field *hid_register_field(struct hid_report *report, unsigned usages, unsigned values)
{
	struct hid_field *field;

	if (report->maxfield == HID_MAX_FIELDS) {
		hid_err(report->device, "too many fields in report\n");
		return NULL;
	field->value = (s32 *)(field->usage + usages);
	field->report = report;

	return field;
}

/*
{
	struct hid_report *report;
	struct hid_field *field;
	unsigned usages;
	unsigned offset;
	unsigned i;

	report = hid_register_report(parser->device, report_type, parser->global.report_id);
	if (!report) {
		hid_err(parser->device, "hid_register_report failed\n");
	if (!parser->local.usage_index) /* Ignore padding fields */
		return 0;

	usages = max_t(unsigned, parser->local.usage_index,
				 parser->global.report_count);

	field = hid_register_field(report, usages, parser->global.report_count);
	if (!field)
		return 0;
	field->application = hid_lookup_collection(parser, HID_COLLECTION_APPLICATION);

	for (i = 0; i < usages; i++) {
		unsigned j = i;
		/* Duplicate the last usage we parsed if we have excess values */
		if (i >= parser->local.usage_index)
			j = parser->local.usage_index - 1;
		field->usage[i].hid = parser->local.usage[j];
		field->usage[i].collection_index =
			parser->local.collection_index[j];
		field->usage[i].usage_index = i;
	}

	field->maxusage = usages;
	field->flags = flags;
}
EXPORT_SYMBOL_GPL(hid_parse_report);

static const char * const hid_report_names[] = {
	"HID_INPUT_REPORT",
	"HID_OUTPUT_REPORT",
	"HID_FEATURE_REPORT",
};
/**
 * hid_validate_values - validate existing device report's value indexes
 *
 * @device: hid device
 * @type: which report type to examine
 * @id: which report ID to examine (0 for first)
 * @field_index: which report field to examine
 * @report_counts: expected number of values
 *
 * Validate the number of values in a given field of a given report, after
 * parsing.
 */
struct hid_report *hid_validate_values(struct hid_device *hid,
				       unsigned int type, unsigned int id,
				       unsigned int field_index,
				       unsigned int report_counts)
{
	struct hid_report *report;

	if (type > HID_FEATURE_REPORT) {
		hid_err(hid, "invalid HID report type %u\n", type);
		return NULL;
	}

	if (id >= HID_MAX_IDS) {
		hid_err(hid, "invalid HID report id %u\n", id);
		return NULL;
	}

	/*
	 * Explicitly not using hid_get_report() here since it depends on
	 * ->numbered being checked, which may not always be the case when
	 * drivers go to access report values.
	 */
	report = hid->report_enum[type].report_id_hash[id];
	if (!report) {
		hid_err(hid, "missing %s %u\n", hid_report_names[type], id);
		return NULL;
	}
	if (report->maxfield <= field_index) {
		hid_err(hid, "not enough fields in %s %u\n",
			hid_report_names[type], id);
		return NULL;
	}
	if (report->field[field_index]->report_count < report_counts) {
		hid_err(hid, "not enough values in %s %u field %u\n",
			hid_report_names[type], id, field_index);
		return NULL;
	}
	return report;
}
EXPORT_SYMBOL_GPL(hid_validate_values);

/**
 * hid_open_report - open a driver-specific device report
 *
 * @device: hid device
			goto out;
	}

	if (hid->claimed != HID_CLAIMED_HIDRAW && report->maxfield) {
		for (a = 0; a < report->maxfield; a++)
			hid_input_field(hid, report->field[a], cdata, interrupt);
		hdrv = hid->driver;
		if (hdrv && hdrv->report)