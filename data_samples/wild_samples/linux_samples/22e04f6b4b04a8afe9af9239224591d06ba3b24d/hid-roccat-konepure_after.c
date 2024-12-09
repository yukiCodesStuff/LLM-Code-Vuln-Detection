static const struct hid_device_id konepure_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_ROCCAT, USB_DEVICE_ID_ROCCAT_KONEPURE) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_ROCCAT, USB_DEVICE_ID_ROCCAT_KONEPURE_OPTICAL) },
	{ }
{
	hid_unregister_driver(&konepure_driver);
	class_destroy(konepure_class);
}

module_init(konepure_init);
module_exit(konepure_exit);

MODULE_AUTHOR("Stefan Achatz");
MODULE_DESCRIPTION("USB Roccat KonePure/Optical driver");
MODULE_LICENSE("GPL v2");