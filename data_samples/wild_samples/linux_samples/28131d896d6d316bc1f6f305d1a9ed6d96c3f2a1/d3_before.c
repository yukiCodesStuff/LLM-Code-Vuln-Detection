{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	return __iwl_mvm_suspend(hw, wowlan, false);
}

/* converted data from the different status responses */
struct iwl_wowlan_status_data {
	u16 pattern_number;
	u16 qos_seq_ctr[8];
	u32 wakeup_reasons;
	u32 wake_packet_length;
	u32 wake_packet_bufsize;
	const u8 *wake_packet;
};

static void iwl_mvm_report_wakeup_reasons(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status_data *status)
{
	struct sk_buff *pkt = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	u32 reasons = status->wakeup_reasons;

	if (reasons == IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS) {
		wakeup_report = NULL;
		goto report;
	}

	pm_wakeup_event(mvm->dev, 0);

	if (reasons & IWL_WOWLAN_WAKEUP_BY_MAGIC_PACKET)
		wakeup.magic_pkt = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_PATTERN)
		wakeup.pattern_idx =
			status->pattern_number;

	if (reasons & (IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		       IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH))
		wakeup.disconnect = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_GTK_REKEY_FAILURE)
		wakeup.gtk_rekey_failure = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_EAPOL_REQUEST)
		wakeup.eap_identity_req = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_FOUR_WAY_HANDSHAKE)
		wakeup.four_way_handshake = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_REM_WAKE_LINK_LOSS)
		wakeup.tcp_connlost = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_REM_WAKE_SIGNATURE_TABLE)
		wakeup.tcp_nomoretokens = true;

	if (reasons & IWL_WOWLAN_WAKEUP_BY_REM_WAKE_WAKEUP_PACKET)
		wakeup.tcp_match = true;

	if (status->wake_packet_bufsize) {
		int pktsize = status->wake_packet_bufsize;
		int pktlen = status->wake_packet_length;
		const u8 *pktdata = status->wake_packet;
		struct ieee80211_hdr *hdr = (void *)pktdata;
		int truncated = pktlen - pktsize;

		/* this would be a firmware bug */
		if (WARN_ON_ONCE(truncated < 0))
			truncated = 0;

		if (ieee80211_is_data(hdr->frame_control)) {
			int hdrlen = ieee80211_hdrlen(hdr->frame_control);
			int ivlen = 0, icvlen = 4; /* also FCS */

			pkt = alloc_skb(pktsize, GFP_KERNEL);
			if (!pkt)
				goto report;

			skb_put_data(pkt, pktdata, hdrlen);
			pktdata += hdrlen;
			pktsize -= hdrlen;

			if (ieee80211_has_protected(hdr->frame_control)) {
				/*
				 * This is unlocked and using gtk_i(c)vlen,
				 * but since everything is under RTNL still
				 * that's not really a problem - changing
				 * it would be difficult.
				 */
				if (is_multicast_ether_addr(hdr->addr1)) {
					ivlen = mvm->gtk_ivlen;
					icvlen += mvm->gtk_icvlen;
				} else {
					ivlen = mvm->ptk_ivlen;
					icvlen += mvm->ptk_icvlen;
				}
			}

			/* if truncated, FCS/ICV is (partially) gone */
			if (truncated >= icvlen) {
				icvlen = 0;
				truncated -= icvlen;
			} else {
				icvlen -= truncated;
				truncated = 0;
			}

			pktsize -= ivlen + icvlen;
			pktdata += ivlen;

			skb_put_data(pkt, pktdata, pktsize);

			if (ieee80211_data_to_8023(pkt, vif->addr, vif->type))
				goto report;
			wakeup.packet = pkt->data;
			wakeup.packet_present_len = pkt->len;
			wakeup.packet_len = pkt->len - truncated;
			wakeup.packet_80211 = false;
		} else {
			int fcslen = 4;

			if (truncated >= 4) {
				truncated -= 4;
				fcslen = 0;
			} else {
				fcslen -= truncated;
				truncated = 0;
			}
			pktsize -= fcslen;
			wakeup.packet = status->wake_packet;
			wakeup.packet_present_len = pktsize;
			wakeup.packet_len = pktlen - truncated;
			wakeup.packet_80211 = true;
		}
	}

 report:
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);
	kfree_skb(pkt);
}

static void iwl_mvm_aes_sc_to_seq(struct aes_sc *sc,
				  struct ieee80211_key_seq *seq)
{
	u64 pn;

	pn = le64_to_cpu(sc->pn);
	seq->ccmp.pn[0] = pn >> 40;
	seq->ccmp.pn[1] = pn >> 32;
	seq->ccmp.pn[2] = pn >> 24;
	seq->ccmp.pn[3] = pn >> 16;
	seq->ccmp.pn[4] = pn >> 8;
	seq->ccmp.pn[5] = pn;
}

static void iwl_mvm_tkip_sc_to_seq(struct tkip_sc *sc,
				   struct ieee80211_key_seq *seq)
{
	seq->tkip.iv32 = le32_to_cpu(sc->iv32);
	seq->tkip.iv16 = le16_to_cpu(sc->iv16);
}

static void iwl_mvm_set_aes_rx_seq(struct iwl_mvm *mvm, struct aes_sc *scs,
				   struct ieee80211_sta *sta,
				   struct ieee80211_key_conf *key)
{
	int tid;

