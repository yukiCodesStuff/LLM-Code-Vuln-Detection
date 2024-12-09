	struct ath_common *common = ath9k_hw_common(ah);

	/*
	 * Pick the MAC address of the first interface as the new hardware
	 * MAC address. The hardware will use it together with the BSSID mask
	 * when matching addresses.
	 */
	memset(iter_data, 0, sizeof(*iter_data));
	memset(&iter_data->mask, 0xff, ETH_ALEN);
