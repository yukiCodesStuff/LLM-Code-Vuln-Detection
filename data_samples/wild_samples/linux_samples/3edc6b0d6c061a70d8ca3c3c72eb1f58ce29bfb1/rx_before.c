{
	struct ieee80211_fragment_cache *cache = &rx->sdata->frags;
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
		if (requires_sequential_pn(rx, fc)) {
			int queue = rx->security_idx;

			/* Store CCMP/GCMP PN so that we can verify that the
			 * next fragment has a sequential PN value.
			 */
			entry->check_sequential_pn = true;
			entry->is_protected = true;
			entry->key_color = rx->key->color;
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
		} else if (rx->key && ieee80211_has_protected(fc)) {
			entry->is_protected = true;
			entry->key_color = rx->key->color;
		}
		for (i = IEEE80211_CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])
				break;
		}

		rpn = rx->ccm_gcm.pn;
		if (memcmp(pn, rpn, IEEE80211_CCMP_PN_LEN))
			return RX_DROP_UNUSABLE;
		memcpy(entry->last_pn, pn, IEEE80211_CCMP_PN_LEN);
	} else if (entry->is_protected &&
		   (!rx->key || !ieee80211_has_protected(fc) ||
		    rx->key->color != entry->key_color)) {
		/* Drop this as a mixed key or fragment cache attack, even
		 * if for TKIP Michael MIC should protect us, and WEP is a
		 * lost cause anyway.
		 */
		return RX_DROP_UNUSABLE;
	}