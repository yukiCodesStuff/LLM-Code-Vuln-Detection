	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID, ie, ielen) {
		if (elem->datalen < 4)
			continue;
		for_each_element(sub, elem->data + 1, elem->datalen - 1) {
			u8 profile_len;

			if (sub->id != 0 || sub->datalen < 4) {