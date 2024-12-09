	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, start, len) {
		if (elem->datalen < 2)
			continue;

		for_each_element(sub, elem->data + 1, elem->datalen - 1) {
			u8 new_bssid[ETH_ALEN];
			const u8 *index;

			if (sub->id != 0 || sub->datalen < 4) {
				/* not a valid BSS profile */
				continue;
			}

			if (sub->data[0] != WLAN_EID_NON_TX_BSSID_CAP ||
			    sub->data[1] != 2) {
				/* The first element of the
				 * Nontransmitted BSSID Profile is not
				 * the Nontransmitted BSSID Capability
				 * element.
				 */
				continue;
			}

			memset(nontransmitted_profile, 0, len);
			profile_len = cfg80211_merge_profile(start, len,
							     elem,
							     sub,
							     nontransmitted_profile,
							     len);

			/* found a Nontransmitted BSSID Profile */
			index = cfg80211_find_ie(WLAN_EID_MULTI_BSSID_IDX,
						 nontransmitted_profile,
						 profile_len);
			if (!index || index[1] < 1 || index[2] == 0) {
				/* Invalid MBSSID Index element */
				continue;
			}

			cfg80211_gen_new_bssid(bss->transmitted_bss->bssid,
					       elem->data[0],
					       index[2],
					       new_bssid);
			if (ether_addr_equal(new_bssid, bss->bssid)) {
				found = true;
				elems->bssid_index_len = index[1];
				elems->bssid_index = (void *)&index[2];
				break;
			}
		}