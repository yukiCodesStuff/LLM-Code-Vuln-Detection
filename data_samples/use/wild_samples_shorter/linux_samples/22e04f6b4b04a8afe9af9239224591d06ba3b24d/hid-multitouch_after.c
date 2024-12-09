#define MT_CLS_NSMU				0x000a
#define MT_CLS_DUAL_CONTACT_NUMBER		0x0010
#define MT_CLS_DUAL_CONTACT_ID			0x0011
#define MT_CLS_WIN_8				0x0012

/* vendor specific classes */
#define MT_CLS_3M				0x0101
#define MT_CLS_CYPRESS				0x0102
			MT_QUIRK_CONTACT_CNT_ACCURATE |
			MT_QUIRK_SLOT_IS_CONTACTID,
		.maxcontacts = 2 },
	{ .name = MT_CLS_WIN_8,
		.quirks = MT_QUIRK_ALWAYS_VALID |
			MT_QUIRK_IGNORE_DUPLICATES |
			MT_QUIRK_HOVERING |
			MT_QUIRK_CONTACT_CNT_ACCURATE },

	/*
	 * vendor specific classes
	 */
	{ }
};

static ssize_t mt_show_quirks(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
			td->maxcontacts = td->mtclass.maxcontacts;

		break;
	}
}

static void set_abs(struct input_dev *input, unsigned int code,
static void mt_pen_input_configured(struct hid_device *hdev,
					struct hid_input *hi)
{
	/* force BTN_STYLUS to allow tablet matching in udev */
	__set_bit(BTN_STYLUS, hi->input->keybit);
}

static void mt_input_configured(struct hid_device *hdev, struct hid_input *hi)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	char *name;
	const char *suffix = NULL;

	if (hi->report->id == td->mt_report_id)
		mt_touch_input_configured(hdev, hi);

	if (hi->report->field[0]->physical == HID_DG_STYLUS) {
		suffix = "Pen";
		mt_pen_input_configured(hdev, hi);
	}

	if (suffix) {
		name = devm_kzalloc(&hi->input->dev,
				    strlen(hdev->name) + strlen(suffix) + 2,
				    GFP_KERNEL);
		if (name) {
			sprintf(name, "%s %s", hdev->name, suffix);
			hi->input->name = name;
		}
	}
}

static int mt_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret, i;
	struct mt_device *td;
	struct mt_class *mtclass = mt_classes; /* MT_CLS_DEFAULT */

	for (i = 0; mt_classes[i].name ; i++) {
		if (id->driver_data == mt_classes[i].name) {
			mtclass = &(mt_classes[i]);
	hdev->quirks |= HID_QUIRK_MULTI_INPUT;
	hdev->quirks |= HID_QUIRK_NO_EMPTY_INPUT;

	/*
	 * Handle special quirks for Windows 8 certified devices.
	 */
	if (id->group == HID_GROUP_MULTITOUCH_WIN_8)
		/*
		 * Some multitouch screens do not like to be polled for input
		 * reports. Fortunately, the Win8 spec says that all touches
		 * should be sent during each report, making the initialization
		 * of input reports unnecessary.
		 */
		hdev->quirks |= HID_QUIRK_NO_INIT_INPUT_REPORTS;

	td = devm_kzalloc(&hdev->dev, sizeof(struct mt_device), GFP_KERNEL);
	if (!td) {
		dev_err(&hdev->dev, "cannot allocate multitouch data\n");
		return -ENOMEM;
	}
	td->pen_report_id = -1;
	hid_set_drvdata(hdev, td);

	td->fields = devm_kzalloc(&hdev->dev, sizeof(struct mt_fields),
				  GFP_KERNEL);
	if (!td->fields) {
		dev_err(&hdev->dev, "cannot allocate multitouch fields data\n");
		return -ENOMEM;
	}

	if (id->vendor == HID_ANY_ID && id->product == HID_ANY_ID)
		td->serial_maybe = true;

	ret = hid_parse(hdev);
	if (ret != 0)
		return ret;

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret)
		return ret;

	ret = sysfs_create_group(&hdev->dev.kobj, &mt_attribute_group);

	mt_set_maxcontacts(hdev);
	mt_set_input_mode(hdev);

	/* release .fields memory as it is not used anymore */
	devm_kfree(&hdev->dev, td->fields);
	td->fields = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int mt_reset_resume(struct hid_device *hdev)

static void mt_remove(struct hid_device *hdev)
{
	sysfs_remove_group(&hdev->dev.kobj, &mt_attribute_group);
	hid_hw_stop(hdev);
}

static const struct hid_device_id mt_devices[] = {


	/* Generic MT device */
	{ HID_DEVICE(HID_BUS_ANY, HID_GROUP_MULTITOUCH, HID_ANY_ID, HID_ANY_ID) },

	/* Generic Win 8 certified MT device */
	{  .driver_data = MT_CLS_WIN_8,
		HID_DEVICE(HID_BUS_ANY, HID_GROUP_MULTITOUCH_WIN_8,
			HID_ANY_ID, HID_ANY_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, mt_devices);
