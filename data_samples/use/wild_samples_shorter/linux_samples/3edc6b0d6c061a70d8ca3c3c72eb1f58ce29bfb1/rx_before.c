	unsigned int frag, seq;
	struct ieee80211_fragment_entry *entry;
	struct sk_buff *skb;

	hdr = (struct ieee80211_hdr *)rx->skb->data;
	fc = hdr->frame_control;

				     sizeof(rx->key->u.gcmp.rx_pn[queue]));
			BUILD_BUG_ON(IEEE80211_CCMP_PN_LEN !=
				     IEEE80211_GCMP_PN_LEN);
		} else if (rx->key && ieee80211_has_protected(fc)) {
			entry->is_protected = true;
			entry->key_color = rx->key->color;
		}
		return RX_QUEUED;
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

	skb_pull(rx->skb, ieee80211_hdrlen(fc));
	__skb_queue_tail(&entry->skb_list, rx->skb);