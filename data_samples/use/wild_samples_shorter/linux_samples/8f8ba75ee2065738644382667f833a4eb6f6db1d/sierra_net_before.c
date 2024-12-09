 */
#define SIERRA_NET_USBCTL_BUF_LEN	1024

/* list of interface numbers - used for constructing interface lists */
struct sierra_net_iface_info {
	const u32 infolen;	/* number of interface numbers on list */
	const u8  *ifaceinfo;	/* pointer to the array holding the numbers */
};

struct sierra_net_info_data {
	u16 rx_urb_size;
	struct sierra_net_iface_info whitelist;
};

/* Private data structure */
struct sierra_net_data {
	return usbnet_change_mtu(net, new_mtu);
}

static int is_whitelisted(const u8 ifnum,
			const struct sierra_net_iface_info *whitelist)
{
	if (whitelist) {
		const u8 *list = whitelist->ifaceinfo;
		int i;

		for (i = 0; i < whitelist->infolen; i++) {
			if (list[i] == ifnum)
				return 1;
		}
	}
	return 0;
}

static int sierra_net_get_fw_attr(struct usbnet *dev, u16 *datap)
{
	int result = 0;
	u16 *attrdata;
	dev_dbg(&dev->udev->dev, "%s", __func__);

	ifacenum = intf->cur_altsetting->desc.bInterfaceNumber;
	/* We only accept certain interfaces */
	if (!is_whitelisted(ifacenum, &data->whitelist)) {
		dev_dbg(&dev->udev->dev, "Ignoring interface: %d", ifacenum);
		return -ENODEV;
	}
	numendpoints = intf->cur_altsetting->desc.bNumEndpoints;
	/* We have three endpoints, bulk in and out, and a status */
	if (numendpoints != 3) {
		dev_err(&dev->udev->dev, "Expected 3 endpoints, found: %d",
	return NULL;
}

static const u8 sierra_net_ifnum_list[] = { 7, 10, 11 };
static const struct sierra_net_info_data sierra_net_info_data_direct_ip = {
	.rx_urb_size = 8 * 1024,
	.whitelist = {
		.infolen = ARRAY_SIZE(sierra_net_ifnum_list),
		.ifaceinfo = sierra_net_ifnum_list
	}
};

static const struct driver_info sierra_net_info_direct_ip = {
	.description = "Sierra Wireless USB-to-WWAN Modem",
	.data = (unsigned long)&sierra_net_info_data_direct_ip,
};

static const struct usb_device_id products[] = {
	{USB_DEVICE(0x1199, 0x68A3), /* Sierra Wireless USB-to-WWAN modem */
	.driver_info = (unsigned long) &sierra_net_info_direct_ip},
	{USB_DEVICE(0x0F3D, 0x68A3), /* AT&T Direct IP modem */
	.driver_info = (unsigned long) &sierra_net_info_direct_ip},
	{USB_DEVICE(0x1199, 0x68AA), /* Sierra Wireless Direct IP LTE modem */
	.driver_info = (unsigned long) &sierra_net_info_direct_ip},
	{USB_DEVICE(0x0F3D, 0x68AA), /* AT&T Direct IP LTE modem */
	.driver_info = (unsigned long) &sierra_net_info_direct_ip},

	{}, /* last item */
};
MODULE_DEVICE_TABLE(usb, products);