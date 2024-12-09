#define MT_CLS_NSMU				0x000a
#define MT_CLS_DUAL_CONTACT_NUMBER		0x0010
#define MT_CLS_DUAL_CONTACT_ID			0x0011

/* vendor specific classes */
#define MT_CLS_3M				0x0101
#define MT_CLS_CYPRESS				0x0102
			MT_QUIRK_CONTACT_CNT_ACCURATE |
			MT_QUIRK_SLOT_IS_CONTACTID,
		.maxcontacts = 2 },

	/*
	 * vendor specific classes
	 */
	{ }
};

static void mt_free_input_name(struct hid_input *hi)
{
	struct hid_device *hdev = hi->report->device;
	const char *name = hi->input->name;

	if (name != hdev->name) {
		hi->input->name = hdev->name;
		kfree(name);
	}
}

static ssize_t mt_show_quirks(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
			td->maxcontacts = td->mtclass.maxcontacts;

		break;
	case 0xff0000c5:
		if (field->report_count == 256 && field->report_size == 8) {
			/* Win 8 devices need special quirks */
			__s32 *quirks = &td->mtclass.quirks;
			*quirks |= MT_QUIRK_ALWAYS_VALID;
			*quirks |= MT_QUIRK_IGNORE_DUPLICATES;
			*quirks |= MT_QUIRK_HOVERING;
			*quirks |= MT_QUIRK_CONTACT_CNT_ACCURATE;
			*quirks &= ~MT_QUIRK_NOT_SEEN_MEANS_UP;
			*quirks &= ~MT_QUIRK_VALID_IS_INRANGE;
			*quirks &= ~MT_QUIRK_VALID_IS_CONFIDENCE;
		}
		break;
	}
}

static void set_abs(struct input_dev *input, unsigned int code,
static void mt_pen_input_configured(struct hid_device *hdev,
					struct hid_input *hi)
{
	char *name = kzalloc(strlen(hi->input->name) + 5, GFP_KERNEL);
	if (name) {
		sprintf(name, "%s Pen", hi->input->name);
		mt_free_input_name(hi);
		hi->input->name = name;
	}

	/* force BTN_STYLUS to allow tablet matching in udev */
	__set_bit(BTN_STYLUS, hi->input->keybit);
}

static void mt_input_configured(struct hid_device *hdev, struct hid_input *hi)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	char *name = kstrdup(hdev->name, GFP_KERNEL);

	if (name)
		hi->input->name = name;

	if (hi->report->id == td->mt_report_id)
		mt_touch_input_configured(hdev, hi);

	if (hi->report->id == td->pen_report_id)
		mt_pen_input_configured(hdev, hi);
}

static int mt_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret, i;
	struct mt_device *td;
	struct mt_class *mtclass = mt_classes; /* MT_CLS_DEFAULT */
	struct hid_input *hi;

	for (i = 0; mt_classes[i].name ; i++) {
		if (id->driver_data == mt_classes[i].name) {
			mtclass = &(mt_classes[i]);
	hdev->quirks |= HID_QUIRK_MULTI_INPUT;
	hdev->quirks |= HID_QUIRK_NO_EMPTY_INPUT;

	td = kzalloc(sizeof(struct mt_device), GFP_KERNEL);
	if (!td) {
		dev_err(&hdev->dev, "cannot allocate multitouch data\n");
		return -ENOMEM;
	}
	td->pen_report_id = -1;
	hid_set_drvdata(hdev, td);

	td->fields = kzalloc(sizeof(struct mt_fields), GFP_KERNEL);
	if (!td->fields) {
		dev_err(&hdev->dev, "cannot allocate multitouch fields data\n");
		ret = -ENOMEM;
		goto fail;
	}

	if (id->vendor == HID_ANY_ID && id->product == HID_ANY_ID)
		td->serial_maybe = true;

	ret = hid_parse(hdev);
	if (ret != 0)
		goto fail;

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret)
		goto hid_fail;

	ret = sysfs_create_group(&hdev->dev.kobj, &mt_attribute_group);

	mt_set_maxcontacts(hdev);
	mt_set_input_mode(hdev);

	kfree(td->fields);
	td->fields = NULL;

	return 0;

hid_fail:
	list_for_each_entry(hi, &hdev->inputs, list)
		mt_free_input_name(hi);
fail:
	kfree(td->fields);
	kfree(td);
	return ret;
}

#ifdef CONFIG_PM
static int mt_reset_resume(struct hid_device *hdev)

static void mt_remove(struct hid_device *hdev)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	struct hid_input *hi;

	sysfs_remove_group(&hdev->dev.kobj, &mt_attribute_group);
	list_for_each_entry(hi, &hdev->inputs, list)
		mt_free_input_name(hi);

	hid_hw_stop(hdev);

	kfree(td);
	hid_set_drvdata(hdev, NULL);
}

static const struct hid_device_id mt_devices[] = {


	/* Generic MT device */
	{ HID_DEVICE(HID_BUS_ANY, HID_GROUP_MULTITOUCH, HID_ANY_ID, HID_ANY_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, mt_devices);
