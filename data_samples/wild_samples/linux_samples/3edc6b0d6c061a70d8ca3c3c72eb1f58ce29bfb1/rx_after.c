{
	struct ieee80211_fragment_cache *cache = &rx->sdata->frags;
	struct ieee80211_hdr *hdr;
	u16 sc;
	__le16 fc;
	unsigned int frag, seq;
	struct ieee80211_fragment_entry *entry;
	struct sk_buff *skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

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
		} else if (rx->key &&
			   (ieee80211_has_protected(fc) ||
			    (status->flag & RX_FLAG_DECRYPTED))) {
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
		   (!rx->key ||
		    (!ieee80211_has_protected(fc) &&
		     !(status->flag & RX_FLAG_DECRYPTED)) ||
		    rx->key->color != entry->key_color)) {
		/* Drop this as a mixed key or fragment cache attack, even
		 * if for TKIP Michael MIC should protect us, and WEP is a
		 * lost cause anyway.
		 */
		return RX_DROP_UNUSABLE;
	} else if (entry->is_protected && rx->key &&
		   entry->key_color != rx->key->color &&
		   (status->flag & RX_FLAG_DECRYPTED)) {
		return RX_DROP_UNUSABLE;
	}

	skb_pull(rx->skb, ieee80211_hdrlen(fc));
	__skb_queue_tail(&entry->skb_list, rx->skb);
	entry->last_frag = frag;
	entry->extra_len += rx->skb->len;
	if (ieee80211_has_morefrags(fc)) {
		rx->skb = NULL;
		return RX_QUEUED;
	}

	rx->skb = __skb_dequeue(&entry->skb_list);
	if (skb_tailroom(rx->skb) < entry->extra_len) {
		I802_DEBUG_INC(rx->local->rx_expand_skb_head_defrag);
		if (unlikely(pskb_expand_head(rx->skb, 0, entry->extra_len,
					      GFP_ATOMIC))) {
			I802_DEBUG_INC(rx->local->rx_handlers_drop_defrag);
			__skb_queue_purge(&entry->skb_list);
			return RX_DROP_UNUSABLE;
		}
	}
	while ((skb = __skb_dequeue(&entry->skb_list))) {
		skb_put_data(rx->skb, skb->data, skb->len);
		dev_kfree_skb(skb);
	}

 out:
	ieee80211_led_rx(rx->local);
 out_no_led:
	if (rx->sta)
		rx->sta->rx_stats.packets++;
	return RX_CONTINUE;
}