	BUILD_BUG_ON(IWL_NUM_RSC != IEEE80211_NUM_TIDS);

	if (sta && iwl_mvm_has_new_rx_api(mvm)) {
		struct iwl_mvm_sta *mvmsta;
		struct iwl_mvm_key_pn *ptk_pn;

		mvmsta = iwl_mvm_sta_from_mac80211(sta);

		rcu_read_lock();
		ptk_pn = rcu_dereference(mvmsta->ptk_pn[key->keyidx]);
		if (WARN_ON(!ptk_pn)) {
			rcu_read_unlock();
			return;
		}

		for (tid = 0; tid < IWL_MAX_TID_COUNT; tid++) {
			struct ieee80211_key_seq seq = {};
			int i;

			iwl_mvm_aes_sc_to_seq(&scs[tid], &seq);
			ieee80211_set_key_rx_seq(key, tid, &seq);
			for (i = 1; i < mvm->trans->num_rx_queues; i++)
				memcpy(ptk_pn->q[i].pn[tid],
				       seq.ccmp.pn, IEEE80211_CCMP_PN_LEN);
		}
		rcu_read_unlock();
	} else {
		for (tid = 0; tid < IWL_NUM_RSC; tid++) {
			struct ieee80211_key_seq seq = {};

			iwl_mvm_aes_sc_to_seq(&scs[tid], &seq);
			ieee80211_set_key_rx_seq(key, tid, &seq);
		}
	}
}

static void iwl_mvm_set_tkip_rx_seq(struct tkip_sc *scs,
				    struct ieee80211_key_conf *key)
{
	int tid;

	BUILD_BUG_ON(IWL_NUM_RSC != IEEE80211_NUM_TIDS);

	for (tid = 0; tid < IWL_NUM_RSC; tid++) {
		struct ieee80211_key_seq seq = {};

		iwl_mvm_tkip_sc_to_seq(&scs[tid], &seq);
		ieee80211_set_key_rx_seq(key, tid, &seq);
	}
}

static void iwl_mvm_set_key_rx_seq(struct iwl_mvm *mvm,
				   struct ieee80211_key_conf *key,
				   struct iwl_wowlan_status *status)
{
	union iwl_all_tsc_rsc *rsc = &status->gtk[0].rsc.all_tsc_rsc;

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		iwl_mvm_set_aes_rx_seq(mvm, rsc->aes.multicast_rsc, NULL, key);
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		iwl_mvm_set_tkip_rx_seq(rsc->tkip.multicast_rsc, key);
		break;
	default:
		WARN_ON(1);
	}
}

struct iwl_mvm_d3_gtk_iter_data {
	struct iwl_mvm *mvm;
	struct iwl_wowlan_status *status;
	void *last_gtk;
	u32 cipher;
	bool find_phase, unhandled_cipher;
	int num_keys;
};

static void iwl_mvm_d3_update_keys(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   struct ieee80211_key_conf *key,
				   void *_data)
{
	struct iwl_mvm_d3_gtk_iter_data *data = _data;

	if (data->unhandled_cipher)
		return;

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		/* ignore WEP completely, nothing to do */
		return;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
	case WLAN_CIPHER_SUITE_TKIP:
		/* we support these */
		break;
	default:
		/* everything else (even CMAC for MFP) - disconnect from AP */
		data->unhandled_cipher = true;
		return;
	}

	data->num_keys++;

	/*
	 * pairwise key - update sequence counters only;
	 * note that this assumes no TDLS sessions are active
	 */
	if (sta) {
		struct ieee80211_key_seq seq = {};
		union iwl_all_tsc_rsc *sc =
			&data->status->gtk[0].rsc.all_tsc_rsc;

		if (data->find_phase)
			return;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			iwl_mvm_set_aes_rx_seq(data->mvm, sc->aes.unicast_rsc,
					       sta, key);
			atomic64_set(&key->tx_pn, le64_to_cpu(sc->aes.tsc.pn));
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			iwl_mvm_tkip_sc_to_seq(&sc->tkip.tsc, &seq);
			iwl_mvm_set_tkip_rx_seq(sc->tkip.unicast_rsc, key);
			atomic64_set(&key->tx_pn,
				     (u64)seq.tkip.iv16 |
				     ((u64)seq.tkip.iv32 << 16));
			break;
		}

		/* that's it for this key */
		return;
	}

	if (data->find_phase) {
		data->last_gtk = key;
		data->cipher = key->cipher;
		return;
	}

	if (data->status->num_of_gtk_rekeys)
		ieee80211_remove_key(key);
	else if (data->last_gtk == key)
		iwl_mvm_set_key_rx_seq(data->mvm, key, data->status);
}

