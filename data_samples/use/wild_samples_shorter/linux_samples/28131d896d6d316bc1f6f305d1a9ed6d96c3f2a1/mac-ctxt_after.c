
		ctxt_sta->is_assoc = cpu_to_le32(1);

		if (!mvmvif->authorized &&
		    fw_has_capa(&mvm->fw->ucode_capa,
				IWL_UCODE_TLV_CAPA_COEX_HIGH_PRIO))
			ctxt_sta->data_policy |=
				cpu_to_le32(COEX_HIGH_PRIORITY_ENABLE);

		/*
		 * allow multicast data frames only as long as the station is
		 * authorized, i.e., GTK keys are already installed (if needed)
		 */
	return rate;
}

u16 iwl_mvm_mac_ctxt_get_beacon_flags(const struct iwl_fw *fw, u8 rate_idx)
{
	u16 flags = iwl_mvm_mac80211_idx_to_hwrate(fw, rate_idx);
	bool is_new_rate = iwl_fw_lookup_cmd_ver(fw,
						 LONG_GROUP,
						 BEACON_TEMPLATE_CMD,
						 0) > 10;

	if (rate_idx <= IWL_FIRST_CCK_RATE)
		flags |= is_new_rate ? IWL_MAC_BEACON_CCK
			  : IWL_MAC_BEACON_CCK_V1;

	return flags;
}

static void iwl_mvm_mac_ctxt_set_tx(struct iwl_mvm *mvm,
				    struct ieee80211_vif *vif,
				    struct sk_buff *beacon,
				    struct iwl_tx_cmd *tx)

	rate = iwl_mvm_mac_ctxt_get_lowest_rate(info, vif);

	tx->rate_n_flags |=
		cpu_to_le32(iwl_mvm_mac80211_idx_to_hwrate(mvm->fw, rate));
	if (rate == IWL_FIRST_CCK_RATE)
		tx->rate_n_flags |= cpu_to_le32(RATE_MCS_CCK_MSK_V1);

}

int iwl_mvm_mac_ctxt_send_beacon_cmd(struct iwl_mvm *mvm,
	u16 flags;
	struct ieee80211_chanctx_conf *ctx;
	int channel;
	flags = iwl_mvm_mac_ctxt_get_beacon_flags(mvm->fw, rate);

	/* Enable FILS on PSC channels only */
	rcu_read_lock();
	ctx = rcu_dereference(vif->chanctx_conf);
	WARN_ON(channel == 0);
	if (cfg80211_channel_is_psc(ctx->def.chan) &&
	    !IWL_MVM_DISABLE_AP_FILS) {
		flags |= iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					       BEACON_TEMPLATE_CMD,
					       0) > 10 ?
			IWL_MAC_BEACON_FILS :
			IWL_MAC_BEACON_FILS_V1;
		beacon_cmd.short_ssid =
			cpu_to_le32(~crc32_le(~0, vif->bss_conf.ssid,
					      vif->bss_conf.ssid_len));
	}
		ieee80211_beacon_set_cntdwn(vif, notif->csa_counter);
}

void iwl_mvm_channel_switch_start_notif(struct iwl_mvm *mvm,
					struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_channel_switch_start_notif *notif = (void *)pkt->data;
	struct ieee80211_vif *csa_vif, *vif;
	struct iwl_mvm_vif *mvmvif;
	u32 id_n_color, csa_id, mac_id;
