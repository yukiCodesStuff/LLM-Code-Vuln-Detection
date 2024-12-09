mt7921_mcu_tx_rate_report(struct mt7921_dev *dev, struct sk_buff *skb,
			  u16 wlan_idx)
{
	struct mt7921_mcu_wlan_info_event *wtbl_info =
		(struct mt7921_mcu_wlan_info_event *)(skb->data);
	struct rate_info rate = {};
	u8 curr_idx = wtbl_info->rate_info.rate_idx;
	u16 curr = le16_to_cpu(wtbl_info->rate_info.rate[curr_idx]);
	struct mt7921_mcu_peer_cap peer = wtbl_info->peer_cap;
	struct mt76_phy *mphy = &dev->mphy;
	struct mt7921_sta_stats *stats;
	struct mt7921_sta *msta;
	struct mt76_wcid *wcid;

	if (wlan_idx >= MT76_N_WCIDS)
		return;

	rcu_read_lock();

	wcid = rcu_dereference(dev->mt76.wcid[wlan_idx]);
	if (!wcid)
	stats = &msta->stats;

	/* current rate */
	mt7921_mcu_tx_rate_parse(mphy, &peer, &rate, curr);
	stats->tx_rate = rate;
out:
	rcu_read_unlock();
}