static bool iwl_mvm_setup_connection_keep(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status *status)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
		.status = status,
	};
	u32 disconnection_reasons =
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH;

	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);
	/* not trying to keep connections with MFP/unhandled ciphers */
	if (gtkdata.unhandled_cipher)
		return false;
	if (!gtkdata.num_keys)
		goto out;
	if (!gtkdata.last_gtk)
		return false;

	/*
	 * invalidate all other GTKs that might still exist and update
	 * the one that we used
	 */
	gtkdata.find_phase = false;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
			u8 key[32];
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);

	/* default to 7 (when we have IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL) */
	notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
					    WOWLAN_GET_STATUSES, 0);
	if (!notif_ver)
		notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LEGACY_GROUP,
						    WOWLAN_GET_STATUSES, 7);

	if (!fw_has_api(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
{
	seq->tkip.iv32 = le32_to_cpu(sc->iv32);
	seq->tkip.iv16 = le16_to_cpu(sc->iv16);
}

static void iwl_mvm_set_aes_rx_seq(struct iwl_mvm *mvm, struct aes_sc *scs,
				   struct ieee80211_sta *sta,
				   struct ieee80211_key_conf *key)
{
	int tid;

	BUILD_BUG_ON(IWL_NUM_RSC != IEEE80211_NUM_TIDS);

	if (sta && iwl_mvm_has_new_rx_api(mvm)) {
		struct iwl_mvm_sta *mvmsta;
		struct iwl_mvm_key_pn *ptk_pn;

		mvmsta = iwl_mvm_sta_from_mac80211(sta);

		rcu_read_lock();
		ptk_pn = rcu_dereference(mvmsta->ptk_pn[key->keyidx]);
		if (WARN_ON(!ptk_pn)) {
			rcu_read_unlock();
			return;
		}

		for (tid = 0; tid < IWL_MAX_TID_COUNT; tid++) {
			struct ieee80211_key_seq seq = {};
			int i;

			iwl_mvm_aes_sc_to_seq(&scs[tid], &seq);
			ieee80211_set_key_rx_seq(key, tid, &seq);
			for (i = 1; i < mvm->trans->num_rx_queues; i++)
				memcpy(ptk_pn->q[i].pn[tid],
				       seq.ccmp.pn, IEEE80211_CCMP_PN_LEN);
		}
		rcu_read_unlock();
	} else {
		for (tid = 0; tid < IWL_NUM_RSC; tid++) {
			struct ieee80211_key_seq seq = {};

			iwl_mvm_aes_sc_to_seq(&scs[tid], &seq);
			ieee80211_set_key_rx_seq(key, tid, &seq);
		}
	}
}
struct iwl_mvm_d3_gtk_iter_data {
	struct iwl_mvm *mvm;
	struct iwl_wowlan_status *status;
	void *last_gtk;
	u32 cipher;
	bool find_phase, unhandled_cipher;
	int num_keys;
};

static void iwl_mvm_d3_update_keys(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   struct ieee80211_key_conf *key,
				   void *_data)
{
	struct iwl_mvm_d3_gtk_iter_data *data = _data;

	if (data->unhandled_cipher)
		return;

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		/* ignore WEP completely, nothing to do */
		return;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
	case WLAN_CIPHER_SUITE_TKIP:
		/* we support these */
		break;
	default:
		/* everything else (even CMAC for MFP) - disconnect from AP */
		data->unhandled_cipher = true;
		return;
	}

	data->num_keys++;

	/*
	 * pairwise key - update sequence counters only;
	 * note that this assumes no TDLS sessions are active
	 */
	if (sta) {
		struct ieee80211_key_seq seq = {};
		union iwl_all_tsc_rsc *sc =
			&data->status->gtk[0].rsc.all_tsc_rsc;

		if (data->find_phase)
			return;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			iwl_mvm_set_aes_rx_seq(data->mvm, sc->aes.unicast_rsc,
					       sta, key);
			atomic64_set(&key->tx_pn, le64_to_cpu(sc->aes.tsc.pn));
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			iwl_mvm_tkip_sc_to_seq(&sc->tkip.tsc, &seq);
			iwl_mvm_set_tkip_rx_seq(sc->tkip.unicast_rsc, key);
			atomic64_set(&key->tx_pn,
				     (u64)seq.tkip.iv16 |
				     ((u64)seq.tkip.iv32 << 16));
			break;
		}

		/* that's it for this key */
		return;
	}

	if (data->find_phase) {
		data->last_gtk = key;
		data->cipher = key->cipher;
		return;
	}

	if (data->status->num_of_gtk_rekeys)
		ieee80211_remove_key(key);
	else if (data->last_gtk == key)
		iwl_mvm_set_key_rx_seq(data->mvm, key, data->status);
}

