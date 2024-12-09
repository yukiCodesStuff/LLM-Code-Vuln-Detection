 */
static int qmi_wwan_bind_shared(struct usbnet *dev, struct usb_interface *intf)
{
	int rv;
	struct qmi_wwan_state *info = (void *)&dev->data;

	/* ZTE makes devices where the interface descriptors and endpoint
	 * configurations of two or more interfaces are identical, even
	 * though the functions are completely different.  If set, then
	 * driver_info->data is a bitmap of acceptable interface numbers
	 * allowing us to bind to one such interface without binding to
	 * all of them
	 */
	if (dev->driver_info->data &&
	    !test_bit(intf->cur_altsetting->desc.bInterfaceNumber, &dev->driver_info->data)) {
		dev_info(&intf->dev, "not on our whitelist - ignored");
		rv = -ENODEV;
		goto err;
	}

	/*  control and data is shared */
	info->control = intf;
	info->data = intf;
	rv = qmi_wwan_register_subdriver(dev);

err:
	return rv;
}

static void qmi_wwan_unbind(struct usbnet *dev, struct usb_interface *intf)
{
	.manage_power	= qmi_wwan_manage_power,
};

static const struct driver_info	qmi_wwan_force_int0 = {
	.description	= "Qualcomm WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(0), /* interface whitelist bitmap */
};

static const struct driver_info	qmi_wwan_force_int1 = {
	.description	= "Qualcomm WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(1), /* interface whitelist bitmap */
};

static const struct driver_info qmi_wwan_force_int2 = {
	.description	= "Qualcomm WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(2), /* interface whitelist bitmap */
};

static const struct driver_info	qmi_wwan_force_int3 = {
	.description	= "Qualcomm WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(3), /* interface whitelist bitmap */
};

static const struct driver_info	qmi_wwan_force_int4 = {
	.description	= "Qualcomm WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(4), /* interface whitelist bitmap */
};

/* Sierra Wireless provide equally useless interface descriptors
 * Devices in QMI mode can be switched between two different
 * configurations:
 *   a) USB interface #8 is QMI/wwan
 *   b) USB interfaces #8, #19 and #20 are QMI/wwan
 *
 * Both configurations provide a number of other interfaces (serial++),
 * some of which have the same endpoint configuration as we expect, so
 * a whitelist or blacklist is necessary.
 *
 * FIXME: The below whitelist should include BIT(20).  It does not
 * because I cannot get it to work...
 */
static const struct driver_info	qmi_wwan_sierra = {
	.description	= "Sierra Wireless wwan/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind_shared,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
	.data		= BIT(8) | BIT(19), /* interface whitelist bitmap */
};

#define HUAWEI_VENDOR_ID	0x12D1

/* Gobi 1000 QMI/wwan interface number is 3 according to qcserial */
#define QMI_GOBI1K_DEVICE(vend, prod) \
	USB_DEVICE(vend, prod), \
	.driver_info = (unsigned long)&qmi_wwan_force_int3

/* Gobi 2000 and Gobi 3000 QMI/wwan interface number is 0 according to qcserial */
#define QMI_GOBI_DEVICE(vend, prod) \
	USB_DEVICE(vend, prod), \
	.driver_info = (unsigned long)&qmi_wwan_force_int0

