	.reconfig_complete = mt76x02_reconfig_complete,
};

static int mt76x0e_register_device(struct mt76x02_dev *dev)
{
	int err;

	mt76x0_chip_onoff(dev, true, false);
	if (err < 0)
		return err;

	err = mt76x02_dma_init(dev);
	if (err < 0)
		return err;

	err = mt76x0_init_hardware(dev);
	if (err < 0)
		return err;
	mt76_clear(dev, 0x110, BIT(9));
	mt76_set(dev, MT_MAX_LEN_CFG, BIT(13));

	err = mt76x0_register_device(dev);
	if (err < 0)
		return err;

	if (ret)
		return ret;

	mdev = mt76_alloc_device(&pdev->dev, sizeof(*dev), &mt76x0e_ops,
				 &drv_ops);
	if (!mdev)
		return -ENOMEM;
	mt76_free_device(mdev);
}

static const struct pci_device_id mt76x0e_device_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7610) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7630) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7650) },
	.id_table	= mt76x0e_device_table,
	.probe		= mt76x0e_probe,
	.remove		= mt76x0e_remove,
};

module_pci_driver(mt76x0e_driver);