static bool iwl_mvm_setup_connection_keep(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status *status)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
		.status = status,
	};
	u32 disconnection_reasons =
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH;

	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);
	/* not trying to keep connections with MFP/unhandled ciphers */
	if (gtkdata.unhandled_cipher)
		return false;
	if (!gtkdata.num_keys)
		goto out;
	if (!gtkdata.last_gtk)
		return false;

	/*
	 * invalidate all other GTKs that might still exist and update
	 * the one that we used
	 */
	gtkdata.find_phase = false;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
			u8 key[32];
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);

	/* default to 7 (when we have IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL) */
	notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
					    WOWLAN_GET_STATUSES, 0);
	if (!notif_ver)
		notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LEGACY_GROUP,
						    WOWLAN_GET_STATUSES, 7);

	if (!fw_has_api(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
{
	struct iwl_mvm_d3_gtk_iter_data *data = _data;

	if (data->unhandled_cipher)
		return;

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		/* ignore WEP completely, nothing to do */
		return;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
	case WLAN_CIPHER_SUITE_TKIP:
		/* we support these */
		break;
	default:
		/* everything else (even CMAC for MFP) - disconnect from AP */
		data->unhandled_cipher = true;
		return;
	}

	data->num_keys++;

	/*
	 * pairwise key - update sequence counters only;
	 * note that this assumes no TDLS sessions are active
	 */
	if (sta) {
		struct ieee80211_key_seq seq = {};
		union iwl_all_tsc_rsc *sc =
			&data->status->gtk[0].rsc.all_tsc_rsc;

		if (data->find_phase)
			return;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			iwl_mvm_set_aes_rx_seq(data->mvm, sc->aes.unicast_rsc,
					       sta, key);
			atomic64_set(&key->tx_pn, le64_to_cpu(sc->aes.tsc.pn));
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			iwl_mvm_tkip_sc_to_seq(&sc->tkip.tsc, &seq);
			iwl_mvm_set_tkip_rx_seq(sc->tkip.unicast_rsc, key);
			atomic64_set(&key->tx_pn,
				     (u64)seq.tkip.iv16 |
				     ((u64)seq.tkip.iv32 << 16));
			break;
		}

		/* that's it for this key */
		return;
	}

	if (data->find_phase) {
		data->last_gtk = key;
		data->cipher = key->cipher;
		return;
	}

	if (data->status->num_of_gtk_rekeys)
		ieee80211_remove_key(key);
	else if (data->last_gtk == key)
		iwl_mvm_set_key_rx_seq(data->mvm, key, data->status);
}
	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		/* ignore WEP completely, nothing to do */
		return;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
	case WLAN_CIPHER_SUITE_TKIP:
		/* we support these */
		break;
	default:
		/* everything else (even CMAC for MFP) - disconnect from AP */
		data->unhandled_cipher = true;
		return;
	}

	data->num_keys++;

	/*
	 * pairwise key - update sequence counters only;
	 * note that this assumes no TDLS sessions are active
	 */
	if (sta) {
		struct ieee80211_key_seq seq = {};
		union iwl_all_tsc_rsc *sc =
			&data->status->gtk[0].rsc.all_tsc_rsc;

		if (data->find_phase)
			return;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			iwl_mvm_set_aes_rx_seq(data->mvm, sc->aes.unicast_rsc,
					       sta, key);
			atomic64_set(&key->tx_pn, le64_to_cpu(sc->aes.tsc.pn));
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			iwl_mvm_tkip_sc_to_seq(&sc->tkip.tsc, &seq);
			iwl_mvm_set_tkip_rx_seq(sc->tkip.unicast_rsc, key);
			atomic64_set(&key->tx_pn,
				     (u64)seq.tkip.iv16 |
				     ((u64)seq.tkip.iv32 << 16));
			break;
		}

		/* that's it for this key */
		return;
	}
	if (data->find_phase) {
		data->last_gtk = key;
		data->cipher = key->cipher;
		return;
	}

	if (data->status->num_of_gtk_rekeys)
		ieee80211_remove_key(key);
	else if (data->last_gtk == key)
		iwl_mvm_set_key_rx_seq(data->mvm, key, data->status);
}

static bool iwl_mvm_setup_connection_keep(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status *status)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
		.status = status,
	};
	u32 disconnection_reasons =
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH;

	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);
	/* not trying to keep connections with MFP/unhandled ciphers */
	if (gtkdata.unhandled_cipher)
		return false;
	if (!gtkdata.num_keys)
		goto out;
	if (!gtkdata.last_gtk)
		return false;

	/*
	 * invalidate all other GTKs that might still exist and update
	 * the one that we used
	 */
	gtkdata.find_phase = false;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
			u8 key[32];
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);

	/* default to 7 (when we have IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL) */
	notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
					    WOWLAN_GET_STATUSES, 0);
	if (!notif_ver)
		notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LEGACY_GROUP,
						    WOWLAN_GET_STATUSES, 7);

	if (!fw_has_api(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
		.status = status,
	};
	u32 disconnection_reasons =
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH;

	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);
	/* not trying to keep connections with MFP/unhandled ciphers */
	if (gtkdata.unhandled_cipher)
		return false;
	if (!gtkdata.num_keys)
		goto out;
	if (!gtkdata.last_gtk)
		return false;

	/*
	 * invalidate all other GTKs that might still exist and update
	 * the one that we used
	 */
	gtkdata.find_phase = false;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
			u8 key[32];
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
		.status = status,
	};
	u32 disconnection_reasons =
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_MISSED_BEACON |
		IWL_WOWLAN_WAKEUP_BY_DISCONNECTION_ON_DEAUTH;

	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);
	/* not trying to keep connections with MFP/unhandled ciphers */
	if (gtkdata.unhandled_cipher)
		return false;
	if (!gtkdata.num_keys)
		goto out;
	if (!gtkdata.last_gtk)
		return false;

	/*
	 * invalidate all other GTKs that might still exist and update
	 * the one that we used
	 */
	gtkdata.find_phase = false;
	ieee80211_iter_keys(mvm->hw, vif,
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
			u8 key[32];
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				iwlmvm_wowlan_gtk_idx(&status->gtk[0]),
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}
		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_CCMP);
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			memcpy(conf.conf.key, status->gtk[0].key,
			       WLAN_KEY_LEN_GCMP_256);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			memcpy(conf.conf.key, status->gtk[0].key, 16);
			/* leave TX MIC key zeroed, we don't use it anyway */
			memcpy(conf.conf.key +
			       NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
			       status->gtk[0].tkip_mic_key, 8);
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
		if (IS_ERR(key))
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}

