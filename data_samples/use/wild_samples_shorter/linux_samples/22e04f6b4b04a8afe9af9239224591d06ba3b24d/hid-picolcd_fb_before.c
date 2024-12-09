void picolcd_exit_framebuffer(struct picolcd_data *data)
{
	struct fb_info *info = data->fb_info;
	struct picolcd_fb_data *fbdata = info->par;
	unsigned long flags;

	device_remove_file(&data->hdev->dev, &dev_attr_fb_update_rate);

	/* disconnect framebuffer from HID dev */
	spin_lock_irqsave(&fbdata->lock, flags);
	fbdata->picolcd = NULL;