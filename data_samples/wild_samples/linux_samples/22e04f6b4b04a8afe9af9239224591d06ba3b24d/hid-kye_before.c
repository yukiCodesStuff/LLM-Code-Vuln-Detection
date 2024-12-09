static __u8 easypen_m610x_rdesc_fixed[] = {
	0x06, 0x00, 0xFF,             /*  Usage Page (FF00h),             */
	0x09, 0x01,                   /*  Usage (01h),                    */
	0xA1, 0x01,                   /*  Collection (Application),       */
	0x85, 0x05,                   /*    Report ID (5),                */
	0x09, 0x01,                   /*    Usage (01h),                  */
	0x15, 0x80,                   /*    Logical Minimum (-128),       */
	0x25, 0x7F,                   /*    Logical Maximum (127),        */
	0x75, 0x08,                   /*    Report Size (8),              */
	0x95, 0x07,                   /*    Report Count (7),             */
	0xB1, 0x02,                   /*    Feature (Variable),           */
	0xC0,                         /*  End Collection,                 */
	0x05, 0x0D,                   /*  Usage Page (Digitizer),         */
	0x09, 0x02,                   /*  Usage (Pen),                    */
	0xA1, 0x01,                   /*  Collection (Application),       */
	0x85, 0x10,                   /*    Report ID (16),               */
	0x09, 0x20,                   /*    Usage (Stylus),               */
	0xA0,                         /*    Collection (Physical),        */
	0x14,                         /*      Logical Minimum (0),        */
	0x25, 0x01,                   /*      Logical Maximum (1),        */
	0x75, 0x01,                   /*      Report Size (1),            */
	0x09, 0x42,                   /*      Usage (Tip Switch),         */
	0x09, 0x44,                   /*      Usage (Barrel Switch),      */
	0x09, 0x46,                   /*      Usage (Tablet Pick),        */
	0x95, 0x03,                   /*      Report Count (3),           */
	0x81, 0x02,                   /*      Input (Variable),           */
	0x95, 0x04,                   /*      Report Count (4),           */
	0x81, 0x03,                   /*      Input (Constant, Variable), */
	0x09, 0x32,                   /*      Usage (In Range),           */
	0x95, 0x01,                   /*      Report Count (1),           */
	0x81, 0x02,                   /*      Input (Variable),           */
	0x75, 0x10,                   /*      Report Size (16),           */
	0x95, 0x01,                   /*      Report Count (1),           */
	0xA4,                         /*      Push,                       */
	0x05, 0x01,                   /*      Usage Page (Desktop),       */
	0x55, 0xFD,                   /*      Unit Exponent (-3),         */
	0x65, 0x13,                   /*      Unit (Inch),                */
	0x34,                         /*      Physical Minimum (0),       */
	0x09, 0x30,                   /*      Usage (X),                  */
	0x46, 0x10, 0x27,             /*      Physical Maximum (10000),   */
	0x27, 0x00, 0xA0, 0x00, 0x00, /*      Logical Maximum (40960),    */
	0x81, 0x02,                   /*      Input (Variable),           */
	0x09, 0x31,                   /*      Usage (Y),                  */
	0x46, 0x6A, 0x18,             /*      Physical Maximum (6250),    */
	0x26, 0x00, 0x64,             /*      Logical Maximum (25600),    */
	0x81, 0x02,                   /*      Input (Variable),           */
	0xB4,                         /*      Pop,                        */
	0x09, 0x30,                   /*      Usage (Tip Pressure),       */
	0x26, 0xFF, 0x03,             /*      Logical Maximum (1023),     */
	0x81, 0x02,                   /*      Input (Variable),           */
	0xC0,                         /*    End Collection,               */
	0xC0,                         /*  End Collection,                 */
	0x05, 0x0C,                   /*  Usage Page (Consumer),          */
	0x09, 0x01,                   /*  Usage (Consumer Control),       */
	0xA1, 0x01,                   /*  Collection (Application),       */
	0x85, 0x12,                   /*    Report ID (18),               */
	0x14,                         /*    Logical Minimum (0),          */
	0x25, 0x01,                   /*    Logical Maximum (1),          */
	0x75, 0x01,                   /*    Report Size (1),              */
	0x95, 0x04,                   /*    Report Count (4),             */
	0x0A, 0x1A, 0x02,             /*    Usage (AC Undo),              */
	0x0A, 0x79, 0x02,             /*    Usage (AC Redo Or Repeat),    */
	0x0A, 0x2D, 0x02,             /*    Usage (AC Zoom In),           */
	0x0A, 0x2E, 0x02,             /*    Usage (AC Zoom Out),          */
	0x81, 0x02,                   /*    Input (Variable),             */
	0x95, 0x01,                   /*    Report Count (1),             */
	0x75, 0x14,                   /*    Report Size (20),             */
	0x81, 0x03,                   /*    Input (Constant, Variable),   */
	0x75, 0x20,                   /*    Report Size (32),             */
	0x81, 0x03,                   /*    Input (Constant, Variable),   */
	0xC0                          /*  End Collection                  */
};

