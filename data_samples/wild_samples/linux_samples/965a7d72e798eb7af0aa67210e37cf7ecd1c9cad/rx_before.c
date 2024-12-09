		if (time_after(jiffies, entry->first_frag_time + 2 * HZ)) {
			__skb_queue_purge(&entry->skb_list);
			continue;
		}
		return entry;
	}

	return NULL;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_defragment(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr;
	u16 sc;
	__le16 fc;
	unsigned int frag, seq;
	struct ieee80211_fragment_entry *entry;
	struct sk_buff *skb;

	hdr = (struct ieee80211_hdr *)rx->skb->data;
	fc = hdr->frame_control;

	if (ieee80211_is_ctl(fc) || ieee80211_is_ext(fc))
		return RX_CONTINUE;

	sc = le16_to_cpu(hdr->seq_ctrl);
	frag = sc & IEEE80211_SCTL_FRAG;

	if (is_multicast_ether_addr(hdr->addr1)) {
		I802_DEBUG_INC(rx->local->dot11MulticastReceivedFrameCount);
		goto out_no_led;
	}
	if (frag == 0) {
		/* This is the first fragment of a new frame. */
		entry = ieee80211_reassemble_add(rx->sdata, frag, seq,
						 rx->seqno_idx, &(rx->skb));
		if (rx->key &&
		    (rx->key->conf.cipher == WLAN_CIPHER_SUITE_CCMP ||
		     rx->key->conf.cipher == WLAN_CIPHER_SUITE_CCMP_256 ||
		     rx->key->conf.cipher == WLAN_CIPHER_SUITE_GCMP ||
		     rx->key->conf.cipher == WLAN_CIPHER_SUITE_GCMP_256) &&
		    ieee80211_has_protected(fc)) {
			int queue = rx->security_idx;

			/* Store CCMP/GCMP PN so that we can verify that the
			 * next fragment has a sequential PN value.
			 */
			entry->check_sequential_pn = true;
			memcpy(entry->last_pn,
			       rx->key->u.ccmp.rx_pn[queue],
			       IEEE80211_CCMP_PN_LEN);
			BUILD_BUG_ON(offsetof(struct ieee80211_key,
					      u.ccmp.rx_pn) !=
				     offsetof(struct ieee80211_key,
					      u.gcmp.rx_pn));
			BUILD_BUG_ON(sizeof(rx->key->u.ccmp.rx_pn[queue]) !=
				     sizeof(rx->key->u.gcmp.rx_pn[queue]));
			BUILD_BUG_ON(IEEE80211_CCMP_PN_LEN !=
				     IEEE80211_GCMP_PN_LEN);
		}
	if (entry->check_sequential_pn) {
		int i;
		u8 pn[IEEE80211_CCMP_PN_LEN], *rpn;
		int queue;

		if (!rx->key ||
		    (rx->key->conf.cipher != WLAN_CIPHER_SUITE_CCMP &&
		     rx->key->conf.cipher != WLAN_CIPHER_SUITE_CCMP_256 &&
		     rx->key->conf.cipher != WLAN_CIPHER_SUITE_GCMP &&
		     rx->key->conf.cipher != WLAN_CIPHER_SUITE_GCMP_256))
			return RX_DROP_UNUSABLE;
		memcpy(pn, entry->last_pn, IEEE80211_CCMP_PN_LEN);
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])
				break;
		}