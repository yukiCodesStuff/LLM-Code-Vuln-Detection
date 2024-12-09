	    !force_assoc_off) {
		struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
		u8 ap_sta_id = mvmvif->ap_sta_id;
		u32 dtim_offs;

		/*
		 * The DTIM count counts down, so when it is N that means N
		 * more beacon intervals happen until the DTIM TBTT. Therefore
		 * add this to the current time. If that ends up being in the
		 * future, the firmware will handle it.
		 *
		 * Also note that the system_timestamp (which we get here as
		 * "sync_device_ts") and TSF timestamp aren't at exactly the
		 * same offset in the frame -- the TSF is at the first symbol
		 * of the TSF, the system timestamp is at signal acquisition
		 * time. This means there's an offset between them of at most
		 * a few hundred microseconds (24 * 8 bits + PLCP time gives
		 * 384us in the longest case), this is currently not relevant
		 * as the firmware wakes up around 2ms before the TBTT.
		 */
		dtim_offs = vif->bss_conf.sync_dtim_count *
				vif->bss_conf.beacon_int;
		/* convert TU to usecs */
		dtim_offs *= 1024;

		ctxt_sta->dtim_tsf =
			cpu_to_le64(vif->bss_conf.sync_tsf + dtim_offs);
		ctxt_sta->dtim_time =
			cpu_to_le32(vif->bss_conf.sync_device_ts + dtim_offs);
		ctxt_sta->assoc_beacon_arrive_time =
			cpu_to_le32(vif->bss_conf.sync_device_ts);

		IWL_DEBUG_INFO(mvm, "DTIM TBTT is 0x%llx/0x%x, offset %d\n",
			       le64_to_cpu(ctxt_sta->dtim_tsf),
			       le32_to_cpu(ctxt_sta->dtim_time),
			       dtim_offs);

		ctxt_sta->is_assoc = cpu_to_le32(1);

		/*
		 * allow multicast data frames only as long as the station is
		 * authorized, i.e., GTK keys are already installed (if needed)
		 */
		if (ap_sta_id < mvm->fw->ucode_capa.num_stations) {
			struct ieee80211_sta *sta;

			rcu_read_lock();

			sta = rcu_dereference(mvm->fw_id_to_mac_id[ap_sta_id]);
			if (!IS_ERR_OR_NULL(sta)) {
				struct iwl_mvm_sta *mvmsta =
					iwl_mvm_sta_from_mac80211(sta);

				if (mvmsta->sta_state ==
				    IEEE80211_STA_AUTHORIZED)
					cmd.filter_flags |=
						cpu_to_le32(MAC_FILTER_ACCEPT_GRP);
			}

			rcu_read_unlock();
		}
{
	u8 rate;
	if (info->band == NL80211_BAND_2GHZ && !vif->p2p)
		rate = IWL_FIRST_CCK_RATE;
	else
		rate = IWL_FIRST_OFDM_RATE;

	return rate;
}

static void iwl_mvm_mac_ctxt_set_tx(struct iwl_mvm *mvm,
				    struct ieee80211_vif *vif,
				    struct sk_buff *beacon,
				    struct iwl_tx_cmd *tx)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct ieee80211_tx_info *info;
	u8 rate;
	u32 tx_flags;

	info = IEEE80211_SKB_CB(beacon);

	/* Set up TX command fields */
	tx->len = cpu_to_le16((u16)beacon->len);
	tx->sta_id = mvmvif->bcast_sta.sta_id;
	tx->life_time = cpu_to_le32(TX_CMD_LIFE_TIME_INFINITE);
	tx_flags = TX_CMD_FLG_SEQ_CTL | TX_CMD_FLG_TSF;
	tx_flags |=
		iwl_mvm_bt_coex_tx_prio(mvm, (void *)beacon->data, info, 0) <<
						TX_CMD_FLG_BT_PRIO_POS;
	tx->tx_flags = cpu_to_le32(tx_flags);

	if (!fw_has_capa(&mvm->fw->ucode_capa,
			 IWL_UCODE_TLV_CAPA_BEACON_ANT_SELECTION))
		iwl_mvm_toggle_tx_ant(mvm, &mvm->mgmt_last_antenna_idx);

	tx->rate_n_flags =
		cpu_to_le32(BIT(mvm->mgmt_last_antenna_idx) <<
			    RATE_MCS_ANT_POS);

	rate = iwl_mvm_mac_ctxt_get_lowest_rate(info, vif);

	tx->rate_n_flags |= cpu_to_le32(iwl_mvm_mac80211_idx_to_hwrate(rate));
	if (rate == IWL_FIRST_CCK_RATE)
		tx->rate_n_flags |= cpu_to_le32(RATE_MCS_CCK_MSK);

}

