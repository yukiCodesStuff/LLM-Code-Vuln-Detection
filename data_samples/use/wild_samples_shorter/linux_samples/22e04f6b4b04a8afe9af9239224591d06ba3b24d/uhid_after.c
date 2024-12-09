	uhid_queue_event(uhid, UHID_CLOSE);
}

static int uhid_hid_parse(struct hid_device *hid)
{
	struct uhid_device *uhid = hid->driver_data;

	.stop = uhid_hid_stop,
	.open = uhid_hid_open,
	.close = uhid_hid_close,
	.parse = uhid_hid_parse,
};

#ifdef CONFIG_COMPAT
MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Herrmann <dh.herrmann@gmail.com>");
MODULE_DESCRIPTION("User-space I/O driver support for HID subsystem");
MODULE_ALIAS("devname:" UHID_NAME);