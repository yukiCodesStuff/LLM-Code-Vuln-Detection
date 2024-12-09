			    "11D: skip setting domain info in FW\n");
		return 0;
	}

	if (country_ie_len >
	    (IEEE80211_COUNTRY_STRING_LEN + MWIFIEX_MAX_TRIPLET_802_11D)) {
		mwifiex_dbg(priv->adapter, ERROR,
			    "11D: country_ie_len overflow!, deauth AP\n");
		return -EINVAL;
	}

	memcpy(priv->adapter->country_code, &country_ie[2], 2);

	domain_info->country_code[0] = country_ie[2];
	domain_info->country_code[1] = country_ie[3];
	priv->scan_block = false;

	if (bss) {
		if (adapter->region_code == 0x00 &&
		    mwifiex_process_country_ie(priv, bss))
			return -EINVAL;

		/* Allocate and fill new bss descriptor */
		bss_desc = kzalloc(sizeof(struct mwifiex_bssdescriptor),
				   GFP_KERNEL);