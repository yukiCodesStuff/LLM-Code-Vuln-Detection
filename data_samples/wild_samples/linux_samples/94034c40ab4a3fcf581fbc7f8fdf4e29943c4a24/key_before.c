	    !(new->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE)) {
		memcpy(tkip_old, tk_old, WLAN_KEY_LEN_TKIP);
		memcpy(tkip_new, tk_new, WLAN_KEY_LEN_TKIP);
		memset(tkip_old + NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY, 0, 8);
		memset(tkip_new + NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY, 0, 8);
		tk_old = tkip_old;
		tk_new = tkip_new;
	}

	return !crypto_memneq(tk_old, tk_new, new->conf.keylen);
}

int ieee80211_key_link(struct ieee80211_key *key,
		       struct ieee80211_sub_if_data *sdata,
		       struct sta_info *sta)
{
	struct ieee80211_key *old_key;
	int idx = key->conf.keyidx;
	bool pairwise = key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE;
	/*
	 * We want to delay tailroom updates only for station - in that
	 * case it helps roaming speed, but in other cases it hurts and
	 * can cause warnings to appear.
	 */
	bool delay_tailroom = sdata->vif.type == NL80211_IFTYPE_STATION;
	int ret = -EOPNOTSUPP;

	mutex_lock(&sdata->local->key_mtx);

	if (sta && pairwise) {
		struct ieee80211_key *alt_key;

		old_key = key_mtx_dereference(sdata->local, sta->ptk[idx]);
		alt_key = key_mtx_dereference(sdata->local, sta->ptk[idx ^ 1]);

		/* The rekey code assumes that the old and new key are using
		 * the same cipher. Enforce the assumption for pairwise keys.
		 */
		if ((alt_key && alt_key->conf.cipher != key->conf.cipher) ||
		    (old_key && old_key->conf.cipher != key->conf.cipher))
			goto out;
	} else if (sta) {
		old_key = key_mtx_dereference(sdata->local, sta->gtk[idx]);
	} else {
		old_key = key_mtx_dereference(sdata->local, sdata->keys[idx]);
	}

	/* Non-pairwise keys must also not switch the cipher on rekey */
	if (!pairwise) {
		if (old_key && old_key->conf.cipher != key->conf.cipher)
			goto out;
	}

	/*
	 * Silently accept key re-installation without really installing the
	 * new version of the key to avoid nonce reuse or replay issues.
	 */
	if (ieee80211_key_identical(sdata, old_key, key)) {
		ieee80211_key_free_unused(key);
		ret = 0;
		goto out;
	}

	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	increment_tailroom_need_count(sdata);

	ret = ieee80211_key_replace(sdata, sta, pairwise, old_key, key);

	if (!ret) {
		ieee80211_debugfs_key_add(key);
		ieee80211_key_destroy(old_key, delay_tailroom);
	} else {
		ieee80211_key_free(key, delay_tailroom);
	}

 out:
	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}

void ieee80211_key_free(struct ieee80211_key *key, bool delay_tailroom)
{
	if (!key)
		return;

	/*
	 * Replace key with nothingness if it was ever used.
	 */
	if (key->sdata)
		ieee80211_key_replace(key->sdata, key->sta,
				key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE,
				key, NULL);
	ieee80211_key_destroy(key, delay_tailroom);
}

void ieee80211_reenable_keys(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_key *key;
	struct ieee80211_sub_if_data *vlan;

	lockdep_assert_wiphy(sdata->local->hw.wiphy);

	mutex_lock(&sdata->local->key_mtx);

	sdata->crypto_tx_tailroom_needed_cnt = 0;
	sdata->crypto_tx_tailroom_pending_dec = 0;

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		list_for_each_entry(vlan, &sdata->u.ap.vlans, u.vlan.list) {
			vlan->crypto_tx_tailroom_needed_cnt = 0;
			vlan->crypto_tx_tailroom_pending_dec = 0;
		}
	}

	if (ieee80211_sdata_running(sdata)) {
		list_for_each_entry(key, &sdata->key_list, list) {
			increment_tailroom_need_count(sdata);
			ieee80211_key_enable_hw_accel(key);
		}
	}

	mutex_unlock(&sdata->local->key_mtx);
}

