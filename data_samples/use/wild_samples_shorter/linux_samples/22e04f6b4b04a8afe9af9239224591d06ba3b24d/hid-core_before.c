{
	int head;
	struct usbhid_device *usbhid = hid->driver_data;
	int len = ((report->size - 1) >> 3) + 1 + (report->id > 0);

	if ((hid->quirks & HID_QUIRK_NOGET) && dir == USB_DIR_IN)
		return;

			return;
		}

		usbhid->out[usbhid->outhead].raw_report = kmalloc(len, GFP_ATOMIC);
		if (!usbhid->out[usbhid->outhead].raw_report) {
			hid_warn(hid, "output queueing failed\n");
			return;
		}
	}

	if (dir == USB_DIR_OUT) {
		usbhid->ctrl[usbhid->ctrlhead].raw_report = kmalloc(len, GFP_ATOMIC);
		if (!usbhid->ctrl[usbhid->ctrlhead].raw_report) {
			hid_warn(hid, "control queueing failed\n");
			return;
		}
	spin_unlock_irqrestore(&usbhid->lock, flags);
}

/* Workqueue routine to send requests to change LEDs */
static void hid_led(struct work_struct *work)
{
	struct usbhid_device *usbhid =
		container_of(work, struct usbhid_device, led_work);
	struct hid_device *hid = usbhid->hid;
	struct hid_field *field;
	unsigned long flags;

	field = hidinput_get_led_field(hid);
	if (!field) {
		hid_warn(hid, "LED event field not found\n");
		return;
	}

	spin_lock_irqsave(&usbhid->lock, flags);
	if (!test_bit(HID_DISCONNECTED, &usbhid->iofl)) {
		usbhid->ledcount = hidinput_count_leds(hid);
		hid_dbg(usbhid->hid, "New ledcount = %u\n", usbhid->ledcount);
		__usbhid_submit_report(hid, field->report, USB_DIR_OUT);
	}
	spin_unlock_irqrestore(&usbhid->lock, flags);
}

static int usb_hidinput_input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	struct hid_device *hid = input_get_drvdata(dev);
	struct usbhid_device *usbhid = hid->driver_data;
	struct hid_field *field;
	unsigned long flags;
	int offset;

	if (type == EV_FF)
		return input_ff_event(dev, type, code, value);

	if (type != EV_LED)
		return -1;

	if ((offset = hidinput_find_field(hid, type, code, &field)) == -1) {
		hid_warn(dev, "event field not found\n");
		return -1;
	}

	spin_lock_irqsave(&usbhid->lock, flags);
	hid_set_field(field, offset, value);
	spin_unlock_irqrestore(&usbhid->lock, flags);

	/*
	 * Defer performing requested LED action.
	 * This is more likely gather all LED changes into a single URB.
	 */
	schedule_work(&usbhid->led_work);

	return 0;
}

static int usbhid_wait_io(struct hid_device *hid)
{
	struct usbhid_device *usbhid = hid->driver_data;

{
	struct hid_report *report;
	struct usbhid_device *usbhid = hid->driver_data;
	int err, ret;

	list_for_each_entry(report, &hid->report_enum[HID_INPUT_REPORT].report_list, list)
		usbhid_submit_report(hid, report, USB_DIR_IN);

	list_for_each_entry(report, &hid->report_enum[HID_FEATURE_REPORT].report_list, list)
		usbhid_submit_report(hid, report, USB_DIR_IN);

	err = 0;
	ret = usbhid_wait_io(hid);
	return -1;
}

void usbhid_set_leds(struct hid_device *hid)
{
	struct hid_field *field;
	int offset;

		usbhid_submit_report(hid, field->report, USB_DIR_OUT);
	}
}
EXPORT_SYMBOL_GPL(usbhid_set_leds);

/*
 * Traverse the supplied list of reports and find the longest
 */
	.open = usbhid_open,
	.close = usbhid_close,
	.power = usbhid_power,
	.hidinput_input_event = usb_hidinput_input_event,
	.request = usbhid_request,
	.wait = usbhid_wait_io,
	.idle = usbhid_idle,
};
	setup_timer(&usbhid->io_retry, hid_retry_timeout, (unsigned long) hid);
	spin_lock_init(&usbhid->lock);

	INIT_WORK(&usbhid->led_work, hid_led);

	ret = hid_add_device(hid);
	if (ret) {
		if (ret != -ENODEV)
			hid_err(intf, "can't add hid device: %d\n", ret);
{
	del_timer_sync(&usbhid->io_retry);
	cancel_work_sync(&usbhid->reset_work);
	cancel_work_sync(&usbhid->led_work);
}

static void hid_cease_io(struct usbhid_device *usbhid)
{
	struct usbhid_device *usbhid = hid->driver_data;
	int status = 0;
	bool driver_suspended = false;

	if (PMSG_IS_AUTO(message)) {
		spin_lock_irq(&usbhid->lock);	/* Sync with error handler */
		if (!test_bit(HID_RESET_PENDING, &usbhid->iofl)
		    && !test_bit(HID_CLEAR_HALT, &usbhid->iofl)
		    && !test_bit(HID_OUT_RUNNING, &usbhid->iofl)
		    && !test_bit(HID_CTRL_RUNNING, &usbhid->iofl)
		    && !test_bit(HID_KEYS_PRESSED, &usbhid->iofl)
		    && (!usbhid->ledcount || ignoreled))
		{
			set_bit(HID_SUSPENDED, &usbhid->iofl);
			spin_unlock_irq(&usbhid->lock);
			if (hid->driver && hid->driver->suspend) {