static const struct usb_device_id products[] = {
	{	/* Huawei E392, E398 and possibly others sharing both device id and more... */
		.match_flags        = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = HUAWEI_VENDOR_ID,
		.bInterfaceClass    = USB_CLASS_VENDOR_SPEC,
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 9, /* CDC Ethernet *control* interface */
		.driver_info        = (unsigned long)&qmi_wwan_info,
	},
	{	/* Vodafone/Huawei K5005 (12d1:14c8) and similar modems */
		.match_flags        = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = HUAWEI_VENDOR_ID,
		.bInterfaceClass    = USB_CLASS_VENDOR_SPEC,
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 57, /* CDC Ethernet *control* interface */
		.driver_info        = (unsigned long)&qmi_wwan_info,
	},
	{	/* Huawei E392, E398 and possibly others in "Windows mode"
		 * using a combined control and data interface without any CDC
		 * functional descriptors
		 */
		.match_flags        = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = HUAWEI_VENDOR_ID,
		.bInterfaceClass    = USB_CLASS_VENDOR_SPEC,
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 17,
		.driver_info        = (unsigned long)&qmi_wwan_shared,
	},
	{	/* Pantech UML290 */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x106c,
		.idProduct          = 0x3718,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xf0,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_shared,
	},
	{	/* ZTE MF820D */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x0167,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE MF821D */
		.match_flags        = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x0326,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE (Vodafone) K3520-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x0055,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int1,
	},
	{	/* ZTE (Vodafone) K3565-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x0063,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE (Vodafone) K3570-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x1008,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE (Vodafone) K3571-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x1010,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE (Vodafone) K3765-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x2002,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE (Vodafone) K4505-Z */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x0104,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int4,
	},
	{	/* ZTE MF60 */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x19d2,
		.idProduct          = 0x1402,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_force_int2,
	},
	{	/* Sierra Wireless MC77xx in QMI mode */
		.match_flags	    = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,
		.idVendor           = 0x1199,
		.idProduct          = 0x68a2,
		.bInterfaceClass    = 0xff,
		.bInterfaceSubClass = 0xff,
		.bInterfaceProtocol = 0xff,
		.driver_info        = (unsigned long)&qmi_wwan_sierra,
	},

	/* Gobi 1000 devices */
	{QMI_GOBI1K_DEVICE(0x05c6, 0x9212)},	/* Acer Gobi Modem Device */
	{QMI_GOBI1K_DEVICE(0x03f0, 0x1f1d)},	/* HP un2400 Gobi Modem Device */
	{QMI_GOBI1K_DEVICE(0x03f0, 0x371d)},	/* HP un2430 Mobile Broadband Module */
	{QMI_GOBI1K_DEVICE(0x04da, 0x250d)},	/* Panasonic Gobi Modem device */
	{QMI_GOBI1K_DEVICE(0x05c6, 0x9222)},	/* Generic Gobi Modem device */
	{QMI_GOBI1K_DEVICE(0x05c6, 0x9009)},	/* Generic Gobi Modem device */

	/* Gobi 2000 and 3000 devices */
	{QMI_GOBI_DEVICE(0x413c, 0x8186)},	/* Dell Gobi 2000 Modem device (N0218, VU936) */
	{QMI_GOBI_DEVICE(0x05c6, 0x920b)},	/* Generic Gobi 2000 Modem device */
	{QMI_GOBI_DEVICE(0x05c6, 0x9225)},	/* Sony Gobi 2000 Modem device (N0279, VU730) */
	{QMI_GOBI_DEVICE(0x05c6, 0x9245)},	/* Samsung Gobi 2000 Modem device (VL176) */
	{QMI_GOBI_DEVICE(0x05c6, 0x9265)},	/* Asus Gobi 2000 Modem device (VR305) */
	{QMI_GOBI_DEVICE(0x05c6, 0x9235)},	/* Top Global Gobi 2000 Modem device (VR306) */
	{QMI_GOBI_DEVICE(0x05c6, 0x9275)},	/* iRex Technologies Gobi 2000 Modem device (VR307) */
	{QMI_GOBI_DEVICE(0x1199, 0x9001)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x9002)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x9003)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x9004)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x9009)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x900a)},	/* Sierra Wireless Gobi 2000 Modem device (VT773) */
	{QMI_GOBI_DEVICE(0x1199, 0x9011)},	/* Sierra Wireless Gobi 2000 Modem device (MC8305) */
	{QMI_GOBI_DEVICE(0x16d8, 0x8002)},	/* CMDTech Gobi 2000 Modem device (VU922) */
	{QMI_GOBI_DEVICE(0x05c6, 0x9205)},	/* Gobi 2000 Modem device */
	{QMI_GOBI_DEVICE(0x1199, 0x9013)},	/* Sierra Wireless Gobi 3000 Modem device (MC8355) */
	{QMI_GOBI_DEVICE(0x1199, 0x9015)},	/* Sierra Wireless Gobi 3000 Modem device */
	{QMI_GOBI_DEVICE(0x1199, 0x9019)},	/* Sierra Wireless Gobi 3000 Modem device */
	{ }					/* END */
};
MODULE_DEVICE_TABLE(usb, products);
