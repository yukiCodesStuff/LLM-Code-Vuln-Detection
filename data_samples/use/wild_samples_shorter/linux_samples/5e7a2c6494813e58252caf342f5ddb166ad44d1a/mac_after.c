			napi_schedule(&dev->mt76.napi[i]);
		mt76_connac_pm_dequeue_skbs(mphy, &dev->pm);
		mt7921_tx_cleanup(dev);
		if (test_bit(MT76_STATE_RUNNING, &mphy->state))
			ieee80211_queue_delayed_work(mphy->hw, &mphy->mac_work,
						     MT7921_WATCHDOG_TIME);
	}

	ieee80211_wake_queues(mphy->hw);
	wake_up(&dev->pm.wait);