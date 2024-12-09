{
	int head;
	struct usbhid_device *usbhid = hid->driver_data;

	if ((hid->quirks & HID_QUIRK_NOGET) && dir == USB_DIR_IN)
		return;

			return;
		}

		usbhid->out[usbhid->outhead].raw_report = hid_alloc_report_buf(report, GFP_ATOMIC);
		if (!usbhid->out[usbhid->outhead].raw_report) {
			hid_warn(hid, "output queueing failed\n");
			return;
		}
	}

	if (dir == USB_DIR_OUT) {
		usbhid->ctrl[usbhid->ctrlhead].raw_report = hid_alloc_report_buf(report, GFP_ATOMIC);
		if (!usbhid->ctrl[usbhid->ctrlhead].raw_report) {
			hid_warn(hid, "control queueing failed\n");
			return;
		}
	spin_unlock_irqrestore(&usbhid->lock, flags);
}

static int usbhid_wait_io(struct hid_device *hid)
{
	struct usbhid_device *usbhid = hid->driver_data;

{
	struct hid_report *report;
	struct usbhid_device *usbhid = hid->driver_data;
	struct hid_report_enum *report_enum;
	int err, ret;

	if (!(hid->quirks & HID_QUIRK_NO_INIT_INPUT_REPORTS)) {
		report_enum = &hid->report_enum[HID_INPUT_REPORT];
		list_for_each_entry(report, &report_enum->report_list, list)
			usbhid_submit_report(hid, report, USB_DIR_IN);
	}

	report_enum = &hid->report_enum[HID_FEATURE_REPORT];
	list_for_each_entry(report, &report_enum->report_list, list)
		usbhid_submit_report(hid, report, USB_DIR_IN);

	err = 0;
	ret = usbhid_wait_io(hid);
	return -1;
}

static void usbhid_set_leds(struct hid_device *hid)
{
	struct hid_field *field;
	int offset;

		usbhid_submit_report(hid, field->report, USB_DIR_OUT);
	}
}

/*
 * Traverse the supplied list of reports and find the longest
 */
	.open = usbhid_open,
	.close = usbhid_close,
	.power = usbhid_power,
	.request = usbhid_request,
	.wait = usbhid_wait_io,
	.idle = usbhid_idle,
};
	setup_timer(&usbhid->io_retry, hid_retry_timeout, (unsigned long) hid);
	spin_lock_init(&usbhid->lock);

	ret = hid_add_device(hid);
	if (ret) {
		if (ret != -ENODEV)
			hid_err(intf, "can't add hid device: %d\n", ret);
{
	del_timer_sync(&usbhid->io_retry);
	cancel_work_sync(&usbhid->reset_work);
}

static void hid_cease_io(struct usbhid_device *usbhid)
{
	struct usbhid_device *usbhid = hid->driver_data;
	int status = 0;
	bool driver_suspended = false;
	unsigned int ledcount;

	if (PMSG_IS_AUTO(message)) {
		ledcount = hidinput_count_leds(hid);
		spin_lock_irq(&usbhid->lock);	/* Sync with error handler */
		if (!test_bit(HID_RESET_PENDING, &usbhid->iofl)
		    && !test_bit(HID_CLEAR_HALT, &usbhid->iofl)
		    && !test_bit(HID_OUT_RUNNING, &usbhid->iofl)
		    && !test_bit(HID_CTRL_RUNNING, &usbhid->iofl)
		    && !test_bit(HID_KEYS_PRESSED, &usbhid->iofl)
		    && (!ledcount || ignoreled))
		{
			set_bit(HID_SUSPENDED, &usbhid->iofl);
			spin_unlock_irq(&usbhid->lock);
			if (hid->driver && hid->driver->suspend) {