		}
		switch (element_id) {
		case WLAN_EID_SSID:
			bss_entry->ssid.ssid_len = element_len;
			memcpy(bss_entry->ssid.ssid, (current_ptr + 2),
			       element_len);
			mwifiex_dbg(adapter, INFO,
			break;

		case WLAN_EID_SUPP_RATES:
			memcpy(bss_entry->data_rates, current_ptr + 2,
			       element_len);
			memcpy(bss_entry->supported_rates, current_ptr + 2,
			       element_len);