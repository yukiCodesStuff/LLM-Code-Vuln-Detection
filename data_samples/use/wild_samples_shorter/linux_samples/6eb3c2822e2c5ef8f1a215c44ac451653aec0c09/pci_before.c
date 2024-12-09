	};
	int index = rtlpci->rx_ring[rx_queue_idx].idx;

	/*RX NORMAL PKT */
	while (count--) {
		/*rx descriptor */
		struct rtl_rx_desc *pdesc = &rtlpci->rx_ring[rx_queue_idx].desc[
	 */
	set_hal_stop(rtlhal);

	rtlpriv->cfg->ops->disable_interrupt(hw);
	cancel_work_sync(&rtlpriv->works.lps_change_work);

	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
	ppsc->rfchange_inprogress = true;
	spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flags);

	rtlpci->driver_is_goingto_unload = true;
	rtlpriv->cfg->ops->hw_disable(hw);
	/* some things are not needed if firmware not available */
	if (!rtlpriv->max_fw_size)
		return;