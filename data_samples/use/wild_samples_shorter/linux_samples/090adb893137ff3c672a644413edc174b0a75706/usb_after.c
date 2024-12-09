
	/* should after adapter start and interrupt enable. */
	set_hal_stop(rtlhal);
	cancel_work_sync(&rtlpriv->works.fill_h2c_cmd);
	/* Enable software */
	SET_USB_STOP(rtlusb);
	rtl_usb_deinit(hw);
	rtlpriv->cfg->ops->hw_disable(hw);
	return false;
}

static void rtl_fill_h2c_cmd_work_callback(struct work_struct *work)
{
	struct rtl_works *rtlworks =
	    container_of(work, struct rtl_works, fill_h2c_cmd);
	struct ieee80211_hw *hw = rtlworks->hw;
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	rtlpriv->cfg->ops->fill_h2c_cmd(hw, H2C_RA_MASK, 5, rtlpriv->rate_mask);
}

static struct rtl_intf_ops rtl_usb_ops = {
	.adapter_start = rtl_usb_start,
	.adapter_stop = rtl_usb_stop,
	.adapter_tx = rtl_usb_tx,

	/* this spin lock must be initialized early */
	spin_lock_init(&rtlpriv->locks.usb_lock);
	INIT_WORK(&rtlpriv->works.fill_h2c_cmd,
		  rtl_fill_h2c_cmd_work_callback);

	rtlpriv->usb_data_index = 0;
	init_completion(&rtlpriv->firmware_loading_complete);
	SET_IEEE80211_DEV(hw, &intf->dev);