		if (!*ie_ptr) {
			*ie_ptr = kzalloc(sizeof(struct mwifiex_ie),
					  GFP_KERNEL);
			if (!*ie_ptr)
				return -ENOMEM;
			ie = *ie_ptr;
		}

		vs_ie = (struct ieee_types_header *)vendor_ie;
		if (le16_to_cpu(ie->ie_length) + vs_ie->len + 2 >
			IEEE_MAX_IE_SIZE)
			return -EINVAL;
		memcpy(ie->ie_buffer + le16_to_cpu(ie->ie_length),
		       vs_ie, vs_ie->len + 2);
		le16_unaligned_add_cpu(&ie->ie_length, vs_ie->len + 2);
		ie->mgmt_subtype_mask = cpu_to_le16(mask);
		ie->ie_index = cpu_to_le16(MWIFIEX_AUTO_IDX_MASK);
	}

	*ie_ptr = ie;
	return 0;
}

/* This function parses beacon IEs, probe response IEs, association response IEs
 * from cfg80211_ap_settings->beacon and sets these IE to FW.
 */
static int mwifiex_set_mgmt_beacon_data_ies(struct mwifiex_private *priv,
					    struct cfg80211_beacon_data *data)
{
	struct mwifiex_ie *beacon_ie = NULL, *pr_ie = NULL, *ar_ie = NULL;
	u16 beacon_idx = MWIFIEX_AUTO_IDX_MASK, pr_idx = MWIFIEX_AUTO_IDX_MASK;
	u16 ar_idx = MWIFIEX_AUTO_IDX_MASK;
	int ret = 0;

	if (data->beacon_ies && data->beacon_ies_len) {
		mwifiex_update_vs_ie(data->beacon_ies, data->beacon_ies_len,
				     &beacon_ie, MGMT_MASK_BEACON,
				     WLAN_OUI_MICROSOFT,
				     WLAN_OUI_TYPE_MICROSOFT_WPS);
		mwifiex_update_vs_ie(data->beacon_ies, data->beacon_ies_len,
				     &beacon_ie, MGMT_MASK_BEACON,
				     WLAN_OUI_WFA, WLAN_OUI_TYPE_WFA_P2P);
	}

	if (data->proberesp_ies && data->proberesp_ies_len) {
		mwifiex_update_vs_ie(data->proberesp_ies,
				     data->proberesp_ies_len, &pr_ie,
				     MGMT_MASK_PROBE_RESP, WLAN_OUI_MICROSOFT,
				     WLAN_OUI_TYPE_MICROSOFT_WPS);
		mwifiex_update_vs_ie(data->proberesp_ies,
				     data->proberesp_ies_len, &pr_ie,
				     MGMT_MASK_PROBE_RESP,
				     WLAN_OUI_WFA, WLAN_OUI_TYPE_WFA_P2P);
	}

	if (data->assocresp_ies && data->assocresp_ies_len) {
		mwifiex_update_vs_ie(data->assocresp_ies,
				     data->assocresp_ies_len, &ar_ie,
				     MGMT_MASK_ASSOC_RESP |
				     MGMT_MASK_REASSOC_RESP,
				     WLAN_OUI_MICROSOFT,
				     WLAN_OUI_TYPE_MICROSOFT_WPS);
		mwifiex_update_vs_ie(data->assocresp_ies,
				     data->assocresp_ies_len, &ar_ie,
				     MGMT_MASK_ASSOC_RESP |
				     MGMT_MASK_REASSOC_RESP, WLAN_OUI_WFA,
				     WLAN_OUI_TYPE_WFA_P2P);
	}

	if (beacon_ie || pr_ie || ar_ie) {
		ret = mwifiex_update_uap_custom_ie(priv, beacon_ie,
						   &beacon_idx, pr_ie,
						   &pr_idx, ar_ie, &ar_idx);
		if (ret)
			goto done;
	}

	priv->beacon_idx = beacon_idx;
	priv->proberesp_idx = pr_idx;
	priv->assocresp_idx = ar_idx;

done:
	kfree(beacon_ie);
	kfree(pr_ie);
	kfree(ar_ie);

	return ret;
}

/* This function parses  head and tail IEs, from cfg80211_beacon_data and sets
 * these IE to FW.
 */
static int mwifiex_uap_parse_tail_ies(struct mwifiex_private *priv,
				      struct cfg80211_beacon_data *info)
{
	struct mwifiex_ie *gen_ie;
	struct ieee_types_header *hdr;
	struct ieee80211_vendor_ie *vendorhdr;
	u16 gen_idx = MWIFIEX_AUTO_IDX_MASK, ie_len = 0;
	int left_len, parsed_len = 0;
	unsigned int token_len;
	int err = 0;

	if (!info->tail || !info->tail_len)
		return 0;

	gen_ie = kzalloc(sizeof(*gen_ie), GFP_KERNEL);
	if (!gen_ie)
		return -ENOMEM;

	left_len = info->tail_len;

	/* Many IEs are generated in FW by parsing bss configuration.
	 * Let's not add them here; else we may end up duplicating these IEs
	 */
	while (left_len > sizeof(struct ieee_types_header)) {
		hdr = (void *)(info->tail + parsed_len);
		token_len = hdr->len + sizeof(struct ieee_types_header);
		if (token_len > left_len) {
			err = -EINVAL;
			goto out;
		}