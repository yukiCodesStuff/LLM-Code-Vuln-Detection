	struct ath5k_vif *avf = (void *)vif->drv_priv;
	struct ath5k_hw *ah = hw->priv;
	struct ath_common *common = ath5k_hw_common(ah);

	mutex_lock(&ah->lock);

	if (changes & BSS_CHANGED_BSSID) {
	}

	if (changes & BSS_CHANGED_BEACON) {
		spin_lock_bh(&ah->block);
		ath5k_beacon_update(hw, vif);
		spin_unlock_bh(&ah->block);
	}

	if (changes & BSS_CHANGED_BEACON_ENABLED)
		ah->enable_beacon = bss_conf->enable_beacon;