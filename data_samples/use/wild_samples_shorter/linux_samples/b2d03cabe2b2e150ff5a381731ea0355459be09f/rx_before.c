
		if (mmie_keyidx < NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS ||
		    mmie_keyidx >= NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS +
		    NUM_DEFAULT_BEACON_KEYS) {
			cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
						     skb->data,
						     skb->len);
			return RX_DROP_MONITOR; /* unexpected BIP keyidx */
		}

		rx->key = ieee80211_rx_get_bigtk(rx, mmie_keyidx);
	/* either the frame has been decrypted or will be dropped */
	status->flag |= RX_FLAG_DECRYPTED;

	if (unlikely(ieee80211_is_beacon(fc) && result == RX_DROP_UNUSABLE))
		cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
					     skb->data, skb->len);

	return result;