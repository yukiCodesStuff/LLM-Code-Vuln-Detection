
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

struct iwl_mvm_d3_gtk_iter_data {
	struct iwl_mvm *mvm;
	struct iwl_wowlan_status *status;
	void *last_gtk;
	u32 cipher;
	bool find_phase, unhandled_cipher;
	int num_keys;
				   void *_data)
{
	struct iwl_mvm_d3_gtk_iter_data *data = _data;

	if (data->unhandled_cipher)
		return;

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

static bool iwl_mvm_setup_connection_keep(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status *status)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
	if (!status || !vif->bss_conf.bssid)
		return false;

	if (le32_to_cpu(status->wakeup_reasons) & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 le32_to_cpu(status->num_of_gtk_rekeys));
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
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
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr =
			cpu_to_be64(le64_to_cpu(status->replay_ctr));

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
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
	return status;
}

static struct iwl_wowlan_status *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
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