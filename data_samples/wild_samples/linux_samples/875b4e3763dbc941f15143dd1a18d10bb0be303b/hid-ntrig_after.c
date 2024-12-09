{
	struct hid_report *report = hdev->report_enum[HID_FEATURE_REPORT].
				    report_id_hash[0x0d];

	if (!report || report->maxfield < 1 ||
	    report->field[0]->report_count < 1)
		return -EINVAL;

	hid_hw_request(hdev, report, HID_REQ_GET_REPORT);
	hid_hw_wait(hdev);
	return (int)report->field[0]->value[0];
}

static inline void ntrig_set_mode(struct hid_device *hdev, const int mode)
{
	struct hid_report *report;
	__u8 mode_commands[4] = { 0xe, 0xf, 0x1b, 0x10 };

	if (mode < 0 || mode > 3)
		return;

	report = hdev->report_enum[HID_FEATURE_REPORT].
		 report_id_hash[mode_commands[mode]];

	if (!report)
		return;

	hid_hw_request(hdev, report, HID_REQ_GET_REPORT);
}

static void ntrig_report_version(struct hid_device *hdev)
{
	int ret;
	char buf[20];
	struct usb_device *usb_dev = hid_to_usb_dev(hdev);
	unsigned char *data = kmalloc(8, GFP_KERNEL);

	if (!data)
		goto err_free;

	ret = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
			      USB_REQ_CLEAR_FEATURE,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE |
			      USB_DIR_IN,
			      0x30c, 1, data, 8,
			      USB_CTRL_SET_TIMEOUT);

	if (ret == 8) {
		ret = ntrig_version_string(&data[2], buf);

		hid_info(hdev, "Firmware version: %s (%02x%02x %02x%02x)\n",
			 buf, data[2], data[3], data[4], data[5]);
	}