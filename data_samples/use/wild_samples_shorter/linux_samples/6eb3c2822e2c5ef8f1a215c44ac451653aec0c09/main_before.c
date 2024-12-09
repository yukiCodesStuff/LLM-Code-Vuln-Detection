	struct ath_common *common = ath9k_hw_common(ah);

	/*
	 * Use the hardware MAC address as reference, the hardware uses it
	 * together with the BSSID mask when matching addresses.
	 */
	memset(iter_data, 0, sizeof(*iter_data));
	memset(&iter_data->mask, 0xff, ETH_ALEN);
