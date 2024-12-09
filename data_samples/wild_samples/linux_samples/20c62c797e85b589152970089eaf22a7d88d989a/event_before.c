	if (len < sizeof(*status)) {
		pr_err("MAC%u: payload is too short\n", mac->macid);
		return -EINVAL;
	}

	if (timer_pending(&mac->scan_timeout))
		del_timer_sync(&mac->scan_timeout);
	qtnf_scan_done(mac, le32_to_cpu(status->flags) & QLINK_SCAN_ABORTED);

	return 0;
}

static int
qtnf_event_handle_freq_change(struct qtnf_wmac *mac,
			      const struct qlink_event_freq_change *data,
			      u16 len)
{
	struct wiphy *wiphy = priv_to_wiphy(mac);
	struct cfg80211_chan_def chandef;
	struct ieee80211_channel *chan;
	struct qtnf_vif *vif;
	int freq;
	int i;

	if (len < sizeof(*data)) {
		pr_err("payload is too short\n");
		return -EINVAL;
	}

	freq = le32_to_cpu(data->freq);
	chan = ieee80211_get_channel(wiphy, freq);
	if (!chan) {
		pr_err("channel at %d MHz not found\n", freq);
		return -EINVAL;
	}

	pr_debug("MAC%d switch to new channel %u MHz\n", mac->macid, freq);

	if (mac->status & QTNF_MAC_CSA_ACTIVE) {
		mac->status &= ~QTNF_MAC_CSA_ACTIVE;
		if (chan->hw_value != mac->csa_chandef.chan->hw_value)
			pr_warn("unexpected switch to %u during CSA to %u\n",
				chan->hw_value,
				mac->csa_chandef.chan->hw_value);
	}

	/* FIXME: need to figure out proper nl80211_channel_type value */
	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_HT20);
	/* fall-back to minimal safe chandef description */
	if (!cfg80211_chandef_valid(&chandef))
		cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_HT20);

	memcpy(&mac->chandef, &chandef, sizeof(mac->chandef));

	for (i = 0; i < QTNF_MAX_INTF; i++) {
		vif = &mac->iflist[i];
		if (vif->wdev.iftype == NL80211_IFTYPE_UNSPECIFIED)
			continue;

		if (vif->netdev) {
			mutex_lock(&vif->wdev.mtx);
			cfg80211_ch_switch_notify(vif->netdev, &chandef);
			mutex_unlock(&vif->wdev.mtx);
		}