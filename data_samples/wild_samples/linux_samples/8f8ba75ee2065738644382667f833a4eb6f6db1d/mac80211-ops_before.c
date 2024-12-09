{
	struct ath5k_vif *avf = (void *)vif->drv_priv;
	struct ath5k_hw *ah = hw->priv;
	struct ath_common *common = ath5k_hw_common(ah);
	unsigned long flags;

	mutex_lock(&ah->lock);

	if (changes & BSS_CHANGED_BSSID) {
		/* Cache for later use during resets */
		memcpy(common->curbssid, bss_conf->bssid, ETH_ALEN);
		common->curaid = 0;
		ath5k_hw_set_bssid(ah);
		mmiowb();
	}
		if (bss_conf->assoc) {
			ATH5K_DBG(ah, ATH5K_DEBUG_ANY,
				  "Bss Info ASSOC %d, bssid: %pM\n",
				  bss_conf->aid, common->curbssid);
			common->curaid = bss_conf->aid;
			ath5k_hw_set_bssid(ah);
			/* Once ANI is available you would start it here */
		}
	}

	if (changes & BSS_CHANGED_BEACON) {
		spin_lock_irqsave(&ah->block, flags);
		ath5k_beacon_update(hw, vif);
		spin_unlock_irqrestore(&ah->block, flags);
	}