out:
	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);

	/* default to 7 (when we have IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL) */
	notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
					    WOWLAN_GET_STATUSES, 0);
	if (!notif_ver)
		notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LEGACY_GROUP,
						    WOWLAN_GET_STATUSES, 7);

	if (!fw_has_api(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = le16_to_cpu(status->non_qos_seq_ctr) + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    void *_data, int len)	\
{									\
	struct iwl_wowlan_status *status;				\
	struct iwl_wowlan_status_ ##_ver *data = _data;			\
	int data_size;							\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	data_size = ALIGN(le32_to_cpu(data->wake_packet_bufsize), 4);	\
	if (len != sizeof(*data) + data_size) {				\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
	}								\
									\
	status = kzalloc(sizeof(*status) + data_size, GFP_KERNEL);	\
	if (!status)							\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = data->replay_ctr;				\
	status->pattern_number = data->pattern_number;			\
	status->non_qos_seq_ctr = data->non_qos_seq_ctr;		\
	memcpy(status->qos_seq_ctr, data->qos_seq_ctr,			\
	       sizeof(status->qos_seq_ctr));				\
	status->wakeup_reasons = data->wakeup_reasons;			\
	status->num_of_gtk_rekeys = data->num_of_gtk_rekeys;		\
	status->received_beacons = data->received_beacons;		\
	status->wake_packet_length = data->wake_packet_length;		\
	status->wake_packet_bufsize = data->wake_packet_bufsize;	\
	memcpy(status->wake_packet, data->wake_packet,			\
	       le32_to_cpu(status->wake_packet_bufsize));		\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v6)
iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static struct iwl_wowlan_status *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
		.id = WOWLAN_GET_STATUSES,
		.flags = CMD_WANT_SKB,
		.data = { &get_status_cmd, },
		.len = { sizeof(get_status_cmd), },
	};
	int ret, len;
	u8 notif_ver;
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   WOWLAN_GET_STATUSES,
					   IWL_FW_CMD_VER_UNKNOWN);

	if (cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
		cmd.len[0] = 0;

	lockdep_assert_held(&mvm->mutex);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query wakeup status (%d)\n", ret);
		return ERR_PTR(ret);
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);

	/* default to 7 (when we have IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL) */
	notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP,
					    WOWLAN_GET_STATUSES, 0);
	if (!notif_ver)
		notif_ver = iwl_fw_lookup_notif_ver(mvm->fw, LEGACY_GROUP,
						    WOWLAN_GET_STATUSES, 7);

	if (!fw_has_api(&mvm->fw->ucode_capa,
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk[0].key));
		BUILD_BUG_ON(sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk[0].tkip_mic_key));

		/* copy GTK info to the right place */
		memcpy(status->gtk[0].key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk[0].tkip_mic_key, v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));
		memcpy(&status->gtk[0].rsc, &v6->gtk.rsc,
		       sizeof(status->gtk[0].rsc));

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk[0].key_len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk[0].key_flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v7->gtk[0];
		status->igtk[0] = v7->igtk[0];
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm,
							       cmd.resp_pkt->data,
							       len);
		if (IS_ERR(status))
			goto out_free_resp;

		status->gtk[0] = v9->gtk[0];
		status->igtk[0] = v9->igtk[0];

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}

	return iwl_mvm_send_wowlan_get_status(mvm, sta_id);
}

/* releases the MVM mutex */
static bool iwl_mvm_query_wakeup_reasons(struct iwl_mvm *mvm,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
	} else {
		IWL_ERR(mvm,
			"Firmware advertises unknown WoWLAN status response %d!\n",
			notif_ver);
		status = ERR_PTR(-EIO);
	}

out_free_resp:
	iwl_free_resp(&cmd);
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					   IWL_FW_CMD_VER_UNKNOWN);
	__le32 station_id = cpu_to_le32(sta_id);
	u32 cmd_size = cmd_ver != IWL_FW_CMD_VER_UNKNOWN ? sizeof(station_id) : 0;

	if (!mvm->net_detect) {
		/* only for tracing for now */
		int ret = iwl_mvm_send_cmd_pdu(mvm, OFFLOADS_QUERY_CMD, 0,
					       cmd_size, &station_id);
		if (ret)
			IWL_ERR(mvm, "failed to query offload statistics (%d)\n", ret);
	}
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data status;
	struct iwl_wowlan_status *fw_status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	fw_status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR_OR_NULL(fw_status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 le32_to_cpu(fw_status->wakeup_reasons));

	status.pattern_number = le16_to_cpu(fw_status->pattern_number);
	for (i = 0; i < 8; i++)
		status.qos_seq_ctr[i] =
			le16_to_cpu(fw_status->qos_seq_ctr[i]);
	status.wakeup_reasons = le32_to_cpu(fw_status->wakeup_reasons);
	status.wake_packet_length =
		le32_to_cpu(fw_status->wake_packet_length);
	status.wake_packet_bufsize =
		le32_to_cpu(fw_status->wake_packet_bufsize);
	status.wake_packet = fw_status->wake_packet;

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status.qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}

	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}
	if (mvm->trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_22000) {
		i = mvm->offload_tid;
		iwl_trans_set_q_ptrs(mvm->trans,
				     mvm_ap_sta->tid_data[i].txq_id,
				     mvm_ap_sta->tid_data[i].seq_number >> 4);
	}

	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, &status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, fw_status);

	kfree(fw_status);
	return keep;

out_free:
	kfree(fw_status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}

