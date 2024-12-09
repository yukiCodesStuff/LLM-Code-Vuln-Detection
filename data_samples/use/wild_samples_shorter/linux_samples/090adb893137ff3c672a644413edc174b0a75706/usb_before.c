
	/* should after adapter start and interrupt enable. */
	set_hal_stop(rtlhal);
	/* Enable software */
	SET_USB_STOP(rtlusb);
	rtl_usb_deinit(hw);
	rtlpriv->cfg->ops->hw_disable(hw);
	return false;
}

static struct rtl_intf_ops rtl_usb_ops = {
	.adapter_start = rtl_usb_start,
	.adapter_stop = rtl_usb_stop,
	.adapter_tx = rtl_usb_tx,

	/* this spin lock must be initialized early */
	spin_lock_init(&rtlpriv->locks.usb_lock);

	rtlpriv->usb_data_index = 0;
	init_completion(&rtlpriv->firmware_loading_complete);
	SET_IEEE80211_DEV(hw, &intf->dev);