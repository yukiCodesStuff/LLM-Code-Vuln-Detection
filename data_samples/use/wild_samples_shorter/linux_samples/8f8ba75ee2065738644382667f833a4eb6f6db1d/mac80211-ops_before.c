	struct ath5k_vif *avf = (void *)vif->drv_priv;
	struct ath5k_hw *ah = hw->priv;
	struct ath_common *common = ath5k_hw_common(ah);
	unsigned long flags;

	mutex_lock(&ah->lock);

	if (changes & BSS_CHANGED_BSSID) {
	}

	if (changes & BSS_CHANGED_BEACON) {
		spin_lock_irqsave(&ah->block, flags);
		ath5k_beacon_update(hw, vif);
		spin_unlock_irqrestore(&ah->block, flags);
	}

	if (changes & BSS_CHANGED_BEACON_ENABLED)
		ah->enable_beacon = bss_conf->enable_beacon;