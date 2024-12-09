 */
#define SIERRA_NET_USBCTL_BUF_LEN	1024

struct sierra_net_info_data {
	u16 rx_urb_size;
};

/* Private data structure */
struct sierra_net_data {
	return usbnet_change_mtu(net, new_mtu);
}

static int sierra_net_get_fw_attr(struct usbnet *dev, u16 *datap)
{
	int result = 0;
	u16 *attrdata;
	dev_dbg(&dev->udev->dev, "%s", __func__);

	ifacenum = intf->cur_altsetting->desc.bInterfaceNumber;
	numendpoints = intf->cur_altsetting->desc.bNumEndpoints;
	/* We have three endpoints, bulk in and out, and a status */
	if (numendpoints != 3) {
		dev_err(&dev->udev->dev, "Expected 3 endpoints, found: %d",
	return NULL;
}

static const struct sierra_net_info_data sierra_net_info_data_direct_ip = {
	.rx_urb_size = 8 * 1024,
};

static const struct driver_info sierra_net_info_direct_ip = {
	.description = "Sierra Wireless USB-to-WWAN Modem",
	.data = (unsigned long)&sierra_net_info_data_direct_ip,
};

#define DIRECT_IP_DEVICE(vend, prod) \
	{USB_DEVICE_INTERFACE_NUMBER(vend, prod, 7), \
	.driver_info = (unsigned long)&sierra_net_info_direct_ip}, \
	{USB_DEVICE_INTERFACE_NUMBER(vend, prod, 10), \
	.driver_info = (unsigned long)&sierra_net_info_direct_ip}, \
	{USB_DEVICE_INTERFACE_NUMBER(vend, prod, 11), \
	.driver_info = (unsigned long)&sierra_net_info_direct_ip}

static const struct usb_device_id products[] = {
	DIRECT_IP_DEVICE(0x1199, 0x68A3), /* Sierra Wireless USB-to-WWAN modem */
	DIRECT_IP_DEVICE(0x0F3D, 0x68A3), /* AT&T Direct IP modem */
	DIRECT_IP_DEVICE(0x1199, 0x68AA), /* Sierra Wireless Direct IP LTE modem */
	DIRECT_IP_DEVICE(0x0F3D, 0x68AA), /* AT&T Direct IP LTE modem */

	{}, /* last item */
};
MODULE_DEVICE_TABLE(usb, products);