#define ND_QUERY_BUF_LEN (sizeof(struct iwl_scan_offload_profile_match) * \
			  IWL_SCAN_MAX_PROFILES)

struct iwl_mvm_nd_query_results {
	u32 matched_profiles;
	u8 matches[ND_QUERY_BUF_LEN];
};

static int
iwl_mvm_netdetect_query_results(struct iwl_mvm *mvm,
				struct iwl_mvm_nd_query_results *results)
{
	struct iwl_scan_offload_profiles_query *query;
	struct iwl_host_cmd cmd = {
		.id = SCAN_OFFLOAD_PROFILES_QUERY_CMD,
		.flags = CMD_WANT_SKB,
	};
	int ret, len;
	size_t query_len, matches_len;
	int max_profiles = iwl_umac_scan_get_max_profiles(mvm->fw);

	ret = iwl_mvm_send_cmd(mvm, &cmd);
	if (ret) {
		IWL_ERR(mvm, "failed to query matched profiles (%d)\n", ret);
		return ret;
	}

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		query_len = sizeof(struct iwl_scan_offload_profiles_query);
		matches_len = sizeof(struct iwl_scan_offload_profile_match) *
			max_profiles;
	} else {
		query_len = sizeof(struct iwl_scan_offload_profiles_query_v1);
		matches_len = sizeof(struct iwl_scan_offload_profile_match_v1) *
			max_profiles;
	}

	len = iwl_rx_packet_payload_len(cmd.resp_pkt);
	if (len < query_len) {
		IWL_ERR(mvm, "Invalid scan offload profiles query response!\n");
		ret = -EIO;
		goto out_free_resp;
	}

	query = (void *)cmd.resp_pkt->data;

	results->matched_profiles = le32_to_cpu(query->matched_profiles);
	memcpy(results->matches, query->matches, matches_len);

#ifdef CONFIG_IWLWIFI_DEBUGFS
	mvm->last_netdetect_scans = le32_to_cpu(query->n_scans_done);
#endif

out_free_resp:
	iwl_free_resp(&cmd);
	return ret;
}

static int iwl_mvm_query_num_match_chans(struct iwl_mvm *mvm,
					 struct iwl_mvm_nd_query_results *query,
					 int idx)
{
	int n_chans = 0, i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1; i++)
			n_chans += hweight8(matches[idx].matching_channels[i]);
	}

	return n_chans;
}

