			 * next fragment has a sequential PN value.
			 */
			entry->check_sequential_pn = true;
			entry->key_color = rx->key->color;
			memcpy(entry->last_pn,
			       rx->key->u.ccmp.rx_pn[queue],
			       IEEE80211_CCMP_PN_LEN);
			BUILD_BUG_ON(offsetof(struct ieee80211_key,

		if (!requires_sequential_pn(rx, fc))
			return RX_DROP_UNUSABLE;

		/* Prevent mixed key and fragment cache attacks */
		if (entry->key_color != rx->key->color)
			return RX_DROP_UNUSABLE;

		memcpy(pn, entry->last_pn, IEEE80211_CCMP_PN_LEN);
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])