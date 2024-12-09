{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	struct rtl_usb *rtlusb = rtl_usbdev(rtl_usbpriv(hw));

	/* should after adapter start and interrupt enable. */
	set_hal_stop(rtlhal);
	/* Enable software */
	SET_USB_STOP(rtlusb);
	rtl_usb_deinit(hw);
	rtlpriv->cfg->ops->hw_disable(hw);
}

static void _rtl_submit_tx_urb(struct ieee80211_hw *hw, struct urb *_urb)
{
	int err;
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_usb *rtlusb = rtl_usbdev(rtl_usbpriv(hw));

	usb_anchor_urb(_urb, &rtlusb->tx_submitted);
	err = usb_submit_urb(_urb, GFP_ATOMIC);
	if (err < 0) {
		struct sk_buff *skb;

		RT_TRACE(rtlpriv, COMP_USB, DBG_EMERG,
			 "Failed to submit urb\n");
		usb_unanchor_urb(_urb);
		skb = (struct sk_buff *)_urb->context;
		kfree_skb(skb);
	}
{
	return false;
}

static struct rtl_intf_ops rtl_usb_ops = {
	.adapter_start = rtl_usb_start,
	.adapter_stop = rtl_usb_stop,
	.adapter_tx = rtl_usb_tx,
	.waitq_insert = rtl_usb_tx_chk_waitq_insert,
};

int rtl_usb_probe(struct usb_interface *intf,
		  const struct usb_device_id *id,
		  struct rtl_hal_cfg *rtl_hal_cfg)
{
	int err;
	struct ieee80211_hw *hw = NULL;
	struct rtl_priv *rtlpriv = NULL;
	struct usb_device	*udev;
	struct rtl_usb_priv *usb_priv;

	hw = ieee80211_alloc_hw(sizeof(struct rtl_priv) +
				sizeof(struct rtl_usb_priv), &rtl_ops);
	if (!hw) {
		RT_ASSERT(false, "ieee80211 alloc failed\n");
		return -ENOMEM;
	}
	rtlpriv = hw->priv;
	rtlpriv->usb_data = kzalloc(RTL_USB_MAX_RX_COUNT * sizeof(u32),
				    GFP_KERNEL);
	if (!rtlpriv->usb_data)
		return -ENOMEM;

	/* this spin lock must be initialized early */
	spin_lock_init(&rtlpriv->locks.usb_lock);

	rtlpriv->usb_data_index = 0;
	init_completion(&rtlpriv->firmware_loading_complete);
	SET_IEEE80211_DEV(hw, &intf->dev);
	udev = interface_to_usbdev(intf);
	usb_get_dev(udev);
	usb_priv = rtl_usbpriv(hw);
	memset(usb_priv, 0, sizeof(*usb_priv));
	usb_priv->dev.intf = intf;
	usb_priv->dev.udev = udev;
	usb_set_intfdata(intf, hw);
	/* init cfg & intf_ops */
	rtlpriv->rtlhal.interface = INTF_USB;
	rtlpriv->cfg = rtl_hal_cfg;
	rtlpriv->intf_ops = &rtl_usb_ops;
	rtl_dbgp_flag_init(hw);
	/* Init IO handler */
	_rtl_usb_io_handler_init(&udev->dev, hw);
	rtlpriv->cfg->ops->read_chip_version(hw);
	/*like read eeprom and so on */
	rtlpriv->cfg->ops->read_eeprom_info(hw);
	err = _rtl_usb_init(hw);
	if (err)
		goto error_out;
	rtl_usb_init_sw(hw);
	/* Init mac80211 sw */
	err = rtl_init_core(hw);
	if (err) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "Can't allocate sw for mac80211\n");
		goto error_out;
	}
	if (rtlpriv->cfg->ops->init_sw_vars(hw)) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG, "Can't init_sw_vars\n");
		goto error_out;
	}
	rtlpriv->cfg->ops->init_sw_leds(hw);

	return 0;
error_out:
	rtl_deinit_core(hw);
	_rtl_usb_io_handler_release(hw);
	usb_put_dev(udev);
	complete(&rtlpriv->firmware_loading_complete);
	return -ENODEV;
}
	if (!hw) {
		RT_ASSERT(false, "ieee80211 alloc failed\n");
		return -ENOMEM;
	}
	rtlpriv = hw->priv;
	rtlpriv->usb_data = kzalloc(RTL_USB_MAX_RX_COUNT * sizeof(u32),
				    GFP_KERNEL);
	if (!rtlpriv->usb_data)
		return -ENOMEM;

	/* this spin lock must be initialized early */
	spin_lock_init(&rtlpriv->locks.usb_lock);

	rtlpriv->usb_data_index = 0;
	init_completion(&rtlpriv->firmware_loading_complete);
	SET_IEEE80211_DEV(hw, &intf->dev);
	udev = interface_to_usbdev(intf);
	usb_get_dev(udev);
	usb_priv = rtl_usbpriv(hw);
	memset(usb_priv, 0, sizeof(*usb_priv));
	usb_priv->dev.intf = intf;
	usb_priv->dev.udev = udev;
	usb_set_intfdata(intf, hw);
	/* init cfg & intf_ops */
	rtlpriv->rtlhal.interface = INTF_USB;
	rtlpriv->cfg = rtl_hal_cfg;
	rtlpriv->intf_ops = &rtl_usb_ops;
	rtl_dbgp_flag_init(hw);
	/* Init IO handler */
	_rtl_usb_io_handler_init(&udev->dev, hw);
	rtlpriv->cfg->ops->read_chip_version(hw);
	/*like read eeprom and so on */
	rtlpriv->cfg->ops->read_eeprom_info(hw);
	err = _rtl_usb_init(hw);
	if (err)
		goto error_out;
	rtl_usb_init_sw(hw);
	/* Init mac80211 sw */
	err = rtl_init_core(hw);
	if (err) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "Can't allocate sw for mac80211\n");
		goto error_out;
	}