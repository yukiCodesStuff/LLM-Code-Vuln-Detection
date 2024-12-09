	static const struct mt76_mcu_ops mt7663u_mcu_ops = {
		.headroom = MT_USB_HDR_SIZE + sizeof(struct mt7615_mcu_txd),
		.tailroom = MT_USB_TAIL_SIZE,
		.mcu_skb_send_msg = mt7663u_mcu_send_message,
		.mcu_parse_response = mt7615_mcu_parse_response,
		.mcu_restart = mt7615_mcu_restart,
	};
	int ret;

	dev->mt76.mcu_ops = &mt7663u_mcu_ops,

	mt76_set(dev, MT_UDMA_TX_QSEL, MT_FW_DL_EN);
	if (test_and_clear_bit(MT76_STATE_POWER_OFF, &dev->mphy.state)) {
		mt7615_mcu_restart(&dev->mt76);
		if (!mt76_poll_msec(dev, MT_CONN_ON_MISC,
				    MT_TOP_MISC2_FW_PWR_ON, 0, 500))
			return -EIO;

		ret = mt76u_vendor_request(&dev->mt76, MT_VEND_POWER_ON,
					   USB_DIR_OUT | USB_TYPE_VENDOR,
					   0x0, 0x1, NULL, 0);
		if (ret)
			return ret;

		if (!mt76_poll_msec(dev, MT_CONN_ON_MISC,
				    MT_TOP_MISC2_FW_PWR_ON,
				    FW_STATE_PWR_ON << 1, 500)) {
			dev_err(dev->mt76.dev, "Timeout for power on\n");
			return -EIO;
		}
	}