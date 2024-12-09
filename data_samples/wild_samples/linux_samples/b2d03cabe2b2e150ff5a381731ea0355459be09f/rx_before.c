	} else if (mmie_keyidx >= 0 && ieee80211_is_beacon(fc)) {
		/* Broadcast/multicast robust management frame / BIP */
		if ((status->flag & RX_FLAG_DECRYPTED) &&
		    (status->flag & RX_FLAG_IV_STRIPPED))
			return RX_CONTINUE;

		if (mmie_keyidx < NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS ||
		    mmie_keyidx >= NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS +
		    NUM_DEFAULT_BEACON_KEYS) {
			cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
						     skb->data,
						     skb->len);
			return RX_DROP_MONITOR; /* unexpected BIP keyidx */
		}
	switch (rx->key->conf.cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		result = ieee80211_crypto_wep_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		result = ieee80211_crypto_tkip_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		result = ieee80211_crypto_ccmp_decrypt(
			rx, IEEE80211_CCMP_MIC_LEN);
		break;
	case WLAN_CIPHER_SUITE_CCMP_256:
		result = ieee80211_crypto_ccmp_decrypt(
			rx, IEEE80211_CCMP_256_MIC_LEN);
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
		result = ieee80211_crypto_aes_cmac_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_BIP_CMAC_256:
		result = ieee80211_crypto_aes_cmac_256_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		result = ieee80211_crypto_aes_gmac_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		result = ieee80211_crypto_gcmp_decrypt(rx);
		break;
	default:
		result = RX_DROP_UNUSABLE;
	}

	/* the hdr variable is invalid after the decrypt handlers */

	/* either the frame has been decrypted or will be dropped */
	status->flag |= RX_FLAG_DECRYPTED;

	if (unlikely(ieee80211_is_beacon(fc) && result == RX_DROP_UNUSABLE))
		cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
					     skb->data, skb->len);

	return result;
}

void ieee80211_init_frag_cache(struct ieee80211_fragment_cache *cache)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cache->entries); i++)
		skb_queue_head_init(&cache->entries[i].skb_list);
}

void ieee80211_destroy_frag_cache(struct ieee80211_fragment_cache *cache)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cache->entries); i++)
		__skb_queue_purge(&cache->entries[i].skb_list);
}

static inline struct ieee80211_fragment_entry *
ieee80211_reassemble_add(struct ieee80211_fragment_cache *cache,
			 unsigned int frag, unsigned int seq, int rx_queue,
			 struct sk_buff **skb)
{
	struct ieee80211_fragment_entry *entry;

	entry = &cache->entries[cache->next++];
	if (cache->next >= IEEE80211_FRAGMENT_MAX)
		cache->next = 0;

	__skb_queue_purge(&entry->skb_list);

	__skb_queue_tail(&entry->skb_list, *skb); /* no need for locking */
	*skb = NULL;
	entry->first_frag_time = jiffies;
	entry->seq = seq;
	entry->rx_queue = rx_queue;
	entry->last_frag = frag;
	entry->check_sequential_pn = false;
	entry->extra_len = 0;

	return entry;
}

static inline struct ieee80211_fragment_entry *
ieee80211_reassemble_find(struct ieee80211_fragment_cache *cache,
			  unsigned int frag, unsigned int seq,
			  int rx_queue, struct ieee80211_hdr *hdr)
{
	struct ieee80211_fragment_entry *entry;
	int i, idx;

	idx = cache->next;
	for (i = 0; i < IEEE80211_FRAGMENT_MAX; i++) {
		struct ieee80211_hdr *f_hdr;
		struct sk_buff *f_skb;

		idx--;
		if (idx < 0)
			idx = IEEE80211_FRAGMENT_MAX - 1;

		entry = &cache->entries[idx];
		if (skb_queue_empty(&entry->skb_list) || entry->seq != seq ||
		    entry->rx_queue != rx_queue ||
		    entry->last_frag + 1 != frag)
			continue;

		f_skb = __skb_peek(&entry->skb_list);
		f_hdr = (struct ieee80211_hdr *) f_skb->data;

		/*
		 * Check ftype and addresses are equal, else check next fragment
		 */
		if (((hdr->frame_control ^ f_hdr->frame_control) &
		     cpu_to_le16(IEEE80211_FCTL_FTYPE)) ||
		    !ether_addr_equal(hdr->addr1, f_hdr->addr1) ||
		    !ether_addr_equal(hdr->addr2, f_hdr->addr2))
			continue;

		if (time_after(jiffies, entry->first_frag_time + 2 * HZ)) {
			__skb_queue_purge(&entry->skb_list);
			continue;
		}
		return entry;
	}