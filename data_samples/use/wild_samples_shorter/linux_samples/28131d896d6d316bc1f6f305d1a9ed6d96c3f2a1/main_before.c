	int i;

	for (i = 0; i < ARRAY_SIZE(mvif->bitrate_mask.control); i++) {
		mvif->bitrate_mask.control[i].legacy = GENMASK(31, 0);
		memset(mvif->bitrate_mask.control[i].ht_mcs, GENMASK(7, 0),
		       sizeof(mvif->bitrate_mask.control[i].ht_mcs));
		memset(mvif->bitrate_mask.control[i].vht_mcs, GENMASK(15, 0),
	if (ret)
		return ret;

	return mt7915_mcu_add_rate_ctrl(dev, vif, sta);
}

void mt7915_mac_sta_remove(struct mt76_dev *mdev, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta)
{
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;
	struct mt7915_dev *dev = msta->vif->phy->dev;
	struct ieee80211_hw *hw = msta->vif->phy->mt76->hw;
	u32 *changed = data;

	spin_lock_bh(&dev->sta_poll_lock);
	msta->changed |= *changed;
	if (list_empty(&msta->rc_list))
		list_add_tail(&msta->rc_list, &dev->sta_rc_list);
	spin_unlock_bh(&dev->sta_poll_lock);

	ieee80211_queue_work(hw, &dev->rc_work);
}

static void mt7915_sta_rc_update(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_sta *sta,
				 u32 changed)
{
	mt7915_sta_rc_work(&changed, sta);
}

static int
mt7915_set_bitrate_mask(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			const struct cfg80211_bitrate_mask *mask)
{
	struct mt7915_vif *mvif = (struct mt7915_vif *)vif->drv_priv;
	enum nl80211_band band = mvif->phy->mt76->chandef.chan->band;
	u32 changed;

	if (mask->control[band].gi == NL80211_TXRATE_FORCE_LGI)
		return -EINVAL;

	changed = IEEE80211_RC_SUPP_RATES_CHANGED;
	mvif->bitrate_mask = *mask;

	/* Update firmware rate control to add a boundary on top of table
	 * to limit the rate selection for each peer, so when set bitrates
	 * vht-mcs-5 1:9, which actually means nss = 1 mcs = 0~9. This only
	 * applies to data frames as for the other mgmt, mcast, bcast still
	 * use legacy rates as it is.
	 */
	ieee80211_iterate_stations_atomic(hw, mt7915_sta_rc_work, &changed);

	return 0;
}
