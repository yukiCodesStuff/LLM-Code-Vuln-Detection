{
	int ret;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), DFU_GETSTATUS,
			      USB_TYPE_CLASS | USB_DIR_IN | USB_RECIP_INTERFACE,
			      0, 0, status, sizeof(struct dfu_status),
			      USB_CTRL_GET_TIMEOUT);
	return ret;
}

static u8 at76_dfu_get_state(struct usb_device *udev, u8 *state)
{
	int ret;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), DFU_GETSTATE,
			      USB_TYPE_CLASS | USB_DIR_IN | USB_RECIP_INTERFACE,
			      0, 0, state, 1, USB_CTRL_GET_TIMEOUT);
	return ret;
}

/* Convert timeout from the DFU status to jiffies */
static inline unsigned long at76_get_timeout(struct dfu_status *s)
{
	return msecs_to_jiffies((s->poll_timeout[2] << 16)
				| (s->poll_timeout[1] << 8)
				| (s->poll_timeout[0]));
}

/* Load internal firmware from the buffer.  If manifest_sync_timeout > 0, use
 * its value in jiffies in the MANIFEST_SYNC state.  */
static int at76_usbdfu_download(struct usb_device *udev, u8 *buf, u32 size,
				int manifest_sync_timeout)
{
	u8 *block;
	struct dfu_status dfu_stat_buf;
	int ret = 0;
	int need_dfu_state = 1;
	int is_done = 0;
	u8 dfu_state = 0;
	u32 dfu_timeout = 0;
	int bsize = 0;
	int blockno = 0;

	at76_dbg(DBG_DFU, "%s( %p, %u, %d)", __func__, buf, size,
		 manifest_sync_timeout);

	if (!size) {
		dev_printk(KERN_ERR, &udev->dev, "FW buffer length invalid!\n");
		return -EINVAL;
	}