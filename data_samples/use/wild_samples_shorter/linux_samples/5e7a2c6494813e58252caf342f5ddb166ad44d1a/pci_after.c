	.reconfig_complete = mt76x02_reconfig_complete,
};

static int mt76x0e_init_hardware(struct mt76x02_dev *dev, bool resume)
{
	int err;

	mt76x0_chip_onoff(dev, true, false);
	if (err < 0)
		return err;

	if (!resume) {
		err = mt76x02_dma_init(dev);
		if (err < 0)
			return err;
	}

	err = mt76x0_init_hardware(dev);
	if (err < 0)
		return err;
	mt76_clear(dev, 0x110, BIT(9));
	mt76_set(dev, MT_MAX_LEN_CFG, BIT(13));

	return 0;
}

static int mt76x0e_register_device(struct mt76x02_dev *dev)
{
	int err;

	err = mt76x0e_init_hardware(dev, false);
	if (err < 0)
		return err;

	err = mt76x0_register_device(dev);
	if (err < 0)
		return err;

	if (ret)
		return ret;

	mt76_pci_disable_aspm(pdev);

	mdev = mt76_alloc_device(&pdev->dev, sizeof(*dev), &mt76x0e_ops,
				 &drv_ops);
	if (!mdev)
		return -ENOMEM;
	mt76_free_device(mdev);
}

#ifdef CONFIG_PM
static int mt76x0e_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct mt76_dev *mdev = pci_get_drvdata(pdev);
	struct mt76x02_dev *dev = container_of(mdev, struct mt76x02_dev, mt76);
	int i;

	mt76_worker_disable(&mdev->tx_worker);
	for (i = 0; i < ARRAY_SIZE(mdev->phy.q_tx); i++)
		mt76_queue_tx_cleanup(dev, mdev->phy.q_tx[i], true);
	for (i = 0; i < ARRAY_SIZE(mdev->q_mcu); i++)
		mt76_queue_tx_cleanup(dev, mdev->q_mcu[i], true);
	napi_disable(&mdev->tx_napi);

	mt76_for_each_q_rx(mdev, i)
		napi_disable(&mdev->napi[i]);

	mt76x02_dma_disable(dev);
	mt76x02_mcu_cleanup(dev);
	mt76x0_chip_onoff(dev, false, false);

	pci_enable_wake(pdev, pci_choose_state(pdev, state), true);
	pci_save_state(pdev);

	return pci_set_power_state(pdev, pci_choose_state(pdev, state));
}

static int mt76x0e_resume(struct pci_dev *pdev)
{
	struct mt76_dev *mdev = pci_get_drvdata(pdev);
	struct mt76x02_dev *dev = container_of(mdev, struct mt76x02_dev, mt76);
	int err, i;

	err = pci_set_power_state(pdev, PCI_D0);
	if (err)
		return err;

	pci_restore_state(pdev);

	mt76_worker_enable(&mdev->tx_worker);

	mt76_for_each_q_rx(mdev, i) {
		mt76_queue_rx_reset(dev, i);
		napi_enable(&mdev->napi[i]);
		napi_schedule(&mdev->napi[i]);
	}

	napi_enable(&mdev->tx_napi);
	napi_schedule(&mdev->tx_napi);

	return mt76x0e_init_hardware(dev, true);
}
#endif /* CONFIG_PM */

static const struct pci_device_id mt76x0e_device_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7610) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7630) },
	{ PCI_DEVICE(PCI_VENDOR_ID_MEDIATEK, 0x7650) },
	.id_table	= mt76x0e_device_table,
	.probe		= mt76x0e_probe,
	.remove		= mt76x0e_remove,
#ifdef CONFIG_PM
	.suspend	= mt76x0e_suspend,
	.resume		= mt76x0e_resume,
#endif /* CONFIG_PM */
};

module_pci_driver(mt76x0e_driver);