static __u8 *kye_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	switch (hdev->product) {
	case USB_DEVICE_ID_KYE_ERGO_525V:
		/* the fixups that need to be done:
		 *   - change led usage page to button for extra buttons
		 *   - report size 8 count 1 must be size 1 count 8 for button
		 *     bitfield
		 *   - change the button usage range to 4-7 for the extra
		 *     buttons
		 */
		if (*rsize >= 74 &&
			rdesc[61] == 0x05 && rdesc[62] == 0x08 &&
			rdesc[63] == 0x19 && rdesc[64] == 0x08 &&
			rdesc[65] == 0x29 && rdesc[66] == 0x0f &&
			rdesc[71] == 0x75 && rdesc[72] == 0x08 &&
			rdesc[73] == 0x95 && rdesc[74] == 0x01) {
			hid_info(hdev,
				 "fixing up Kye/Genius Ergo Mouse "
				 "report descriptor\n");
			rdesc[62] = 0x09;
			rdesc[64] = 0x04;
			rdesc[66] = 0x07;
			rdesc[72] = 0x01;
			rdesc[74] = 0x08;
		}
		break;
	case USB_DEVICE_ID_KYE_EASYPEN_I405X:
		if (*rsize == EASYPEN_I405X_RDESC_ORIG_SIZE) {
			rdesc = easypen_i405x_rdesc_fixed;
			*rsize = sizeof(easypen_i405x_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_KYE_MOUSEPEN_I608X:
		if (*rsize == MOUSEPEN_I608X_RDESC_ORIG_SIZE) {
			rdesc = mousepen_i608x_rdesc_fixed;
			*rsize = sizeof(mousepen_i608x_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_KYE_EASYPEN_M610X:
		if (*rsize == EASYPEN_M610X_RDESC_ORIG_SIZE) {
			rdesc = easypen_m610x_rdesc_fixed;
			*rsize = sizeof(easypen_m610x_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_GENIUS_GILA_GAMING_MOUSE:
		/*
		 * the fixup that need to be done:
		 *   - change Usage Maximum in the Comsumer Control
		 *     (report ID 3) to a reasonable value
		 */
		if (*rsize >= 135 &&
			/* Usage Page (Consumer Devices) */
			rdesc[104] == 0x05 && rdesc[105] == 0x0c &&
			/* Usage (Consumer Control) */
			rdesc[106] == 0x09 && rdesc[107] == 0x01 &&
			/*   Usage Maximum > 12287 */
			rdesc[114] == 0x2a && rdesc[116] > 0x2f) {
			hid_info(hdev,
				 "fixing up Genius Gila Gaming Mouse "
				 "report descriptor\n");
			rdesc[116] = 0x2f;
		}
		break;
	}
	return rdesc;
}
		if (*rsize == EASYPEN_M610X_RDESC_ORIG_SIZE) {
			rdesc = easypen_m610x_rdesc_fixed;
			*rsize = sizeof(easypen_m610x_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_GENIUS_GILA_GAMING_MOUSE:
		/*
		 * the fixup that need to be done:
		 *   - change Usage Maximum in the Comsumer Control
		 *     (report ID 3) to a reasonable value
		 */
		if (*rsize >= 135 &&
			/* Usage Page (Consumer Devices) */
			rdesc[104] == 0x05 && rdesc[105] == 0x0c &&
			/* Usage (Consumer Control) */
			rdesc[106] == 0x09 && rdesc[107] == 0x01 &&
			/*   Usage Maximum > 12287 */
			rdesc[114] == 0x2a && rdesc[116] > 0x2f) {
			hid_info(hdev,
				 "fixing up Genius Gila Gaming Mouse "
				 "report descriptor\n");
			rdesc[116] = 0x2f;
		}
static const struct hid_device_id kye_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE, USB_DEVICE_ID_KYE_ERGO_525V) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE,
				USB_DEVICE_ID_KYE_EASYPEN_I405X) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE,
				USB_DEVICE_ID_KYE_MOUSEPEN_I608X) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE,
				USB_DEVICE_ID_KYE_EASYPEN_M610X) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE,
				USB_DEVICE_ID_GENIUS_GILA_GAMING_MOUSE) },
	{ }
};
MODULE_DEVICE_TABLE(hid, kye_devices);

static struct hid_driver kye_driver = {
	.name = "kye",
	.id_table = kye_devices,
	.probe = kye_probe,
	.report_fixup = kye_report_fixup,
};
module_hid_driver(kye_driver);

MODULE_LICENSE("GPL");