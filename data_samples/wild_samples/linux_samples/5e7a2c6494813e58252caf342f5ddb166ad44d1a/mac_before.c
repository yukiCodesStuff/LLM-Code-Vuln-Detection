	if (!mt7921_mcu_drv_pmctrl(dev)) {
		int i;

		mt76_for_each_q_rx(&dev->mt76, i)
			napi_schedule(&dev->mt76.napi[i]);
		mt76_connac_pm_dequeue_skbs(mphy, &dev->pm);
		mt7921_tx_cleanup(dev);
		ieee80211_queue_delayed_work(mphy->hw, &mphy->mac_work,
					     MT7921_WATCHDOG_TIME);
	}

	ieee80211_wake_queues(mphy->hw);
	wake_up(&dev->pm.wait);
}

void mt7921_pm_power_save_work(struct work_struct *work)
{
	struct mt7921_dev *dev;
	unsigned long delta;

	dev = (struct mt7921_dev *)container_of(work, struct mt7921_dev,
						pm.ps_work.work);

	delta = dev->pm.idle_timeout;
	if (test_bit(MT76_HW_SCANNING, &dev->mphy.state) ||
	    test_bit(MT76_HW_SCHED_SCANNING, &dev->mphy.state))
		goto out;

	if (time_is_after_jiffies(dev->pm.last_activity + delta)) {
		delta = dev->pm.last_activity + delta - jiffies;
		goto out;
	}

	if (!mt7921_mcu_fw_pmctrl(dev))
		return;
out:
	queue_delayed_work(dev->mt76.wq, &dev->pm.ps_work, delta);
}

int mt7921_mac_set_beacon_filter(struct mt7921_phy *phy,
				 struct ieee80211_vif *vif,
				 bool enable)
{
	struct mt7921_dev *dev = phy->dev;
	bool ext_phy = phy != &dev->phy;
	int err;

	if (!dev->pm.enable)
		return -EOPNOTSUPP;

	err = mt7921_mcu_set_bss_pm(dev, vif, enable);
	if (err)
		return err;

	if (enable) {
		vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER;
		mt76_set(dev, MT_WF_RFCR(ext_phy),
			 MT_WF_RFCR_DROP_OTHER_BEACON);
	} else {
		vif->driver_flags &= ~IEEE80211_VIF_BEACON_FILTER;
		mt76_clear(dev, MT_WF_RFCR(ext_phy),
			   MT_WF_RFCR_DROP_OTHER_BEACON);
	}