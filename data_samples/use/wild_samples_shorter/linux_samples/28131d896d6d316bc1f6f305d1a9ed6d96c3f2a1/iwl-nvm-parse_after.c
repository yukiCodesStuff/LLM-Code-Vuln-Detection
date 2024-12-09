		cpu_to_le16(IEEE80211_VHT_EXT_NSS_BW_CAPABLE);
}

static const u8 iwl_vendor_caps[] = {
	0xdd,			/* vendor element */
	0x06,			/* length */
	0x00, 0x17, 0x35,	/* Intel OUI */
	0x08,			/* type (Intel Capabilities) */
	/* followed by 16 bits of capabilities */
#define IWL_VENDOR_CAP_IMPROVED_BF_FDBK_HE	BIT(0)
	IWL_VENDOR_CAP_IMPROVED_BF_FDBK_HE,
	0x00
};

static const struct ieee80211_sband_iftype_data iwl_he_capa[] = {
	{
		.types_mask = BIT(NL80211_IFTYPE_STATION),
		.he_cap = {
	if (fw_has_capa(&fw->ucode_capa, IWL_UCODE_TLV_CAPA_BROADCAST_TWT))
		iftype_data->he_cap.he_cap_elem.mac_cap_info[2] |=
			IEEE80211_HE_MAC_CAP2_BCAST_TWT;

	if (trans->trans_cfg->device_family == IWL_DEVICE_FAMILY_22000 &&
	    !is_ap) {
		iftype_data->vendor_elems.data = iwl_vendor_caps;
		iftype_data->vendor_elems.len = ARRAY_SIZE(iwl_vendor_caps);
	}
}

static void iwl_init_he_hw_capab(struct iwl_trans *trans,
				 struct iwl_nvm_data *data,