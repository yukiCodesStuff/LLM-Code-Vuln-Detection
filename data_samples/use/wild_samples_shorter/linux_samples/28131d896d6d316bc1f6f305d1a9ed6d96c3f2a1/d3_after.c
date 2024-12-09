
/* converted data from the different status responses */
struct iwl_wowlan_status_data {
	u64 replay_ctr;
	u32 num_of_gtk_rekeys;
	u32 received_beacons;
	u32 wakeup_reasons;
	u32 wake_packet_length;
	u32 wake_packet_bufsize;
	u16 pattern_number;
	u16 non_qos_seq_ctr;
	u16 qos_seq_ctr[8];
	u8 tid_tear_down;

	struct {
		/*
		 * We store both the TKIP and AES representations
		 * coming from the firmware because we decode the
		 * data from there before we iterate the keys and
		 * know which one we need.
		 */
		struct {
			struct ieee80211_key_seq seq[IWL_MAX_TID_COUNT];
		} tkip, aes;
		/* including RX MIC key for TKIP */
		u8 key[WOWLAN_KEY_MAX_SIZE];
		u8 len;
		u8 flags;
	} gtk;

	struct {
		/* Same as above */
		struct {
			struct ieee80211_key_seq seq[IWL_MAX_TID_COUNT];
			u64 tx_pn;
		} tkip, aes;
	} ptk;

	struct {
		u64 ipn;
		u8 key[WOWLAN_KEY_MAX_SIZE];
		u8 len;
		u8 flags;
	} igtk;

	u8 wake_packet[];
};

static void iwl_mvm_report_wakeup_reasons(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
	seq->tkip.iv16 = le16_to_cpu(sc->iv16);
}

static void iwl_mvm_set_key_rx_seq_tids(struct ieee80211_key_conf *key,
					struct ieee80211_key_seq *seq)
{
	int tid;

	for (tid = 0; tid < IWL_MAX_TID_COUNT; tid++)
		ieee80211_set_key_rx_seq(key, tid, &seq[tid]);
}

static void iwl_mvm_set_aes_ptk_rx_seq(struct iwl_mvm *mvm,
				       struct iwl_wowlan_status_data *status,
				       struct ieee80211_sta *sta,
				       struct ieee80211_key_conf *key)
{
	struct iwl_mvm_sta *mvmsta = iwl_mvm_sta_from_mac80211(sta);
	struct iwl_mvm_key_pn *ptk_pn;
	int tid;

	iwl_mvm_set_key_rx_seq_tids(key, status->ptk.aes.seq);

	if (!iwl_mvm_has_new_rx_api(mvm))
		return;


	rcu_read_lock();
	ptk_pn = rcu_dereference(mvmsta->ptk_pn[key->keyidx]);
	if (WARN_ON(!ptk_pn)) {
		rcu_read_unlock();
		return;
	}

	for (tid = 0; tid < IWL_MAX_TID_COUNT; tid++) {
		int i;

		for (i = 1; i < mvm->trans->num_rx_queues; i++)
			memcpy(ptk_pn->q[i].pn[tid],
			       status->ptk.aes.seq[tid].ccmp.pn,
			       IEEE80211_CCMP_PN_LEN);
	}
	rcu_read_unlock();
}

static void iwl_mvm_convert_key_counters(struct iwl_wowlan_status_data *status,
					 union iwl_all_tsc_rsc *sc)
{
	int i;

	BUILD_BUG_ON(IWL_MAX_TID_COUNT > IWL_MAX_TID_COUNT);
	BUILD_BUG_ON(IWL_MAX_TID_COUNT > IWL_NUM_RSC);

	/* GTK RX counters */
	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		iwl_mvm_tkip_sc_to_seq(&sc->tkip.multicast_rsc[i],
				       &status->gtk.tkip.seq[i]);
		iwl_mvm_aes_sc_to_seq(&sc->aes.multicast_rsc[i],
				      &status->gtk.aes.seq[i]);
	}

	/* PTK TX counter */
	status->ptk.tkip.tx_pn = (u64)le16_to_cpu(sc->tkip.tsc.iv16) |
				 ((u64)le32_to_cpu(sc->tkip.tsc.iv32) << 16);
	status->ptk.aes.tx_pn = le64_to_cpu(sc->aes.tsc.pn);

	/* PTK RX counters */
	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		iwl_mvm_tkip_sc_to_seq(&sc->tkip.unicast_rsc[i],
				       &status->ptk.tkip.seq[i]);
		iwl_mvm_aes_sc_to_seq(&sc->aes.unicast_rsc[i],
				      &status->ptk.aes.seq[i]);
	}
}