void ieee80211_iter_keys(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif,
			 void (*iter)(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ieee80211_key_conf *key,
				      void *data),
			 void *iter_data)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_key *key, *tmp;
	struct ieee80211_sub_if_data *sdata;

	lockdep_assert_wiphy(hw->wiphy);

	mutex_lock(&local->key_mtx);
	if (vif) {
		sdata = vif_to_sdata(vif);
		list_for_each_entry_safe(key, tmp, &sdata->key_list, list)
			iter(hw, &sdata->vif,
			     key->sta ? &key->sta->sta : NULL,
			     &key->conf, iter_data);
	} else {
		list_for_each_entry(sdata, &local->interfaces, list)
			list_for_each_entry_safe(key, tmp,
						 &sdata->key_list, list)
				iter(hw, &sdata->vif,
				     key->sta ? &key->sta->sta : NULL,
				     &key->conf, iter_data);
	}
	mutex_unlock(&local->key_mtx);
}
EXPORT_SYMBOL(ieee80211_iter_keys);

static void
_ieee80211_iter_keys_rcu(struct ieee80211_hw *hw,
			 struct ieee80211_sub_if_data *sdata,
			 void (*iter)(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ieee80211_key_conf *key,
				      void *data),
			 void *iter_data)
{
	struct ieee80211_key *key;

	list_for_each_entry_rcu(key, &sdata->key_list, list) {
		/* skip keys of station in removal process */
		if (key->sta && key->sta->removed)
			continue;
		if (!(key->flags & KEY_FLAG_UPLOADED_TO_HARDWARE))
			continue;

		iter(hw, &sdata->vif,
		     key->sta ? &key->sta->sta : NULL,
		     &key->conf, iter_data);
	}
}

void ieee80211_iter_keys_rcu(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif,
			     void (*iter)(struct ieee80211_hw *hw,
					  struct ieee80211_vif *vif,
					  struct ieee80211_sta *sta,
					  struct ieee80211_key_conf *key,
					  void *data),
			     void *iter_data)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_sub_if_data *sdata;

	if (vif) {
		sdata = vif_to_sdata(vif);
		_ieee80211_iter_keys_rcu(hw, sdata, iter, iter_data);
	} else {
		list_for_each_entry_rcu(sdata, &local->interfaces, list)
			_ieee80211_iter_keys_rcu(hw, sdata, iter, iter_data);
	}
}
EXPORT_SYMBOL(ieee80211_iter_keys_rcu);

static void ieee80211_free_keys_iface(struct ieee80211_sub_if_data *sdata,
				      struct list_head *keys)
{
	struct ieee80211_key *key, *tmp;

	decrease_tailroom_need_count(sdata,
				     sdata->crypto_tx_tailroom_pending_dec);
	sdata->crypto_tx_tailroom_pending_dec = 0;

	ieee80211_debugfs_key_remove_mgmt_default(sdata);
	ieee80211_debugfs_key_remove_beacon_default(sdata);

	list_for_each_entry_safe(key, tmp, &sdata->key_list, list) {
		ieee80211_key_replace(key->sdata, key->sta,
				key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE,
				key, NULL);
		list_add_tail(&key->list, keys);
	}

	ieee80211_debugfs_key_update_default(sdata);
}

