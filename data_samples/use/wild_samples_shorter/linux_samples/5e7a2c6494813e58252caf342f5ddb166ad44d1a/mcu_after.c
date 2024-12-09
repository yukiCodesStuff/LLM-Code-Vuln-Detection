mt7921_mcu_tx_rate_report(struct mt7921_dev *dev, struct sk_buff *skb,
			  u16 wlan_idx)
{
	struct mt7921_mcu_wlan_info_event *wtbl_info;
	struct mt76_phy *mphy = &dev->mphy;
	struct mt7921_sta_stats *stats;
	struct rate_info rate = {};
	struct mt7921_sta *msta;
	struct mt76_wcid *wcid;
	u8 idx;

	if (wlan_idx >= MT76_N_WCIDS)
		return;

	wtbl_info = (struct mt7921_mcu_wlan_info_event *)skb->data;
	idx = wtbl_info->rate_info.rate_idx;
	if (idx >= ARRAY_SIZE(wtbl_info->rate_info.rate))
		return;

	rcu_read_lock();

	wcid = rcu_dereference(dev->mt76.wcid[wlan_idx]);
	if (!wcid)
	stats = &msta->stats;

	/* current rate */
	mt7921_mcu_tx_rate_parse(mphy, &wtbl_info->peer_cap, &rate,
				 le16_to_cpu(wtbl_info->rate_info.rate[idx]));
	stats->tx_rate = rate;
out:
	rcu_read_unlock();
}