int iwl_mvm_mac_ctxt_send_beacon_cmd(struct iwl_mvm *mvm,
				     struct sk_buff *beacon,
				     void *data, int len)
{
	struct iwl_host_cmd cmd = {
		.id = BEACON_TEMPLATE_CMD,
		.flags = CMD_ASYNC,
	};

	cmd.len[0] = len;
	cmd.data[0] = data;
	cmd.dataflags[0] = 0;
	cmd.len[1] = beacon->len;
	cmd.data[1] = beacon->data;
	cmd.dataflags[1] = IWL_HCMD_DFL_DUP;

	return iwl_mvm_send_cmd(mvm, &cmd);
}
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct ieee80211_tx_info *info;
	u8 rate;
	u32 tx_flags;

	info = IEEE80211_SKB_CB(beacon);

	/* Set up TX command fields */
	tx->len = cpu_to_le16((u16)beacon->len);
	tx->sta_id = mvmvif->bcast_sta.sta_id;
	tx->life_time = cpu_to_le32(TX_CMD_LIFE_TIME_INFINITE);
	tx_flags = TX_CMD_FLG_SEQ_CTL | TX_CMD_FLG_TSF;
	tx_flags |=
		iwl_mvm_bt_coex_tx_prio(mvm, (void *)beacon->data, info, 0) <<
						TX_CMD_FLG_BT_PRIO_POS;
	tx->tx_flags = cpu_to_le32(tx_flags);

	if (!fw_has_capa(&mvm->fw->ucode_capa,
			 IWL_UCODE_TLV_CAPA_BEACON_ANT_SELECTION))
		iwl_mvm_toggle_tx_ant(mvm, &mvm->mgmt_last_antenna_idx);

	tx->rate_n_flags =
		cpu_to_le32(BIT(mvm->mgmt_last_antenna_idx) <<
			    RATE_MCS_ANT_POS);

	rate = iwl_mvm_mac_ctxt_get_lowest_rate(info, vif);

	tx->rate_n_flags |= cpu_to_le32(iwl_mvm_mac80211_idx_to_hwrate(rate));
	if (rate == IWL_FIRST_CCK_RATE)
		tx->rate_n_flags |= cpu_to_le32(RATE_MCS_CCK_MSK);

}