void ieee80211_free_keys(struct ieee80211_sub_if_data *sdata,
			 bool force_synchronize)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *vlan;
	struct ieee80211_sub_if_data *master;
	struct ieee80211_key *key, *tmp;
	LIST_HEAD(keys);

	cancel_delayed_work_sync(&sdata->dec_tailroom_needed_wk);

	mutex_lock(&local->key_mtx);

	ieee80211_free_keys_iface(sdata, &keys);

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		list_for_each_entry(vlan, &sdata->u.ap.vlans, u.vlan.list)
			ieee80211_free_keys_iface(vlan, &keys);
	}

	if (!list_empty(&keys) || force_synchronize)
		synchronize_net();
	list_for_each_entry_safe(key, tmp, &keys, list)
		__ieee80211_key_destroy(key, false);

	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN) {
		if (sdata->bss) {
			master = container_of(sdata->bss,
					      struct ieee80211_sub_if_data,
					      u.ap);

			WARN_ON_ONCE(sdata->crypto_tx_tailroom_needed_cnt !=
				     master->crypto_tx_tailroom_needed_cnt);
		}
	} else {
		WARN_ON_ONCE(sdata->crypto_tx_tailroom_needed_cnt ||
			     sdata->crypto_tx_tailroom_pending_dec);
	}

	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		list_for_each_entry(vlan, &sdata->u.ap.vlans, u.vlan.list)
			WARN_ON_ONCE(vlan->crypto_tx_tailroom_needed_cnt ||
				     vlan->crypto_tx_tailroom_pending_dec);
	}

	mutex_unlock(&local->key_mtx);
}

void ieee80211_free_sta_keys(struct ieee80211_local *local,
			     struct sta_info *sta)
{
	struct ieee80211_key *key;
	int i;

	mutex_lock(&local->key_mtx);
	for (i = 0; i < ARRAY_SIZE(sta->gtk); i++) {
		key = key_mtx_dereference(local, sta->gtk[i]);
		if (!key)
			continue;
		ieee80211_key_replace(key->sdata, key->sta,
				key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE,
				key, NULL);
		__ieee80211_key_destroy(key, key->sdata->vif.type ==
					NL80211_IFTYPE_STATION);
	}

	for (i = 0; i < NUM_DEFAULT_KEYS; i++) {
		key = key_mtx_dereference(local, sta->ptk[i]);
		if (!key)
			continue;
		ieee80211_key_replace(key->sdata, key->sta,
				key->conf.flags & IEEE80211_KEY_FLAG_PAIRWISE,
				key, NULL);
		__ieee80211_key_destroy(key, key->sdata->vif.type ==
					NL80211_IFTYPE_STATION);
	}

	mutex_unlock(&local->key_mtx);
}

void ieee80211_delayed_tailroom_dec(struct work_struct *wk)
{
	struct ieee80211_sub_if_data *sdata;

	sdata = container_of(wk, struct ieee80211_sub_if_data,
			     dec_tailroom_needed_wk.work);

	/*
	 * The reason for the delayed tailroom needed decrementing is to
	 * make roaming faster: during roaming, all keys are first deleted
	 * and then new keys are installed. The first new key causes the
	 * crypto_tx_tailroom_needed_cnt to go from 0 to 1, which invokes
	 * the cost of synchronize_net() (which can be slow). Avoid this
	 * by deferring the crypto_tx_tailroom_needed_cnt decrementing on
	 * key removal for a while, so if we roam the value is larger than
	 * zero and no 0->1 transition happens.
	 *
	 * The cost is that if the AP switching was from an AP with keys
	 * to one without, we still allocate tailroom while it would no
	 * longer be needed. However, in the typical (fast) roaming case
	 * within an ESS this usually won't happen.
	 */

	mutex_lock(&sdata->local->key_mtx);
	decrease_tailroom_need_count(sdata,
				     sdata->crypto_tx_tailroom_pending_dec);
	sdata->crypto_tx_tailroom_pending_dec = 0;
	mutex_unlock(&sdata->local->key_mtx);
}

void ieee80211_gtk_rekey_notify(struct ieee80211_vif *vif, const u8 *bssid,
				const u8 *replay_ctr, gfp_t gfp)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);

	trace_api_gtk_rekey_notify(sdata, bssid, replay_ctr);

	cfg80211_gtk_rekey_notify(sdata->dev, bssid, replay_ctr, gfp);
}
EXPORT_SYMBOL_GPL(ieee80211_gtk_rekey_notify);

