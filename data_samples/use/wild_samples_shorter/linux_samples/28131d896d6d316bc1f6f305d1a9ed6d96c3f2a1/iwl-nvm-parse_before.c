		cpu_to_le16(IEEE80211_VHT_EXT_NSS_BW_CAPABLE);
}

static const struct ieee80211_sband_iftype_data iwl_he_capa[] = {
	{
		.types_mask = BIT(NL80211_IFTYPE_STATION),
		.he_cap = {
	if (fw_has_capa(&fw->ucode_capa, IWL_UCODE_TLV_CAPA_BROADCAST_TWT))
		iftype_data->he_cap.he_cap_elem.mac_cap_info[2] |=
			IEEE80211_HE_MAC_CAP2_BCAST_TWT;
}

static void iwl_init_he_hw_capab(struct iwl_trans *trans,
				 struct iwl_nvm_data *data,