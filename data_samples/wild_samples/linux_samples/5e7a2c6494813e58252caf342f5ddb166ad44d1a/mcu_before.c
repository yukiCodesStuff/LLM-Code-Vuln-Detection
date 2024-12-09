	switch (bw) {
	case IEEE80211_STA_RX_BW_160:
		rate->bw = RATE_INFO_BW_160;
		break;
	case IEEE80211_STA_RX_BW_80:
		rate->bw = RATE_INFO_BW_80;
		break;
	case IEEE80211_STA_RX_BW_40:
		rate->bw = RATE_INFO_BW_40;
		break;
	default:
		rate->bw = RATE_INFO_BW_20;
		break;
	}
}

static void
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
		goto out;

	msta = container_of(wcid, struct mt7921_sta, wcid);
	stats = &msta->stats;

	/* current rate */
	mt7921_mcu_tx_rate_parse(mphy, &peer, &rate, curr);
	stats->tx_rate = rate;
out:
	rcu_read_unlock();
}
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
		goto out;

	msta = container_of(wcid, struct mt7921_sta, wcid);
	stats = &msta->stats;

	/* current rate */
	mt7921_mcu_tx_rate_parse(mphy, &peer, &rate, curr);
	stats->tx_rate = rate;
out:
	rcu_read_unlock();
}

static void
mt7921_mcu_scan_event(struct mt7921_dev *dev, struct sk_buff *skb)
{
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct mt7921_phy *phy = (struct mt7921_phy *)mphy->priv;

	spin_lock_bh(&dev->mt76.lock);
	__skb_queue_tail(&phy->scan_event_list, skb);
	spin_unlock_bh(&dev->mt76.lock);

	ieee80211_queue_delayed_work(mphy->hw, &phy->scan_work,
				     MT7921_HW_SCAN_TIMEOUT);
}

static void
mt7921_mcu_beacon_loss_event(struct mt7921_dev *dev, struct sk_buff *skb)
{
	struct mt76_connac_beacon_loss_event *event;
	struct mt76_phy *mphy;
	u8 band_idx = 0; /* DBDC support */

	skb_pull(skb, sizeof(struct mt7921_mcu_rxd));
	event = (struct mt76_connac_beacon_loss_event *)skb->data;
	if (band_idx && dev->mt76.phy2)
		mphy = dev->mt76.phy2;
	else
		mphy = &dev->mt76.phy;

	ieee80211_iterate_active_interfaces_atomic(mphy->hw,
					IEEE80211_IFACE_ITER_RESUME_ALL,
					mt76_connac_mcu_beacon_loss_iter, event);
}

static void
mt7921_mcu_bss_event(struct mt7921_dev *dev, struct sk_buff *skb)
{
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct mt76_connac_mcu_bss_event *event;

	skb_pull(skb, sizeof(struct mt7921_mcu_rxd));
	event = (struct mt76_connac_mcu_bss_event *)skb->data;
	if (event->is_absent)
		ieee80211_stop_queues(mphy->hw);
	else
		ieee80211_wake_queues(mphy->hw);
}

static void
mt7921_mcu_debug_msg_event(struct mt7921_dev *dev, struct sk_buff *skb)
{
	struct mt7921_debug_msg {
		__le16 id;
		u8 type;
		u8 flag;
		__le32 value;
		__le16 len;
		u8 content[512];
	} __packed * msg;

	skb_pull(skb, sizeof(struct mt7921_mcu_rxd));
	msg = (struct mt7921_debug_msg *)skb->data;

	if (msg->type == 3) { /* fw log */
		u16 len = min_t(u16, le16_to_cpu(msg->len), 512);
		int i;

		for (i = 0 ; i < len; i++) {
			if (!msg->content[i])
				msg->content[i] = ' ';
		}
		wiphy_info(mt76_hw(dev)->wiphy, "%.*s", len, msg->content);
	}