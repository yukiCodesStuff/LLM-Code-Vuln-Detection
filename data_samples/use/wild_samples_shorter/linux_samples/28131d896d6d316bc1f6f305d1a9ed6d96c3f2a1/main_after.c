	int i;

	for (i = 0; i < ARRAY_SIZE(mvif->bitrate_mask.control); i++) {
		mvif->bitrate_mask.control[i].gi = NL80211_TXRATE_DEFAULT_GI;
		mvif->bitrate_mask.control[i].he_gi = GENMASK(7, 0);
		mvif->bitrate_mask.control[i].he_ltf = GENMASK(7, 0);
		mvif->bitrate_mask.control[i].legacy = GENMASK(31, 0);
		memset(mvif->bitrate_mask.control[i].ht_mcs, GENMASK(7, 0),
		       sizeof(mvif->bitrate_mask.control[i].ht_mcs));
		memset(mvif->bitrate_mask.control[i].vht_mcs, GENMASK(15, 0),
	if (ret)
		return ret;

	return mt7915_mcu_add_rate_ctrl(dev, vif, sta, false);
}

void mt7915_mac_sta_remove(struct mt76_dev *mdev, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta)
{
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt7915_dev *dev = msta->vif->phy->dev;
	u32 *changed = data;

	spin_lock_bh(&dev->sta_poll_lock);
	msta->changed |= *changed;
	if (list_empty(&msta->rc_list))
		list_add_tail(&msta->rc_list, &dev->sta_rc_list);
	spin_unlock_bh(&dev->sta_poll_lock);
}

static void mt7915_sta_rc_update(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_sta *sta,
				 u32 changed)
{
	struct mt7915_phy *phy = mt7915_hw_phy(hw);
	struct mt7915_dev *dev = phy->dev;

	mt7915_sta_rc_work(&changed, sta);
	ieee80211_queue_work(hw, &dev->rc_work);
}

static int
mt7915_set_bitrate_mask(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			const struct cfg80211_bitrate_mask *mask)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	struct mt7915_phy *phy = mt7915_hw_phy(hw);
	struct mt7915_dev *dev = phy->dev;
	u32 changed = IEEE80211_RC_SUPP_RATES_CHANGED;

	mvif->bitrate_mask = *mask;

	/* if multiple rates across different preambles are given we can
	 * reconfigure this info with all peers using sta_rec command with
	 * the below exception cases.
	 * - single rate : if a rate is passed along with different preambles,
	 * we select the highest one as fixed rate. i.e VHT MCS for VHT peers.
	 * - multiple rates: if it's not in range format i.e 0-{7,8,9} for VHT
	 * then multiple MCS setting (MCS 4,5,6) is not supported.
	 */
	ieee80211_iterate_stations_atomic(hw, mt7915_sta_rc_work, &changed);
	ieee80211_queue_work(hw, &dev->rc_work);

	return 0;
}