static void iwl_mvm_set_key_rx_seq(struct iwl_mvm *mvm,
				   struct ieee80211_key_conf *key,
				   struct iwl_wowlan_status_data *status)
{
	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		iwl_mvm_set_key_rx_seq_tids(key, status->gtk.aes.seq);
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		iwl_mvm_set_key_rx_seq_tids(key, status->gtk.tkip.seq);
		break;
	default:
		WARN_ON(1);
	}

struct iwl_mvm_d3_gtk_iter_data {
	struct iwl_mvm *mvm;
	struct iwl_wowlan_status_data *status;
	void *last_gtk;
	u32 cipher;
	bool find_phase, unhandled_cipher;
	int num_keys;
				   void *_data)
{
	struct iwl_mvm_d3_gtk_iter_data *data = _data;
	struct iwl_wowlan_status_data *status = data->status;

	if (data->unhandled_cipher)
		return;

	 * note that this assumes no TDLS sessions are active
	 */
	if (sta) {
		if (data->find_phase)
			return;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			atomic64_set(&key->tx_pn, status->ptk.aes.tx_pn);
			iwl_mvm_set_aes_ptk_rx_seq(data->mvm, status, sta, key);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			atomic64_set(&key->tx_pn, status->ptk.tkip.tx_pn);
			iwl_mvm_set_key_rx_seq_tids(key, status->ptk.tkip.seq);
			break;
		}

		/* that's it for this key */

static bool iwl_mvm_setup_connection_keep(struct iwl_mvm *mvm,
					  struct ieee80211_vif *vif,
					  struct iwl_wowlan_status_data *status)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_mvm_d3_gtk_iter_data gtkdata = {
		.mvm = mvm,
	if (!status || !vif->bss_conf.bssid)
		return false;

	if (status->wakeup_reasons & disconnection_reasons)
		return false;

	/* find last GTK that we used initially, if any */
	gtkdata.find_phase = true;
			    iwl_mvm_d3_update_keys, &gtkdata);

	IWL_DEBUG_WOWLAN(mvm, "num of GTK rekeying %d\n",
			 status->num_of_gtk_rekeys);
	if (status->num_of_gtk_rekeys) {
		struct ieee80211_key_conf *key;
		struct {
			struct ieee80211_key_conf conf;
		} conf = {
			.conf.cipher = gtkdata.cipher,
			.conf.keyidx =
				status->gtk.flags & IWL_WOWLAN_GTK_IDX_MASK,
		};
		__be64 replay_ctr;

		IWL_DEBUG_WOWLAN(mvm,
				 "Received from FW GTK cipher %d, key index %d\n",
				 conf.conf.cipher, conf.conf.keyidx);

		BUILD_BUG_ON(WLAN_KEY_LEN_CCMP != WLAN_KEY_LEN_GCMP);
		BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_CCMP);
		BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_GCMP_256);
		BUILD_BUG_ON(sizeof(conf.key) < WLAN_KEY_LEN_TKIP);
		BUILD_BUG_ON(sizeof(conf.key) < sizeof(status->gtk.key));

		memcpy(conf.conf.key, status->gtk.key, sizeof(status->gtk.key));

		switch (gtkdata.cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_GCMP:
			conf.conf.keylen = WLAN_KEY_LEN_CCMP;
			break;
		case WLAN_CIPHER_SUITE_GCMP_256:
			conf.conf.keylen = WLAN_KEY_LEN_GCMP_256;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			conf.conf.keylen = WLAN_KEY_LEN_TKIP;
			break;
		}

		key = ieee80211_gtk_rekey_add(vif, &conf.conf);
			return false;
		iwl_mvm_set_key_rx_seq(mvm, key, status);

		replay_ctr = cpu_to_be64(status->replay_ctr);

		ieee80211_gtk_rekey_notify(vif, vif->bss_conf.bssid,
					   (void *)&replay_ctr, GFP_KERNEL);
	}
				    WOWLAN_GET_STATUSES, 0) < 10) {
		mvmvif->seqno_valid = true;
		/* +0x10 because the set API expects next-to-use, not last-used */
		mvmvif->seqno = status->non_qos_seq_ctr + 0x10;
	}

	return true;
}

