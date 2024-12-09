	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, start, len) {
		if (elem->datalen < 2)
			continue;

		for_each_element(sub, elem->data + 1, elem->datalen - 1) {
			u8 new_bssid[ETH_ALEN];
			const u8 *index;