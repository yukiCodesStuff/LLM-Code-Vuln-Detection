{
	u8 version, fae;
	u16 data;
	int err;

	err = mt76x0_load_eeprom(dev);
	if (err < 0)
		return err;

	data = mt76x02_eeprom_get(dev, MT_EE_VERSION);
	version = data >> 8;
	fae = data;

	if (version > MT76X0U_EE_MAX_VER)
		dev_warn(dev->mt76.dev,
			 "Warning: unsupported EEPROM version %02hhx\n",
			 version);
	dev_info(dev->mt76.dev, "EEPROM ver:%02hhx fae:%02hhx\n",
		 version, fae);

	mt76x02_mac_setaddr(dev, dev->mt76.eeprom.data + MT_EE_MAC_ADDR);
	mt76_eeprom_override(&dev->mt76);
	mt76x0_set_chip_cap(dev);
	mt76x0_set_freq_offset(dev);
	mt76x0_set_temp_offset(dev);

	return 0;
}

MODULE_LICENSE("Dual BSD/GPL");