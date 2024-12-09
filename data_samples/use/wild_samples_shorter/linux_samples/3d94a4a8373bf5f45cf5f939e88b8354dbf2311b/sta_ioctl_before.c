			    "11D: skip setting domain info in FW\n");
		return 0;
	}
	memcpy(priv->adapter->country_code, &country_ie[2], 2);

	domain_info->country_code[0] = country_ie[2];
	domain_info->country_code[1] = country_ie[3];
	priv->scan_block = false;

	if (bss) {
		if (adapter->region_code == 0x00)
			mwifiex_process_country_ie(priv, bss);

		/* Allocate and fill new bss descriptor */
		bss_desc = kzalloc(sizeof(struct mwifiex_bssdescriptor),
				   GFP_KERNEL);