	uhid_queue_event(uhid, UHID_CLOSE);
}

static int uhid_hid_input(struct input_dev *input, unsigned int type,
			  unsigned int code, int value)
{
	struct hid_device *hid = input_get_drvdata(input);
	struct uhid_device *uhid = hid->driver_data;
	unsigned long flags;
	struct uhid_event *ev;

	ev = kzalloc(sizeof(*ev), GFP_ATOMIC);
	if (!ev)
		return -ENOMEM;

	ev->type = UHID_OUTPUT_EV;
	ev->u.output_ev.type = type;
	ev->u.output_ev.code = code;
	ev->u.output_ev.value = value;

	spin_lock_irqsave(&uhid->qlock, flags);
	uhid_queue(uhid, ev);
	spin_unlock_irqrestore(&uhid->qlock, flags);

	return 0;
}

static int uhid_hid_parse(struct hid_device *hid)
{
	struct uhid_device *uhid = hid->driver_data;

	.stop = uhid_hid_stop,
	.open = uhid_hid_open,
	.close = uhid_hid_close,
	.hidinput_input_event = uhid_hid_input,
	.parse = uhid_hid_parse,
};

#ifdef CONFIG_COMPAT
MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Herrmann <dh.herrmann@gmail.com>");
MODULE_DESCRIPTION("User-space I/O driver support for HID subsystem");