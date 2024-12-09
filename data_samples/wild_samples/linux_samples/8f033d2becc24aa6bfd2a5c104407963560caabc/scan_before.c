	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, ie, ielen) {
		if (elem->datalen < 4)
			continue;
		for_each_element(sub, elem->data + 1, elem->datalen - 1) {
			u8 profile_len;

			if (sub->id != 0 || sub->datalen < 4) {
				/* not a valid BSS profile */
				continue;
			}

			if (sub->data[0] != WLAN_EID_NON_TX_BSSID_CAP ||
			    sub->data[1] != 2) {
				/* The first element within the Nontransmitted
				 * BSSID Profile is not the Nontransmitted
				 * BSSID Capability element.
				 */
				continue;
			}

			memset(profile, 0, ielen);
			profile_len = cfg80211_merge_profile(ie, ielen,
							     elem,
							     sub,
							     profile,
							     ielen);

			/* found a Nontransmitted BSSID Profile */
			mbssid_index_ie = cfg80211_find_ie
				(WLAN_EID_MULTI_BSSID_IDX,
				 profile, profile_len);
			if (!mbssid_index_ie || mbssid_index_ie[1] < 1 ||
			    mbssid_index_ie[2] == 0 ||
			    mbssid_index_ie[2] > 46) {
				/* No valid Multiple BSSID-Index element */
				continue;
			}

			if (seen_indices & BIT_ULL(mbssid_index_ie[2]))
				/* We don't support legacy split of a profile */
				net_dbg_ratelimited("Partial info for BSSID index %d\n",
						    mbssid_index_ie[2]);

			seen_indices |= BIT_ULL(mbssid_index_ie[2]);

			non_tx_data->bssid_index = mbssid_index_ie[2];
			non_tx_data->max_bssid_indicator = elem->data[0];

			cfg80211_gen_new_bssid(bssid,
					       non_tx_data->max_bssid_indicator,
					       non_tx_data->bssid_index,
					       new_bssid);
			memset(new_ie, 0, IEEE80211_MAX_DATA_LEN);
			new_ie_len = cfg80211_gen_new_ie(ie, ielen,
							 profile,
							 profile_len, new_ie,
							 gfp);
			if (!new_ie_len)
				continue;

			capability = get_unaligned_le16(profile + 2);
			bss = cfg80211_inform_single_bss_data(wiphy, data,
							      ftype,
							      new_bssid, tsf,
							      capability,
							      beacon_interval,
							      new_ie,
							      new_ie_len,
							      non_tx_data,
							      gfp);
			if (!bss)
				break;
			cfg80211_put_bss(wiphy, bss);
		}