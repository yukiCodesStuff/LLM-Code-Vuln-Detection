	0xC0                          /*  End Collection                  */
};

static __u8 *kye_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	switch (hdev->product) {
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
				USB_DEVICE_ID_KYE_EASYPEN_M610X) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_KYE,
				USB_DEVICE_ID_GENIUS_GILA_GAMING_MOUSE) },
	{ }
};
MODULE_DEVICE_TABLE(hid, kye_devices);