static void iwl_mvm_query_set_freqs(struct iwl_mvm *mvm,
				    struct iwl_mvm_nd_query_results *query,
				    struct cfg80211_wowlan_nd_match *match,
				    int idx)
{
	int i;

	if (fw_has_api(&mvm->fw->ucode_capa,
		       IWL_UCODE_TLV_API_SCAN_OFFLOAD_CHANS)) {
		struct iwl_scan_offload_profile_match *matches =
			(struct iwl_scan_offload_profile_match *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	} else {
		struct iwl_scan_offload_profile_match_v1 *matches =
			(struct iwl_scan_offload_profile_match_v1 *)query->matches;

		for (i = 0; i < SCAN_OFFLOAD_MATCHING_CHANNELS_LEN_V1 * 8; i++)
			if (matches[idx].matching_channels[i / 8] & (BIT(i % 8)))
				match->channels[match->n_channels++] =
					mvm->nd_channels[i]->center_freq;
	}
}

static void iwl_mvm_query_netdetect_reasons(struct iwl_mvm *mvm,
					    struct ieee80211_vif *vif)
{
	struct cfg80211_wowlan_nd_info *net_detect = NULL;
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;

	if (reasons != IWL_WOWLAN_WAKEUP_BY_NON_WIRELESS)
		goto out;

	ret = iwl_mvm_netdetect_query_results(mvm, &query);
	if (ret || !query.matched_profiles) {
		wakeup_report = NULL;
		goto out;
	}

	matched_profiles = query.matched_profiles;
	if (mvm->n_nd_match_sets) {
		n_matches = hweight_long(matched_profiles);
	} else {
		IWL_ERR(mvm, "no net detect match information available\n");
		n_matches = 0;
	}

	net_detect = kzalloc(struct_size(net_detect, matches, n_matches),
			     GFP_KERNEL);
	if (!net_detect || !n_matches)
		goto out_report_nd;

	for_each_set_bit(i, &matched_profiles, mvm->n_nd_match_sets) {
		struct cfg80211_wowlan_nd_match *match;
		int idx, n_channels = 0;

		n_channels = iwl_mvm_query_num_match_chans(mvm, &query, i);

		match = kzalloc(struct_size(match, channels, n_channels),
				GFP_KERNEL);
		if (!match)
			goto out_report_nd;

		net_detect->matches[net_detect->n_matches++] = match;

		/* We inverted the order of the SSIDs in the scan
		 * request, so invert the index here.
		 */
		idx = mvm->n_nd_match_sets - i - 1;
		match->ssid.ssid_len = mvm->nd_match_sets[idx].ssid.ssid_len;
		memcpy(match->ssid.ssid, mvm->nd_match_sets[idx].ssid.ssid,
		       match->ssid.ssid_len);

		if (mvm->n_nd_channels < n_channels)
			continue;

		iwl_mvm_query_set_freqs(mvm, &query, match, i);
	}

out_report_nd:
	wakeup.net_detect = net_detect;
out:
	iwl_mvm_free_nd(mvm);

	mutex_unlock(&mvm->mutex);
	ieee80211_report_wowlan_wakeup(vif, wakeup_report, GFP_KERNEL);

	if (net_detect) {
		for (i = 0; i < net_detect->n_matches; i++)
			kfree(net_detect->matches[i]);
		kfree(net_detect);
	}
}

static void iwl_mvm_d3_disconnect_iter(void *data, u8 *mac,
				       struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_resume_disconnect(vif);
}

static bool iwl_mvm_rt_status(struct iwl_trans *trans, u32 base, u32 *err_id)
{
	struct error_table_start {
		/* cf. struct iwl_error_event_table */
		u32 valid;
		__le32 err_id;
	} err_info;

	if (!base)
		return false;

	iwl_trans_read_mem_bytes(trans, base,
				 &err_info, sizeof(err_info));
	if (err_info.valid && err_id)
		*err_id = le32_to_cpu(err_info.err_id);

	return !!err_info.valid;
}

static bool iwl_mvm_check_rt_status(struct iwl_mvm *mvm,
				   struct ieee80211_vif *vif)
{
	u32 err_id;

	/* check for lmac1 error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[0],
			      &err_id)) {
		if (err_id == RF_KILL_INDICATOR_FOR_WOWLAN) {
			struct cfg80211_wowlan_wakeup wakeup = {
				.rfkill_release = true,
			};
			ieee80211_report_wowlan_wakeup(vif, &wakeup,
						       GFP_KERNEL);
		}
		return true;
	}

	/* check if we have lmac2 set and check for error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.lmac_error_event_table[1], NULL))
		return true;

	/* check for umac error */
	if (iwl_mvm_rt_status(mvm->trans,
			      mvm->trans->dbg.umac_error_event_table, NULL))
		return true;

	return false;
}

static int __iwl_mvm_resume(struct iwl_mvm *mvm, bool test)
{
	struct ieee80211_vif *vif = NULL;
	int ret = 1;
	enum iwl_d3_status d3_status;
	bool keep = false;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);
	bool d0i3_first = fw_has_capa(&mvm->fw->ucode_capa,
				      IWL_UCODE_TLV_CAPA_D0I3_END_FIRST);

	mutex_lock(&mvm->mutex);

	mvm->last_reset_or_resume_time_jiffies = jiffies;

	/* get the BSS vif pointer again */
	vif = iwl_mvm_get_bss_vif(mvm);
	if (IS_ERR_OR_NULL(vif))
		goto err;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	if (iwl_mvm_check_rt_status(mvm, vif)) {
		set_bit(STATUS_FW_ERROR, &mvm->trans->status);
		iwl_mvm_dump_nic_error_log(mvm);
		iwl_dbg_tlv_time_point(&mvm->fwrt,
				       IWL_FW_INI_TIME_POINT_FW_ASSERT, NULL);
		iwl_fw_dbg_collect_desc(&mvm->fwrt, &iwl_dump_desc_assert,
					false, 0);
		ret = 1;
		goto err;
	}

	ret = iwl_trans_d3_resume(mvm->trans, &d3_status, test, !unified_image);
	if (ret)
		goto err;

	if (d3_status != IWL_D3_STATUS_ALIVE) {
		IWL_INFO(mvm, "Device was reset during suspend\n");
		goto err;
	}

	if (d0i3_first) {
		struct iwl_host_cmd cmd = {
			.id = D0I3_END_CMD,
			.flags = CMD_WANT_SKB | CMD_SEND_IN_D3,
		};
		int len;

		ret = iwl_mvm_send_cmd(mvm, &cmd);
		if (ret < 0) {
			IWL_ERR(mvm, "Failed to send D0I3_END_CMD first (%d)\n",
				ret);
			goto err;
		}
		switch (mvm->cmd_ver.d0i3_resp) {
		case 0:
			break;
		case 1:
			len = iwl_rx_packet_payload_len(cmd.resp_pkt);
			if (len != sizeof(u32)) {
				IWL_ERR(mvm,
					"Error with D0I3_END_CMD response size (%d)\n",
					len);
				goto err;
			}
			if (IWL_D0I3_RESET_REQUIRE &
			    le32_to_cpu(*(__le32 *)cmd.resp_pkt->data)) {
				iwl_write32(mvm->trans, CSR_RESET,
					    CSR_RESET_REG_FLAG_FORCE_NMI);
				iwl_free_resp(&cmd);
			}
			break;
		default:
			WARN_ON(1);
		}
	}

	/* after the successful handshake, we're out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	/*
	 * Query the current location and source from the D3 firmware so we
	 * can play it back when we re-intiailize the D0 firmware
	 */
	iwl_mvm_update_changed_regdom(mvm);

	/* Re-configure PPAG settings */
	iwl_mvm_ppag_send_cmd(mvm);

	if (!unified_image)
		/*  Re-configure default SAR profile */
		iwl_mvm_sar_select_profile(mvm, 1, 1);

	if (mvm->net_detect) {
		/* If this is a non-unified image, we restart the FW,
		 * so no need to stop the netdetect scan.  If that
		 * fails, continue and try to get the wake-up reasons,
		 * but trigger a HW restart by keeping a failure code
		 * in ret.
		 */
		if (unified_image)
			ret = iwl_mvm_scan_stop(mvm, IWL_MVM_SCAN_NETDETECT,
						false);

		iwl_mvm_query_netdetect_reasons(mvm, vif);
		/* has unlocked the mutex, so skip that */
		goto out;
	} else {
		keep = iwl_mvm_query_wakeup_reasons(mvm, vif);
#ifdef CONFIG_IWLWIFI_DEBUGFS
		if (keep)
			mvm->keep_vif = vif;
#endif
		/* has unlocked the mutex, so skip that */
		goto out_iterate;
	}

err:
	iwl_mvm_free_nd(mvm);
	mutex_unlock(&mvm->mutex);

out_iterate:
	if (!test)
		ieee80211_iterate_active_interfaces_mtx(mvm->hw,
			IEEE80211_IFACE_ITER_NORMAL,
			iwl_mvm_d3_disconnect_iter, keep ? vif : NULL);

out:
	clear_bit(IWL_MVM_STATUS_IN_D3, &mvm->status);

	/* no need to reset the device in unified images, if successful */
	if (unified_image && !ret) {
		/* nothing else to do if we already sent D0I3_END_CMD */
		if (d0i3_first)
			return 0;

		ret = iwl_mvm_send_cmd_pdu(mvm, D0I3_END_CMD, 0, 0, NULL);
		if (!ret)
			return 0;
	}

	/*
	 * Reconfigure the device in one of the following cases:
	 * 1. We are not using a unified image
	 * 2. We are using a unified image but had an error while exiting D3
	 */
	set_bit(IWL_MVM_STATUS_HW_RESTART_REQUESTED, &mvm->status);

	/* regardless of what happened, we're now out of D3 */
	mvm->trans->system_pm_mode = IWL_PLAT_PM_MODE_DISABLED;

	return 1;
}