void ieee80211_get_key_rx_seq(struct ieee80211_key_conf *keyconf,
			      int tid, struct ieee80211_key_seq *seq)
{
	struct ieee80211_key *key;
	const u8 *pn;

	key = container_of(keyconf, struct ieee80211_key, conf);

	switch (key->conf.cipher) {
	case WLAN_CIPHER_SUITE_TKIP:
		if (WARN_ON(tid < 0 || tid >= IEEE80211_NUM_TIDS))
			return;
		seq->tkip.iv32 = key->u.tkip.rx[tid].iv32;
		seq->tkip.iv16 = key->u.tkip.rx[tid].iv16;
		break;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_CCMP_256:
		if (WARN_ON(tid < -1 || tid >= IEEE80211_NUM_TIDS))
			return;
		if (tid < 0)
			pn = key->u.ccmp.rx_pn[IEEE80211_NUM_TIDS];
		else
			pn = key->u.ccmp.rx_pn[tid];
		memcpy(seq->ccmp.pn, pn, IEEE80211_CCMP_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
	case WLAN_CIPHER_SUITE_BIP_CMAC_256:
		if (WARN_ON(tid != 0))
			return;
		pn = key->u.aes_cmac.rx_pn;
		memcpy(seq->aes_cmac.pn, pn, IEEE80211_CMAC_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		if (WARN_ON(tid != 0))
			return;
		pn = key->u.aes_gmac.rx_pn;
		memcpy(seq->aes_gmac.pn, pn, IEEE80211_GMAC_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		if (WARN_ON(tid < -1 || tid >= IEEE80211_NUM_TIDS))
			return;
		if (tid < 0)
			pn = key->u.gcmp.rx_pn[IEEE80211_NUM_TIDS];
		else
			pn = key->u.gcmp.rx_pn[tid];
		memcpy(seq->gcmp.pn, pn, IEEE80211_GCMP_PN_LEN);
		break;
	}
}
EXPORT_SYMBOL(ieee80211_get_key_rx_seq);

void ieee80211_set_key_rx_seq(struct ieee80211_key_conf *keyconf,
			      int tid, struct ieee80211_key_seq *seq)
{
	struct ieee80211_key *key;
	u8 *pn;

	key = container_of(keyconf, struct ieee80211_key, conf);

	switch (key->conf.cipher) {
	case WLAN_CIPHER_SUITE_TKIP:
		if (WARN_ON(tid < 0 || tid >= IEEE80211_NUM_TIDS))
			return;
		key->u.tkip.rx[tid].iv32 = seq->tkip.iv32;
		key->u.tkip.rx[tid].iv16 = seq->tkip.iv16;
		break;
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_CCMP_256:
		if (WARN_ON(tid < -1 || tid >= IEEE80211_NUM_TIDS))
			return;
		if (tid < 0)
			pn = key->u.ccmp.rx_pn[IEEE80211_NUM_TIDS];
		else
			pn = key->u.ccmp.rx_pn[tid];
		memcpy(pn, seq->ccmp.pn, IEEE80211_CCMP_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
	case WLAN_CIPHER_SUITE_BIP_CMAC_256:
		if (WARN_ON(tid != 0))
			return;
		pn = key->u.aes_cmac.rx_pn;
		memcpy(pn, seq->aes_cmac.pn, IEEE80211_CMAC_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		if (WARN_ON(tid != 0))
			return;
		pn = key->u.aes_gmac.rx_pn;
		memcpy(pn, seq->aes_gmac.pn, IEEE80211_GMAC_PN_LEN);
		break;
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		if (WARN_ON(tid < -1 || tid >= IEEE80211_NUM_TIDS))
			return;
		if (tid < 0)
			pn = key->u.gcmp.rx_pn[IEEE80211_NUM_TIDS];
		else
			pn = key->u.gcmp.rx_pn[tid];
		memcpy(pn, seq->gcmp.pn, IEEE80211_GCMP_PN_LEN);
		break;
	default:
		WARN_ON(1);
		break;
	}
}
EXPORT_SYMBOL_GPL(ieee80211_set_key_rx_seq);

void ieee80211_remove_key(struct ieee80211_key_conf *keyconf)
{
	struct ieee80211_key *key;

	key = container_of(keyconf, struct ieee80211_key, conf);

	assert_key_lock(key->local);

	/*
	 * if key was uploaded, we assume the driver will/has remove(d)
	 * it, so adjust bookkeeping accordingly
	 */
	if (key->flags & KEY_FLAG_UPLOADED_TO_HARDWARE) {
		key->flags &= ~KEY_FLAG_UPLOADED_TO_HARDWARE;

		if (!(key->conf.flags & (IEEE80211_KEY_FLAG_GENERATE_MMIC |
					 IEEE80211_KEY_FLAG_PUT_MIC_SPACE |
					 IEEE80211_KEY_FLAG_RESERVE_TAILROOM)))
			increment_tailroom_need_count(key->sdata);
	}

	ieee80211_key_free(key, false);
}
EXPORT_SYMBOL_GPL(ieee80211_remove_key);

struct ieee80211_key_conf *
ieee80211_gtk_rekey_add(struct ieee80211_vif *vif,
			struct ieee80211_key_conf *keyconf)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_key *key;
	int err;

	if (WARN_ON(!local->wowlan))
		return ERR_PTR(-EINVAL);

	if (WARN_ON(vif->type != NL80211_IFTYPE_STATION))
		return ERR_PTR(-EINVAL);

	key = ieee80211_key_alloc(keyconf->cipher, keyconf->keyidx,
				  keyconf->keylen, keyconf->key,
				  0, NULL, NULL);
	if (IS_ERR(key))
		return ERR_CAST(key);

	if (sdata->u.mgd.mfp != IEEE80211_MFP_DISABLED)
		key->conf.flags |= IEEE80211_KEY_FLAG_RX_MGMT;

	err = ieee80211_key_link(key, sdata, NULL);
	if (err)
		return ERR_PTR(err);

	return &key->conf;
}
EXPORT_SYMBOL_GPL(ieee80211_gtk_rekey_add);

void ieee80211_key_mic_failure(struct ieee80211_key_conf *keyconf)
{
	struct ieee80211_key *key;

	key = container_of(keyconf, struct ieee80211_key, conf);

	switch (key->conf.cipher) {
	case WLAN_CIPHER_SUITE_AES_CMAC:
	case WLAN_CIPHER_SUITE_BIP_CMAC_256:
		key->u.aes_cmac.icverrors++;
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		key->u.aes_gmac.icverrors++;
		break;
	default:
		/* ignore the others for now, we don't keep counters now */
		break;
	}
}
EXPORT_SYMBOL_GPL(ieee80211_key_mic_failure);

void ieee80211_key_replay(struct ieee80211_key_conf *keyconf)
{
	struct ieee80211_key *key;

	key = container_of(keyconf, struct ieee80211_key, conf);

	switch (key->conf.cipher) {
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_CCMP_256:
		key->u.ccmp.replays++;
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
	case WLAN_CIPHER_SUITE_BIP_CMAC_256:
		key->u.aes_cmac.replays++;
		break;
	case WLAN_CIPHER_SUITE_BIP_GMAC_128:
	case WLAN_CIPHER_SUITE_BIP_GMAC_256:
		key->u.aes_gmac.replays++;
		break;
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
		key->u.gcmp.replays++;
		break;
	}
}
EXPORT_SYMBOL_GPL(ieee80211_key_replay);
	if (ieee80211_key_identical(sdata, old_key, key)) {
		ieee80211_key_free_unused(key);
		ret = 0;
		goto out;
	}

	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	increment_tailroom_need_count(sdata);

	ret = ieee80211_key_replace(sdata, sta, pairwise, old_key, key);

	if (!ret) {
		ieee80211_debugfs_key_add(key);
		ieee80211_key_destroy(old_key, delay_tailroom);
	} else {
		ieee80211_key_free(key, delay_tailroom);
	}

 out:
	mutex_unlock(&sdata->local->key_mtx);

	return ret;
}