/* Occasionally, templates would be nice. This is one of those times ... */
#define iwl_mvm_parse_wowlan_status_common(_ver)			\
static struct iwl_wowlan_status_data *					\
iwl_mvm_parse_wowlan_status_common_ ## _ver(struct iwl_mvm *mvm,	\
					    struct iwl_wowlan_status_ ##_ver *data,\
					    int len)			\
{									\
	struct iwl_wowlan_status_data *status;				\
	int data_size, i;						\
									\
	if (len < sizeof(*data)) {					\
		IWL_ERR(mvm, "Invalid WoWLAN status response!\n");	\
		return ERR_PTR(-EIO);					\
		return ERR_PTR(-ENOMEM);				\
									\
	/* copy all the common fields */				\
	status->replay_ctr = le64_to_cpu(data->replay_ctr);		\
	status->pattern_number = le16_to_cpu(data->pattern_number);	\
	status->non_qos_seq_ctr = le16_to_cpu(data->non_qos_seq_ctr);	\
	for (i = 0; i < 8; i++)						\
		status->qos_seq_ctr[i] =				\
			le16_to_cpu(data->qos_seq_ctr[i]);		\
	status->wakeup_reasons = le32_to_cpu(data->wakeup_reasons);	\
	status->num_of_gtk_rekeys =					\
		le32_to_cpu(data->num_of_gtk_rekeys);			\
	status->received_beacons = le32_to_cpu(data->received_beacons);	\
	status->wake_packet_length =					\
		le32_to_cpu(data->wake_packet_length);			\
	status->wake_packet_bufsize =					\
		le32_to_cpu(data->wake_packet_bufsize);			\
	memcpy(status->wake_packet, data->wake_packet,			\
	       status->wake_packet_bufsize);				\
									\
	return status;							\
}

iwl_mvm_parse_wowlan_status_common(v7)
iwl_mvm_parse_wowlan_status_common(v9)

static void iwl_mvm_convert_gtk(struct iwl_wowlan_status_data *status,
				struct iwl_wowlan_gtk_status *data)
{
	BUILD_BUG_ON(sizeof(status->gtk.key) < sizeof(data->key));
	BUILD_BUG_ON(NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY +
		     sizeof(data->tkip_mic_key) >
		     sizeof(status->gtk.key));

	status->gtk.len = data->key_len;
	status->gtk.flags = data->key_flags;

	memcpy(status->gtk.key, data->key, sizeof(data->key));

	/* if it's as long as the TKIP encryption key, copy MIC key */
	if (status->gtk.len == NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY)
		memcpy(status->gtk.key + NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
		       data->tkip_mic_key, sizeof(data->tkip_mic_key));
}

static void iwl_mvm_convert_igtk(struct iwl_wowlan_status_data *status,
				 struct iwl_wowlan_igtk_status *data)
{
	const u8 *ipn = data->ipn;

	BUILD_BUG_ON(sizeof(status->igtk.key) < sizeof(data->key));

	status->igtk.len = data->key_len;
	status->igtk.flags = data->key_flags;

	memcpy(status->igtk.key, data->key, sizeof(data->key));

	status->igtk.ipn = ((u64)ipn[5] <<  0) |
			   ((u64)ipn[4] <<  8) |
			   ((u64)ipn[3] << 16) |
			   ((u64)ipn[2] << 24) |
			   ((u64)ipn[1] << 32) |
			   ((u64)ipn[0] << 40);
}

static struct iwl_wowlan_status_data *
iwl_mvm_send_wowlan_get_status(struct iwl_mvm *mvm, u8 sta_id)
{
	struct iwl_wowlan_status_data *status;
	struct iwl_wowlan_get_status_cmd get_status_cmd = {
		.sta_id = cpu_to_le32(sta_id),
	};
	struct iwl_host_cmd cmd = {
			IWL_UCODE_TLV_API_WOWLAN_KEY_MATERIAL)) {
		struct iwl_wowlan_status_v6 *v6 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v6(mvm, v6, len);
		if (IS_ERR(status))
			goto out_free_resp;

		BUILD_BUG_ON(sizeof(v6->gtk.decrypt_key) >
			     sizeof(status->gtk.key));
		BUILD_BUG_ON(NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY +
			     sizeof(v6->gtk.tkip_mic_key) >
			     sizeof(status->gtk.key));

		/* copy GTK info to the right place */
		memcpy(status->gtk.key, v6->gtk.decrypt_key,
		       sizeof(v6->gtk.decrypt_key));
		memcpy(status->gtk.key + NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY,
		       v6->gtk.tkip_mic_key,
		       sizeof(v6->gtk.tkip_mic_key));

		iwl_mvm_convert_key_counters(status, &v6->gtk.rsc.all_tsc_rsc);

		/* hardcode the key length to 16 since v6 only supports 16 */
		status->gtk.len = 16;

		/*
		 * The key index only uses 2 bits (values 0 to 3) and
		 * we always set bit 7 which means this is the
		 * currently used key.
		 */
		status->gtk.flags = v6->gtk.key_index | BIT(7);
	} else if (notif_ver == 7) {
		struct iwl_wowlan_status_v7 *v7 = (void *)cmd.resp_pkt->data;

		status = iwl_mvm_parse_wowlan_status_common_v7(mvm, v7, len);
		if (IS_ERR(status))
			goto out_free_resp;

		iwl_mvm_convert_key_counters(status, &v7->gtk[0].rsc.all_tsc_rsc);
		iwl_mvm_convert_gtk(status, &v7->gtk[0]);
		iwl_mvm_convert_igtk(status, &v7->igtk[0]);
	} else if (notif_ver == 9 || notif_ver == 10 || notif_ver == 11) {
		struct iwl_wowlan_status_v9 *v9 = (void *)cmd.resp_pkt->data;

		/* these three command versions have same layout and size, the
		 * difference is only in a few not used (reserved) fields.
		 */
		status = iwl_mvm_parse_wowlan_status_common_v9(mvm, v9, len);
		if (IS_ERR(status))
			goto out_free_resp;

		iwl_mvm_convert_key_counters(status, &v9->gtk[0].rsc.all_tsc_rsc);
		iwl_mvm_convert_gtk(status, &v9->gtk[0]);
		iwl_mvm_convert_igtk(status, &v9->igtk[0]);

		status->tid_tear_down = v9->tid_tear_down;
	} else {
		IWL_ERR(mvm,
	return status;
}

static struct iwl_wowlan_status_data *
iwl_mvm_get_wakeup_status(struct iwl_mvm *mvm, u8 sta_id)
{
	u8 cmd_ver = iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP,
					   OFFLOADS_QUERY_CMD,
					 struct ieee80211_vif *vif)
{
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);
	struct iwl_wowlan_status_data *status;
	int i;
	bool keep;
	struct iwl_mvm_sta *mvm_ap_sta;

	status = iwl_mvm_get_wakeup_status(mvm, mvmvif->ap_sta_id);
	if (IS_ERR(status))
		goto out_unlock;

	IWL_DEBUG_WOWLAN(mvm, "wakeup reason 0x%x\n",
			 status->wakeup_reasons);

	/* still at hard-coded place 0 for D3 image */
	mvm_ap_sta = iwl_mvm_sta_from_staid_protected(mvm, 0);
	if (!mvm_ap_sta)
		goto out_free;

	for (i = 0; i < IWL_MAX_TID_COUNT; i++) {
		u16 seq = status->qos_seq_ctr[i];
		/* firmware stores last-used value, we store next value */
		seq += 0x10;
		mvm_ap_sta->tid_data[i].seq_number = seq;
	}
	/* now we have all the data we need, unlock to avoid mac80211 issues */
	mutex_unlock(&mvm->mutex);

	iwl_mvm_report_wakeup_reasons(mvm, vif, status);

	keep = iwl_mvm_setup_connection_keep(mvm, vif, status);

	kfree(status);
	return keep;

out_free:
	kfree(status);
out_unlock:
	mutex_unlock(&mvm->mutex);
	return false;
}
		.pattern_idx = -1,
	};
	struct cfg80211_wowlan_wakeup *wakeup_report = &wakeup;
	struct iwl_wowlan_status_data *status;
	struct iwl_mvm_nd_query_results query;
	unsigned long matched_profiles;
	u32 reasons = 0;
	int i, n_matches, ret;

	status = iwl_mvm_get_wakeup_status(mvm, IWL_MVM_INVALID_STA);
	if (!IS_ERR(status)) {
		reasons = status->wakeup_reasons;
		kfree(status);
	}

	if (reasons & IWL_WOWLAN_WAKEUP_BY_RFKILL_DEASSERTED)
		wakeup.rfkill_release = true;