int iwl_mvm_resume(struct ieee80211_hw *hw)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int ret;

	ret = __iwl_mvm_resume(mvm, false);

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	return ret;
}

void iwl_mvm_set_wakeup(struct ieee80211_hw *hw, bool enabled)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);

	device_set_wakeup_enable(mvm->trans->dev, enabled);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
static int iwl_mvm_d3_test_open(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	int err;

	if (mvm->d3_test_active)
		return -EBUSY;

	file->private_data = inode->i_private;

	iwl_mvm_pause_tcm(mvm, true);

	iwl_fw_runtime_suspend(&mvm->fwrt);

	/* start pseudo D3 */
	rtnl_lock();
	err = __iwl_mvm_suspend(mvm->hw, mvm->hw->wiphy->wowlan_config, true);
	rtnl_unlock();
	if (err > 0)
		err = -EINVAL;
	if (err)
		return err;

	mvm->d3_test_active = true;
	mvm->keep_vif = NULL;
	return 0;
}

static ssize_t iwl_mvm_d3_test_read(struct file *file, char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct iwl_mvm *mvm = file->private_data;
	u32 pme_asserted;

	while (true) {
		/* read pme_ptr if available */
		if (mvm->d3_test_pme_ptr) {
			pme_asserted = iwl_trans_read_mem32(mvm->trans,
						mvm->d3_test_pme_ptr);
			if (pme_asserted)
				break;
		}

		if (msleep_interruptible(100))
			break;
	}

	return 0;
}

static void iwl_mvm_d3_test_disconn_work_iter(void *_data, u8 *mac,
					      struct ieee80211_vif *vif)
{
	/* skip the one we keep connection on */
	if (_data == vif)
		return;

	if (vif->type == NL80211_IFTYPE_STATION)
		ieee80211_connection_loss(vif);
}

static int iwl_mvm_d3_test_release(struct inode *inode, struct file *file)
{
	struct iwl_mvm *mvm = inode->i_private;
	bool unified_image = fw_has_capa(&mvm->fw->ucode_capa,
					 IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG);

	mvm->d3_test_active = false;

	iwl_fw_dbg_read_d3_debug_data(&mvm->fwrt);

	rtnl_lock();
	__iwl_mvm_resume(mvm, true);
	rtnl_unlock();

	iwl_mvm_resume_tcm(mvm);

	iwl_fw_runtime_resume(&mvm->fwrt);

	iwl_abort_notification_waits(&mvm->notif_wait);
	if (!unified_image) {
		int remaining_time = 10;

		ieee80211_restart_hw(mvm->hw);

		/* wait for restart and disconnect all interfaces */
		while (test_bit(IWL_MVM_STATUS_IN_HW_RESTART, &mvm->status) &&
		       remaining_time > 0) {
			remaining_time--;
			msleep(1000);
		}

		if (remaining_time == 0)
			IWL_ERR(mvm, "Timed out waiting for HW restart!\n");
	}

	ieee80211_iterate_active_interfaces_atomic(
		mvm->hw, IEEE80211_IFACE_ITER_NORMAL,
		iwl_mvm_d3_test_disconn_work_iter, mvm->keep_vif);

	return 0;
}

const struct file_operations iwl_dbgfs_d3_test_ops = {
	.llseek = no_llseek,
	.open = iwl_mvm_d3_test_open,
	.read = iwl_mvm_d3_test_read,
	.release = iwl_mvm_d3_test_release,
};
#endif
	struct cfg80211_wowlan_wakeup wakeup = {
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_mvm_nd_query_results query;
	struct iwl_wowlan_status *fw_status;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	fw_status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR_OR_NULL(fw_status)) {
		reasons = le32_to_cpu(fw_status->wakeup_reasons);
		kfree(fw_status);
	}