	return NULL;
}

static bool requires_sequential_pn(struct ieee80211_rx_data *rx, __le16 fc)
{
	return rx->key &&
		(rx->key->conf.cipher == WLAN_CIPHER_SUITE_CCMP ||
		 rx->key->conf.cipher == WLAN_CIPHER_SUITE_CCMP_256 ||
		 rx->key->conf.cipher == WLAN_CIPHER_SUITE_GCMP ||
		 rx->key->conf.cipher == WLAN_CIPHER_SUITE_GCMP_256) &&
		ieee80211_has_protected(fc);
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_defragment(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr;
		/* This is the first fragment of a new frame. */
		entry = ieee80211_reassemble_add(rx->sdata, frag, seq,
						 rx->seqno_idx, &(rx->skb));
		if (requires_sequential_pn(rx, fc)) {
			int queue = rx->security_idx;

			/* Store CCMP/GCMP PN so that we can verify that the
			 * next fragment has a sequential PN value.
		u8 pn[IEEE80211_CCMP_PN_LEN], *rpn;
		int queue;

		if (!requires_sequential_pn(rx, fc))
			return RX_DROP_UNUSABLE;
		memcpy(pn, entry->last_pn, IEEE80211_CCMP_PN_LEN);
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;