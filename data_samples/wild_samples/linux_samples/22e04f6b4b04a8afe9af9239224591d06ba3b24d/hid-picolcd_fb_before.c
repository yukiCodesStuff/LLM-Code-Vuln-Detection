{
	struct fb_info *info = data->fb_info;
	struct picolcd_fb_data *fbdata = info->par;
	unsigned long flags;

	device_remove_file(&data->hdev->dev, &dev_attr_fb_update_rate);

	/* disconnect framebuffer from HID dev */
	spin_lock_irqsave(&fbdata->lock, flags);
	fbdata->picolcd = NULL;
	spin_unlock_irqrestore(&fbdata->lock, flags);

	/* make sure there is no running update - thus that fbdata->picolcd
	 * once obtained under lock is guaranteed not to get free() under
	 * the feet of the deferred work */
	flush_delayed_work(&info->deferred_work);

	data->fb_info = NULL;
	unregister_framebuffer(info);
}