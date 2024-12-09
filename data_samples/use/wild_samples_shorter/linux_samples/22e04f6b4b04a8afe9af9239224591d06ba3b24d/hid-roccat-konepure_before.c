
static const struct hid_device_id konepure_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_ROCCAT, USB_DEVICE_ID_ROCCAT_KONEPURE) },
	{ }
};

MODULE_DEVICE_TABLE(hid, konepure_devices);
module_exit(konepure_exit);

MODULE_AUTHOR("Stefan Achatz");
MODULE_DESCRIPTION("USB Roccat KonePure driver");
MODULE_LICENSE("GPL v2");