int iwl_mvm_mac_ctxt_send_beacon_cmd(struct iwl_mvm *mvm,
				     struct sk_buff *beacon,
				     void *data, int len)
{
	struct iwl_host_cmd cmd = {
		.id = BEACON_TEMPLATE_CMD,
		.flags = CMD_ASYNC,
	};

	cmd.len[0] = len;
	cmd.data[0] = data;
	cmd.dataflags[0] = 0;
	cmd.len[1] = beacon->len;
	cmd.data[1] = beacon->data;
	cmd.dataflags[1] = IWL_HCMD_DFL_DUP;

	return iwl_mvm_send_cmd(mvm, &cmd);
}
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(beacon);
	struct iwl_mac_beacon_cmd beacon_cmd = {};
	u8 rate = iwl_mvm_mac_ctxt_get_lowest_rate(info, vif);
	u16 flags;
	struct ieee80211_chanctx_conf *ctx;
	int channel;

	flags = iwl_mvm_mac80211_idx_to_hwrate(rate);

	if (rate == IWL_FIRST_CCK_RATE)
		flags |= IWL_MAC_BEACON_CCK;

	/* Enable FILS on PSC channels only */
	rcu_read_lock();
	ctx = rcu_dereference(vif->chanctx_conf);
	channel = ieee80211_frequency_to_channel(ctx->def.chan->center_freq);
	WARN_ON(channel == 0);
	if (cfg80211_channel_is_psc(ctx->def.chan) &&
	    !IWL_MVM_DISABLE_AP_FILS) {
		flags |= IWL_MAC_BEACON_FILS;
		beacon_cmd.short_ssid =
			cpu_to_le32(~crc32_le(~0, vif->bss_conf.ssid,
					      vif->bss_conf.ssid_len));
	}
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(beacon);
	struct iwl_mac_beacon_cmd beacon_cmd = {};
	u8 rate = iwl_mvm_mac_ctxt_get_lowest_rate(info, vif);
	u16 flags;
	struct ieee80211_chanctx_conf *ctx;
	int channel;

	flags = iwl_mvm_mac80211_idx_to_hwrate(rate);

	if (rate == IWL_FIRST_CCK_RATE)
		flags |= IWL_MAC_BEACON_CCK;

	/* Enable FILS on PSC channels only */
	rcu_read_lock();
	ctx = rcu_dereference(vif->chanctx_conf);
	channel = ieee80211_frequency_to_channel(ctx->def.chan->center_freq);
	WARN_ON(channel == 0);
	if (cfg80211_channel_is_psc(ctx->def.chan) &&
	    !IWL_MVM_DISABLE_AP_FILS) {
		flags |= IWL_MAC_BEACON_FILS;
		beacon_cmd.short_ssid =
			cpu_to_le32(~crc32_le(~0, vif->bss_conf.ssid,
					      vif->bss_conf.ssid_len));
	}
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_probe_resp_data_notif *notif = (void *)pkt->data;
	struct iwl_probe_resp_data *old_data, *new_data;
	u32 id = le32_to_cpu(notif->mac_id);
	struct ieee80211_vif *vif;
	struct iwl_mvm_vif *mvmvif;

	IWL_DEBUG_INFO(mvm, "Probe response data notif: noa %d, csa %d\n",
		       notif->noa_active, notif->csa_counter);

	vif = iwl_mvm_rcu_dereference_vif_id(mvm, id, false);
	if (!vif)
		return;

	mvmvif = iwl_mvm_vif_from_mac80211(vif);

	new_data = kzalloc(sizeof(*new_data), GFP_KERNEL);
	if (!new_data)
		return;

	memcpy(&new_data->notif, notif, sizeof(new_data->notif));

	/* noa_attr contains 1 reserved byte, need to substruct it */
	new_data->noa_len = sizeof(struct ieee80211_vendor_ie) +
			    sizeof(new_data->notif.noa_attr) - 1;

	/*
	 * If it's a one time NoA, only one descriptor is needed,
	 * adjust the length according to len_low.
	 */
	if (new_data->notif.noa_attr.len_low ==
	    sizeof(struct ieee80211_p2p_noa_desc) + 2)
		new_data->noa_len -= sizeof(struct ieee80211_p2p_noa_desc);

	old_data = rcu_dereference_protected(mvmvif->probe_resp_data,
					lockdep_is_held(&mvmvif->mvm->mutex));
	rcu_assign_pointer(mvmvif->probe_resp_data, new_data);

	if (old_data)
		kfree_rcu(old_data, rcu_head);

	if (notif->csa_counter != IWL_PROBE_RESP_DATA_NO_CSA &&
	    notif->csa_counter >= 1)
		ieee80211_beacon_set_cntdwn(vif, notif->csa_counter);
}

void iwl_mvm_channel_switch_noa_notif(struct iwl_mvm *mvm,
				      struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_channel_switch_noa_notif *notif = (void *)pkt->data;
	struct ieee80211_vif *csa_vif, *vif;
	struct iwl_mvm_vif *mvmvif;
	u32 id_n_color, csa_id, mac_id;

	id_n_color = le32_to_cpu(notif->id_and_color);
	mac_id = id_n_color & FW_CTXT_ID_MSK;

	if (WARN_ON_ONCE(mac_id >= NUM_MAC_INDEX_DRIVER))
		return;

	rcu_read_lock();
	vif = rcu_dereference(mvm->vif_id_to_mac[mac_id]);
	mvmvif = iwl_mvm_vif_from_mac80211(vif);

	switch (vif->type) {
	case NL80211_IFTYPE_AP:
		csa_vif = rcu_dereference(mvm->csa_vif);
		if (WARN_ON(!csa_vif || !csa_vif->csa_active ||
			    csa_vif != vif))
			goto out_unlock;

		csa_id = FW_CMD_ID_AND_COLOR(mvmvif->id, mvmvif->color);
		if (WARN(csa_id != id_n_color,
			 "channel switch noa notification on unexpected vif (csa_vif=%d, notif=%d)",
			 csa_id, id_n_color))
			goto out_unlock;

		IWL_DEBUG_INFO(mvm, "Channel Switch Started Notification\n");

		schedule_delayed_work(&mvm->cs_tx_unblock_dwork,
				      msecs_to_jiffies(IWL_MVM_CS_UNBLOCK_TX_TIMEOUT *
						       csa_vif->bss_conf.beacon_int));

		ieee80211_csa_finish(csa_vif);

		rcu_read_unlock();

		RCU_INIT_POINTER(mvm->csa_vif, NULL);
		return;
	case NL80211_IFTYPE_STATION:
		iwl_mvm_csa_client_absent(mvm, vif);
		cancel_delayed_work(&mvmvif->csa_work);
		ieee80211_chswitch_done(vif, true);
		break;
	default:
		/* should never happen */
		WARN_ON_ONCE(1);
		break;
	}