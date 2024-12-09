
	dev->mt76.mcu_ops = &mt7663u_mcu_ops,

	mt76_set(dev, MT_UDMA_TX_QSEL, MT_FW_DL_EN);
	if (test_and_clear_bit(MT76_STATE_POWER_OFF, &dev->mphy.state)) {
		mt7615_mcu_restart(&dev->mt76);
		if (!mt76_poll_msec(dev, MT_CONN_ON_MISC,
				    MT_TOP_MISC2_FW_PWR_ON, 0, 500))