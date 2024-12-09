	return ret;
}

static int at76_dfu_get_state(struct usb_device *udev, u8 *state)
{
	int ret;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), DFU_GETSTATE,