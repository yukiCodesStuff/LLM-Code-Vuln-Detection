	return ret;
}

static int mt7663s_mcu_drv_pmctrl(struct mt7615_dev *dev)
{
	struct sdio_func *func = dev->mt76.sdio.func;
	struct mt76_phy *mphy = &dev->mt76.phy;
	u32 status;
	int ret;

	if (!test_and_clear_bit(MT76_STATE_PM, &mphy->state))
		goto out;

	sdio_claim_host(func);

	sdio_writel(func, WHLPCR_FW_OWN_REQ_CLR, MCR_WHLPCR, NULL);

	}

	sdio_release_host(func);

out:
	dev->pm.last_activity = jiffies;

	return 0;
}

static int mt7663s_mcu_fw_pmctrl(struct mt7615_dev *dev)
{
	struct sdio_func *func = dev->mt76.sdio.func;
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct mt7615_mcu_ops *mcu_ops;
	int ret;

	ret = mt7663s_mcu_drv_pmctrl(dev);
	if (ret)
		return ret;

	dev->mt76.mcu_ops = &mt7663s_mcu_ops,