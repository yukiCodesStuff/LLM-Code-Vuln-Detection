{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct ath11k_peer *peer;
	struct ath11k_sta *arsta;
	const u8 *peer_addr;
	int ret = 0;
	u32 flags = 0;

	/* BIP needs to be done in software */
	if (key->cipher == WLAN_CIPHER_SUITE_AES_CMAC ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_128 ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_256 ||
	    key->cipher == WLAN_CIPHER_SUITE_BIP_CMAC_256)
		return 1;

	if (test_bit(ATH11K_FLAG_HW_CRYPTO_DISABLED, &ar->ab->dev_flags))
		return 1;

	if (key->keyidx > WMI_MAX_KEY_INDEX)
		return -ENOSPC;

	mutex_lock(&ar->conf_mutex);

	if (sta)
		peer_addr = sta->addr;
	else if (arvif->vdev_type == WMI_VDEV_TYPE_STA)
		peer_addr = vif->bss_conf.bssid;
	else
		peer_addr = vif->addr;

	key->hw_key_idx = key->keyidx;

	/* the peer should not disappear in mid-way (unless FW goes awry) since
	 * we already hold conf_mutex. we just make sure its there now.
	 */
	spin_lock_bh(&ab->base_lock);
	peer = ath11k_peer_find(ab, arvif->vdev_id, peer_addr);

	/* flush the fragments cache during key (re)install to
	 * ensure all frags in the new frag list belong to the same key.
	 */
	if (peer && cmd == SET_KEY)
		ath11k_peer_frags_flush(ar, peer);
	spin_unlock_bh(&ab->base_lock);

	if (!peer) {
		if (cmd == SET_KEY) {
			ath11k_warn(ab, "cannot install key for non-existent peer %pM\n",
				    peer_addr);
			ret = -EOPNOTSUPP;
			goto exit;
		} else {
			/* if the peer doesn't exist there is no key to disable
			 * anymore
			 */
			goto exit;
		}
	}

	if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
		flags |= WMI_KEY_PAIRWISE;
	else
		flags |= WMI_KEY_GROUP;

	ret = ath11k_install_key(arvif, key, cmd, peer_addr, flags);
	if (ret) {
		ath11k_warn(ab, "ath11k_install_key failed (%d)\n", ret);
		goto exit;
	}

	ret = ath11k_dp_peer_rx_pn_replay_config(arvif, peer_addr, cmd, key);
	if (ret) {
		ath11k_warn(ab, "failed to offload PN replay detection %d\n", ret);
		goto exit;
	}

	spin_lock_bh(&ab->base_lock);
	peer = ath11k_peer_find(ab, arvif->vdev_id, peer_addr);
	if (peer && cmd == SET_KEY) {
		peer->keys[key->keyidx] = key;
		if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE) {
			peer->ucast_keyidx = key->keyidx;
			peer->sec_type = ath11k_dp_tx_get_encrypt_type(key->cipher);
		} else {
			peer->mcast_keyidx = key->keyidx;
			peer->sec_type_grp = ath11k_dp_tx_get_encrypt_type(key->cipher);
		}
	} else if (peer && cmd == DISABLE_KEY) {
		peer->keys[key->keyidx] = NULL;
		if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
			peer->ucast_keyidx = 0;
		else
			peer->mcast_keyidx = 0;
	} else if (!peer)
		/* impossible unless FW goes crazy */
		ath11k_warn(ab, "peer %pM disappeared!\n", peer_addr);

	if (sta) {
		arsta = (struct ath11k_sta *)sta->drv_priv;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_TKIP:
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_CCMP_256:
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			if (cmd == SET_KEY)
				arsta->pn_type = HAL_PN_TYPE_WPA;
			else
				arsta->pn_type = HAL_PN_TYPE_NONE;
			break;
		default:
			arsta->pn_type = HAL_PN_TYPE_NONE;
			break;
		}
	}

	spin_unlock_bh(&ab->base_lock);

exit:
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static int
ath11k_mac_bitrate_mask_num_vht_rates(struct ath11k *ar,
				      enum nl80211_band band,
				      const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++)
		num_rates += hweight16(mask->control[band].vht_mcs[i]);

	return num_rates;
}

static int
ath11k_mac_set_peer_vht_fixed_rate(struct ath11k_vif *arvif,
				   struct ieee80211_sta *sta,
				   const struct cfg80211_bitrate_mask *mask,
				   enum nl80211_band band)
{
	struct ath11k *ar = arvif->ar;
	u8 vht_rate, nss;
	u32 rate_code;
	int ret, i;

	lockdep_assert_held(&ar->conf_mutex);

	nss = 0;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++) {
		if (hweight16(mask->control[band].vht_mcs[i]) == 1) {
			nss = i + 1;
			vht_rate = ffs(mask->control[band].vht_mcs[i]) - 1;
		}
	}

	if (!nss) {
		ath11k_warn(ar->ab, "No single VHT Fixed rate found to set for %pM",
			    sta->addr);
		return -EINVAL;
	}

	ath11k_dbg(ar->ab, ATH11K_DBG_MAC,
		   "Setting Fixed VHT Rate for peer %pM. Device will not switch to any other selected rates",
		   sta->addr);

	rate_code = ATH11K_HW_RATE_CODE(vht_rate, nss - 1,
					WMI_RATE_PREAMBLE_VHT);
	ret = ath11k_wmi_set_peer_param(ar, sta->addr,
					arvif->vdev_id,
					WMI_PEER_PARAM_FIXED_RATE,
					rate_code);
	if (ret)
		ath11k_warn(ar->ab,
			    "failed to update STA %pM Fixed Rate %d: %d\n",
			     sta->addr, rate_code, ret);

	return ret;
}

static int ath11k_station_assoc(struct ath11k *ar,
				struct ieee80211_vif *vif,
				struct ieee80211_sta *sta,
				bool reassoc)
{
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct peer_assoc_params peer_arg;
	int ret = 0;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	struct cfg80211_bitrate_mask *mask;
	u8 num_vht_rates;

	lockdep_assert_held(&ar->conf_mutex);

	if (WARN_ON(ath11k_mac_vif_chan(vif, &def)))
		return -EPERM;

	band = def.chan->band;
	mask = &arvif->bitrate_mask;

	ath11k_peer_assoc_prepare(ar, vif, sta, &peer_arg, reassoc);

	ret = ath11k_wmi_send_peer_assoc_cmd(ar, &peer_arg);
	if (ret) {
		ath11k_warn(ar->ab, "failed to run peer assoc for STA %pM vdev %i: %d\n",
			    sta->addr, arvif->vdev_id, ret);
		return ret;
	}

	if (!wait_for_completion_timeout(&ar->peer_assoc_done, 1 * HZ)) {
		ath11k_warn(ar->ab, "failed to get peer assoc conf event for %pM vdev %i\n",
			    sta->addr, arvif->vdev_id);
		return -ETIMEDOUT;
	}

	num_vht_rates = ath11k_mac_bitrate_mask_num_vht_rates(ar, band, mask);

	/* If single VHT rate is configured (by set_bitrate_mask()),
	 * peer_assoc will disable VHT. This is now enabled by a peer specific
	 * fixed param.
	 * Note that all other rates and NSS will be disabled for this peer.
	 */
	if (sta->vht_cap.vht_supported && num_vht_rates == 1) {
		ret = ath11k_mac_set_peer_vht_fixed_rate(arvif, sta, mask,
							 band);
		if (ret)
			return ret;
	}

	/* Re-assoc is run only to update supported rates for given station. It
	 * doesn't make much sense to reconfigure the peer completely.
	 */
	if (reassoc)
		return 0;

	ret = ath11k_setup_peer_smps(ar, arvif, sta->addr,
				     &sta->ht_cap);
	if (ret) {
		ath11k_warn(ar->ab, "failed to setup peer SMPS for vdev %d: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}

	if (!sta->wme) {
		arvif->num_legacy_stations++;
		ret = ath11k_recalc_rtscts_prot(arvif);
		if (ret)
			return ret;
	}

	if (sta->wme && sta->uapsd_queues) {
		ret = ath11k_peer_assoc_qos_ap(ar, arvif, sta);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set qos params for STA %pM for vdev %i: %d\n",
				    sta->addr, arvif->vdev_id, ret);
			return ret;
		}
	}

	return 0;
}

static int ath11k_station_disassoc(struct ath11k *ar,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta)
{
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	int ret = 0;

	lockdep_assert_held(&ar->conf_mutex);

	if (!sta->wme) {
		arvif->num_legacy_stations--;
		ret = ath11k_recalc_rtscts_prot(arvif);
		if (ret)
			return ret;
	}

	ret = ath11k_clear_peer_keys(arvif, sta->addr);
	if (ret) {
		ath11k_warn(ar->ab, "failed to clear all peer keys for vdev %i: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}
	return 0;
}

static void ath11k_sta_rc_update_wk(struct work_struct *wk)
{
	struct ath11k *ar;
	struct ath11k_vif *arvif;
	struct ath11k_sta *arsta;
	struct ieee80211_sta *sta;
	struct cfg80211_chan_def def;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	const u16 *vht_mcs_mask;
	u32 changed, bw, nss, smps;
	int err, num_vht_rates;
	const struct cfg80211_bitrate_mask *mask;
	struct peer_assoc_params peer_arg;

	arsta = container_of(wk, struct ath11k_sta, update_wk);
	sta = container_of((void *)arsta, struct ieee80211_sta, drv_priv);
	arvif = arsta->arvif;
	ar = arvif->ar;

	if (WARN_ON(ath11k_mac_vif_chan(arvif->vif, &def)))
		return;

	band = def.chan->band;
	ht_mcs_mask = arvif->bitrate_mask.control[band].ht_mcs;
	vht_mcs_mask = arvif->bitrate_mask.control[band].vht_mcs;

	spin_lock_bh(&ar->data_lock);

	changed = arsta->changed;
	arsta->changed = 0;

	bw = arsta->bw;
	nss = arsta->nss;
	smps = arsta->smps;

	spin_unlock_bh(&ar->data_lock);

	mutex_lock(&ar->conf_mutex);

	nss = max_t(u32, 1, nss);
	nss = min(nss, max(ath11k_mac_max_ht_nss(ht_mcs_mask),
			   ath11k_mac_max_vht_nss(vht_mcs_mask)));

	if (changed & IEEE80211_RC_BW_CHANGED) {
		err = ath11k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
						WMI_PEER_CHWIDTH, bw);
		if (err)
			ath11k_warn(ar->ab, "failed to update STA %pM peer bw %d: %d\n",
				    sta->addr, bw, err);
	}

	if (changed & IEEE80211_RC_NSS_CHANGED) {
		ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "mac update sta %pM nss %d\n",
			   sta->addr, nss);

		err = ath11k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
						WMI_PEER_NSS, nss);
		if (err)
			ath11k_warn(ar->ab, "failed to update STA %pM nss %d: %d\n",
				    sta->addr, nss, err);
	}

	if (changed & IEEE80211_RC_SMPS_CHANGED) {
		ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "mac update sta %pM smps %d\n",
			   sta->addr, smps);

		err = ath11k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
						WMI_PEER_MIMO_PS_STATE, smps);
		if (err)
			ath11k_warn(ar->ab, "failed to update STA %pM smps %d: %d\n",
				    sta->addr, smps, err);
	}

	if (changed & IEEE80211_RC_SUPP_RATES_CHANGED) {
		mask = &arvif->bitrate_mask;
		num_vht_rates = ath11k_mac_bitrate_mask_num_vht_rates(ar, band,
								      mask);

		/* Peer_assoc_prepare will reject vht rates in
		 * bitrate_mask if its not available in range format and
		 * sets vht tx_rateset as unsupported. So multiple VHT MCS
		 * setting(eg. MCS 4,5,6) per peer is not supported here.
		 * But, Single rate in VHT mask can be set as per-peer
		 * fixed rate. But even if any HT rates are configured in
		 * the bitrate mask, device will not switch to those rates
		 * when per-peer Fixed rate is set.
		 * TODO: Check RATEMASK_CMDID to support auto rates selection
		 * across HT/VHT and for multiple VHT MCS support.
		 */
		if (sta->vht_cap.vht_supported && num_vht_rates == 1) {
			ath11k_mac_set_peer_vht_fixed_rate(arvif, sta, mask,
							   band);
		} else {
			/* If the peer is non-VHT or no fixed VHT rate
			 * is provided in the new bitrate mask we set the
			 * other rates using peer_assoc command.
			 */
			ath11k_peer_assoc_prepare(ar, arvif->vif, sta,
						  &peer_arg, true);

			err = ath11k_wmi_send_peer_assoc_cmd(ar, &peer_arg);
			if (err)
				ath11k_warn(ar->ab, "failed to run peer assoc for STA %pM vdev %i: %d\n",
					    sta->addr, arvif->vdev_id, err);

			if (!wait_for_completion_timeout(&ar->peer_assoc_done, 1 * HZ))
				ath11k_warn(ar->ab, "failed to get peer assoc conf event for %pM vdev %i\n",
					    sta->addr, arvif->vdev_id);
		}
	}

	mutex_unlock(&ar->conf_mutex);
}

static int ath11k_mac_inc_num_stations(struct ath11k_vif *arvif,
				       struct ieee80211_sta *sta)
{
	struct ath11k *ar = arvif->ar;

	lockdep_assert_held(&ar->conf_mutex);

	if (arvif->vdev_type == WMI_VDEV_TYPE_STA && !sta->tdls)
		return 0;

	if (ar->num_stations >= ar->max_num_stations)
		return -ENOBUFS;

	ar->num_stations++;

	return 0;
}

static void ath11k_mac_dec_num_stations(struct ath11k_vif *arvif,
					struct ieee80211_sta *sta)
{
	struct ath11k *ar = arvif->ar;

	lockdep_assert_held(&ar->conf_mutex);

	if (arvif->vdev_type == WMI_VDEV_TYPE_STA && !sta->tdls)
		return;

	ar->num_stations--;
}

static int ath11k_mac_station_add(struct ath11k *ar,
				  struct ieee80211_vif *vif,
				  struct ieee80211_sta *sta)
{
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct ath11k_sta *arsta = (struct ath11k_sta *)sta->drv_priv;
	struct peer_create_params peer_param;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ret = ath11k_mac_inc_num_stations(arvif, sta);
	if (ret) {
		ath11k_warn(ab, "refusing to associate station: too many connected already (%d)\n",
			    ar->max_num_stations);
		goto exit;
	}

	arsta->rx_stats = kzalloc(sizeof(*arsta->rx_stats), GFP_KERNEL);
	if (!arsta->rx_stats) {
		ret = -ENOMEM;
		goto dec_num_station;
	}

	peer_param.vdev_id = arvif->vdev_id;
	peer_param.peer_addr = sta->addr;
	peer_param.peer_type = WMI_PEER_TYPE_DEFAULT;

	ret = ath11k_peer_create(ar, arvif, sta, &peer_param);
	if (ret) {
		ath11k_warn(ab, "Failed to add peer: %pM for VDEV: %d\n",
			    sta->addr, arvif->vdev_id);
		goto free_rx_stats;
	}

	ath11k_dbg(ab, ATH11K_DBG_MAC, "Added peer: %pM for VDEV: %d\n",
		   sta->addr, arvif->vdev_id);

	if (ath11k_debugfs_is_extd_tx_stats_enabled(ar)) {
		arsta->tx_stats = kzalloc(sizeof(*arsta->tx_stats), GFP_KERNEL);
		if (!arsta->tx_stats) {
			ret = -ENOMEM;
			goto free_peer;
		}
	}

	if (ieee80211_vif_is_mesh(vif)) {
		ret = ath11k_wmi_set_peer_param(ar, sta->addr,
						arvif->vdev_id,
						WMI_PEER_USE_4ADDR, 1);
		if (ret) {
			ath11k_warn(ab, "failed to STA %pM 4addr capability: %d\n",
				    sta->addr, ret);
			goto free_tx_stats;
		}
	}

	ret = ath11k_dp_peer_setup(ar, arvif->vdev_id, sta->addr);
	if (ret) {
		ath11k_warn(ab, "failed to setup dp for peer %pM on vdev %i (%d)\n",
			    sta->addr, arvif->vdev_id, ret);
		goto free_tx_stats;
	}

	if (ab->hw_params.vdev_start_delay &&
	    !arvif->is_started &&
	    arvif->vdev_type != WMI_VDEV_TYPE_AP) {
		ret = ath11k_start_vdev_delay(ar->hw, vif);
		if (ret) {
			ath11k_warn(ab, "failed to delay vdev start: %d\n", ret);
			goto free_tx_stats;
		}
	}

	return 0;

free_tx_stats:
	kfree(arsta->tx_stats);
	arsta->tx_stats = NULL;
free_peer:
	ath11k_peer_delete(ar, arvif->vdev_id, sta->addr);
free_rx_stats:
	kfree(arsta->rx_stats);
	arsta->rx_stats = NULL;
dec_num_station:
	ath11k_mac_dec_num_stations(arvif, sta);
exit:
	return ret;
}

static int ath11k_mac_op_sta_state(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   enum ieee80211_sta_state old_state,
				   enum ieee80211_sta_state new_state)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct ath11k_sta *arsta = (struct ath11k_sta *)sta->drv_priv;
	struct ath11k_peer *peer;
	int ret = 0;

	/* cancel must be done outside the mutex to avoid deadlock */
	if ((old_state == IEEE80211_STA_NONE &&
	     new_state == IEEE80211_STA_NOTEXIST))
		cancel_work_sync(&arsta->update_wk);

	mutex_lock(&ar->conf_mutex);

	if (old_state == IEEE80211_STA_NOTEXIST &&
	    new_state == IEEE80211_STA_NONE) {
		memset(arsta, 0, sizeof(*arsta));
		arsta->arvif = arvif;
		INIT_WORK(&arsta->update_wk, ath11k_sta_rc_update_wk);

		ret = ath11k_mac_station_add(ar, vif, sta);
		if (ret)
			ath11k_warn(ar->ab, "Failed to add station: %pM for VDEV: %d\n",
				    sta->addr, arvif->vdev_id);
	} else if ((old_state == IEEE80211_STA_NONE &&
		    new_state == IEEE80211_STA_NOTEXIST)) {
		ath11k_dp_peer_cleanup(ar, arvif->vdev_id, sta->addr);

		ret = ath11k_peer_delete(ar, arvif->vdev_id, sta->addr);
		if (ret)
			ath11k_warn(ar->ab, "Failed to delete peer: %pM for VDEV: %d\n",
				    sta->addr, arvif->vdev_id);
		else
			ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "Removed peer: %pM for VDEV: %d\n",
				   sta->addr, arvif->vdev_id);

		ath11k_mac_dec_num_stations(arvif, sta);
		spin_lock_bh(&ar->ab->base_lock);
		peer = ath11k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
		if (peer && peer->sta == sta) {
			ath11k_warn(ar->ab, "Found peer entry %pM n vdev %i after it was supposedly removed\n",
				    vif->addr, arvif->vdev_id);
			peer->sta = NULL;
			list_del(&peer->list);
			kfree(peer);
			ar->num_peers--;
		}
		spin_unlock_bh(&ar->ab->base_lock);

		kfree(arsta->tx_stats);
		arsta->tx_stats = NULL;

		kfree(arsta->rx_stats);
		arsta->rx_stats = NULL;
	} else if (old_state == IEEE80211_STA_AUTH &&
		   new_state == IEEE80211_STA_ASSOC &&
		   (vif->type == NL80211_IFTYPE_AP ||
		    vif->type == NL80211_IFTYPE_MESH_POINT ||
		    vif->type == NL80211_IFTYPE_ADHOC)) {
		ret = ath11k_station_assoc(ar, vif, sta, false);
		if (ret)
			ath11k_warn(ar->ab, "Failed to associate station: %pM\n",
				    sta->addr);
	} else if (old_state == IEEE80211_STA_ASSOC &&
		   new_state == IEEE80211_STA_AUTH &&
		   (vif->type == NL80211_IFTYPE_AP ||
		    vif->type == NL80211_IFTYPE_MESH_POINT ||
		    vif->type == NL80211_IFTYPE_ADHOC)) {
		ret = ath11k_station_disassoc(ar, vif, sta);
		if (ret)
			ath11k_warn(ar->ab, "Failed to disassociate station: %pM\n",
				    sta->addr);
	}

	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static int ath11k_mac_op_sta_set_txpwr(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif,
				       struct ieee80211_sta *sta)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	int ret = 0;
	s16 txpwr;

	if (sta->txpwr.type == NL80211_TX_POWER_AUTOMATIC) {
		txpwr = 0;
	} else {
		txpwr = sta->txpwr.power;
		if (!txpwr)
			return -EINVAL;
	}

	if (txpwr > ATH11K_TX_POWER_MAX_VAL || txpwr < ATH11K_TX_POWER_MIN_VAL)
		return -EINVAL;

	mutex_lock(&ar->conf_mutex);

	ret = ath11k_wmi_set_peer_param(ar, sta->addr, arvif->vdev_id,
					WMI_PEER_USE_FIXED_PWR, txpwr);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set tx power for station ret: %d\n",
			    ret);
		goto out;
	}

out:
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static void ath11k_mac_op_sta_rc_update(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
					struct ieee80211_sta *sta,
					u32 changed)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_sta *arsta = (struct ath11k_sta *)sta->drv_priv;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	struct ath11k_peer *peer;
	u32 bw, smps;

	spin_lock_bh(&ar->ab->base_lock);

	peer = ath11k_peer_find(ar->ab, arvif->vdev_id, sta->addr);
	if (!peer) {
		spin_unlock_bh(&ar->ab->base_lock);
		ath11k_warn(ar->ab, "mac sta rc update failed to find peer %pM on vdev %i\n",
			    sta->addr, arvif->vdev_id);
		return;
	}

	spin_unlock_bh(&ar->ab->base_lock);

	ath11k_dbg(ar->ab, ATH11K_DBG_MAC,
		   "mac sta rc update for %pM changed %08x bw %d nss %d smps %d\n",
		   sta->addr, changed, sta->bandwidth, sta->rx_nss,
		   sta->smps_mode);

	spin_lock_bh(&ar->data_lock);

	if (changed & IEEE80211_RC_BW_CHANGED) {
		bw = WMI_PEER_CHWIDTH_20MHZ;

		switch (sta->bandwidth) {
		case IEEE80211_STA_RX_BW_20:
			bw = WMI_PEER_CHWIDTH_20MHZ;
			break;
		case IEEE80211_STA_RX_BW_40:
			bw = WMI_PEER_CHWIDTH_40MHZ;
			break;
		case IEEE80211_STA_RX_BW_80:
			bw = WMI_PEER_CHWIDTH_80MHZ;
			break;
		case IEEE80211_STA_RX_BW_160:
			bw = WMI_PEER_CHWIDTH_160MHZ;
			break;
		default:
			ath11k_warn(ar->ab, "Invalid bandwidth %d in rc update for %pM\n",
				    sta->bandwidth, sta->addr);
			bw = WMI_PEER_CHWIDTH_20MHZ;
			break;
		}

		arsta->bw = bw;
	}

	if (changed & IEEE80211_RC_NSS_CHANGED)
		arsta->nss = sta->rx_nss;

	if (changed & IEEE80211_RC_SMPS_CHANGED) {
		smps = WMI_PEER_SMPS_PS_NONE;

		switch (sta->smps_mode) {
		case IEEE80211_SMPS_AUTOMATIC:
		case IEEE80211_SMPS_OFF:
			smps = WMI_PEER_SMPS_PS_NONE;
			break;
		case IEEE80211_SMPS_STATIC:
			smps = WMI_PEER_SMPS_STATIC;
			break;
		case IEEE80211_SMPS_DYNAMIC:
			smps = WMI_PEER_SMPS_DYNAMIC;
			break;
		default:
			ath11k_warn(ar->ab, "Invalid smps %d in sta rc update for %pM\n",
				    sta->smps_mode, sta->addr);
			smps = WMI_PEER_SMPS_PS_NONE;
			break;
		}

		arsta->smps = smps;
	}

	arsta->changed |= changed;

	spin_unlock_bh(&ar->data_lock);

	ieee80211_queue_work(hw, &arsta->update_wk);
}

static int ath11k_conf_tx_uapsd(struct ath11k *ar, struct ieee80211_vif *vif,
				u16 ac, bool enable)
{
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	u32 value = 0;
	int ret = 0;

	if (arvif->vdev_type != WMI_VDEV_TYPE_STA)
		return 0;

	switch (ac) {
	case IEEE80211_AC_VO:
		value = WMI_STA_PS_UAPSD_AC3_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC3_TRIGGER_EN;
		break;
	case IEEE80211_AC_VI:
		value = WMI_STA_PS_UAPSD_AC2_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC2_TRIGGER_EN;
		break;
	case IEEE80211_AC_BE:
		value = WMI_STA_PS_UAPSD_AC1_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC1_TRIGGER_EN;
		break;
	case IEEE80211_AC_BK:
		value = WMI_STA_PS_UAPSD_AC0_DELIVERY_EN |
			WMI_STA_PS_UAPSD_AC0_TRIGGER_EN;
		break;
	}

	if (enable)
		arvif->u.sta.uapsd |= value;
	else
		arvif->u.sta.uapsd &= ~value;

	ret = ath11k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
					  WMI_STA_PS_PARAM_UAPSD,
					  arvif->u.sta.uapsd);
	if (ret) {
		ath11k_warn(ar->ab, "could not set uapsd params %d\n", ret);
		goto exit;
	}

	if (arvif->u.sta.uapsd)
		value = WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD;
	else
		value = WMI_STA_PS_RX_WAKE_POLICY_WAKE;

	ret = ath11k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
					  WMI_STA_PS_PARAM_RX_WAKE_POLICY,
					  value);
	if (ret)
		ath11k_warn(ar->ab, "could not set rx wake param %d\n", ret);

exit:
	return ret;
}

static int ath11k_mac_op_conf_tx(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif, u16 ac,
				 const struct ieee80211_tx_queue_params *params)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	struct wmi_wmm_params_arg *p = NULL;
	int ret;

	mutex_lock(&ar->conf_mutex);

	switch (ac) {
	case IEEE80211_AC_VO:
		p = &arvif->wmm_params.ac_vo;
		break;
	case IEEE80211_AC_VI:
		p = &arvif->wmm_params.ac_vi;
		break;
	case IEEE80211_AC_BE:
		p = &arvif->wmm_params.ac_be;
		break;
	case IEEE80211_AC_BK:
		p = &arvif->wmm_params.ac_bk;
		break;
	}

	if (WARN_ON(!p)) {
		ret = -EINVAL;
		goto exit;
	}

	p->cwmin = params->cw_min;
	p->cwmax = params->cw_max;
	p->aifs = params->aifs;
	p->txop = params->txop;

	ret = ath11k_wmi_send_wmm_update_cmd_tlv(ar, arvif->vdev_id,
						 &arvif->wmm_params);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set wmm params: %d\n", ret);
		goto exit;
	}

	ret = ath11k_conf_tx_uapsd(ar, vif, ac, params->uapsd);

	if (ret)
		ath11k_warn(ar->ab, "failed to set sta uapsd: %d\n", ret);

exit:
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static struct ieee80211_sta_ht_cap
ath11k_create_ht_cap(struct ath11k *ar, u32 ar_ht_cap, u32 rate_cap_rx_chainmask)
{
	int i;
	struct ieee80211_sta_ht_cap ht_cap = {0};
	u32 ar_vht_cap = ar->pdev->cap.vht_cap;

	if (!(ar_ht_cap & WMI_HT_CAP_ENABLED))
		return ht_cap;

	ht_cap.ht_supported = 1;
	ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
	ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_NONE;
	ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	ht_cap.cap |= IEEE80211_HT_CAP_DSSSCCK40;
	ht_cap.cap |= WLAN_HT_CAP_SM_PS_STATIC << IEEE80211_HT_CAP_SM_PS_SHIFT;

	if (ar_ht_cap & WMI_HT_CAP_HT20_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;

	if (ar_ht_cap & WMI_HT_CAP_HT40_SGI)
		ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;

	if (ar_ht_cap & WMI_HT_CAP_DYNAMIC_SMPS) {
		u32 smps;

		smps   = WLAN_HT_CAP_SM_PS_DYNAMIC;
		smps <<= IEEE80211_HT_CAP_SM_PS_SHIFT;

		ht_cap.cap |= smps;
	}

	if (ar_ht_cap & WMI_HT_CAP_TX_STBC)
		ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

	if (ar_ht_cap & WMI_HT_CAP_RX_STBC) {
		u32 stbc;

		stbc   = ar_ht_cap;
		stbc  &= WMI_HT_CAP_RX_STBC;
		stbc >>= WMI_HT_CAP_RX_STBC_MASK_SHIFT;
		stbc <<= IEEE80211_HT_CAP_RX_STBC_SHIFT;
		stbc  &= IEEE80211_HT_CAP_RX_STBC;

		ht_cap.cap |= stbc;
	}

	if (ar_ht_cap & WMI_HT_CAP_RX_LDPC)
		ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;

	if (ar_ht_cap & WMI_HT_CAP_L_SIG_TXOP_PROT)
		ht_cap.cap |= IEEE80211_HT_CAP_LSIG_TXOP_PROT;

	if (ar_vht_cap & WMI_VHT_CAP_MAX_MPDU_LEN_MASK)
		ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;

	for (i = 0; i < ar->num_rx_chains; i++) {
		if (rate_cap_rx_chainmask & BIT(i))
			ht_cap.mcs.rx_mask[i] = 0xFF;
	}

	ht_cap.mcs.tx_params |= IEEE80211_HT_MCS_TX_DEFINED;

	return ht_cap;
}

static int ath11k_mac_set_txbf_conf(struct ath11k_vif *arvif)
{
	u32 value = 0;
	struct ath11k *ar = arvif->ar;
	int nsts;
	int sound_dim;
	u32 vht_cap = ar->pdev->cap.vht_cap;
	u32 vdev_param = WMI_VDEV_PARAM_TXBF;

	if (vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE)) {
		nsts = vht_cap & IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;
		nsts >>= IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
		value |= SM(nsts, WMI_TXBF_STS_CAP_OFFSET);
	}

	if (vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE)) {
		sound_dim = vht_cap &
			    IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
		sound_dim >>= IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
		if (sound_dim > (ar->num_tx_chains - 1))
			sound_dim = ar->num_tx_chains - 1;
		value |= SM(sound_dim, WMI_BF_SOUND_DIM_OFFSET);
	}

	if (!value)
		return 0;

	if (vht_cap & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE) {
		value |= WMI_VDEV_PARAM_TXBF_SU_TX_BFER;

		if ((vht_cap & IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE) &&
		    arvif->vdev_type == WMI_VDEV_TYPE_AP)
			value |= WMI_VDEV_PARAM_TXBF_MU_TX_BFER;
	}

	/* TODO: SUBFEE not validated in HK, disable here until validated? */

	if (vht_cap & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE) {
		value |= WMI_VDEV_PARAM_TXBF_SU_TX_BFEE;

		if ((vht_cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE) &&
		    arvif->vdev_type == WMI_VDEV_TYPE_STA)
			value |= WMI_VDEV_PARAM_TXBF_MU_TX_BFEE;
	}

	return ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					     vdev_param, value);
}

static void ath11k_set_vht_txbf_cap(struct ath11k *ar, u32 *vht_cap)
{
	bool subfer, subfee;
	int sound_dim = 0;

	subfer = !!(*vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE));
	subfee = !!(*vht_cap & (IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE));

	if (ar->num_tx_chains < 2) {
		*vht_cap &= ~(IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
		subfer = false;
	}

	/* If SU Beaformer is not set, then disable MU Beamformer Capability */
	if (!subfer)
		*vht_cap &= ~(IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE);

	/* If SU Beaformee is not set, then disable MU Beamformee Capability */
	if (!subfee)
		*vht_cap &= ~(IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	sound_dim = (*vht_cap & IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK);
	sound_dim >>= IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
	*vht_cap &= ~IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;

	/* TODO: Need to check invalid STS and Sound_dim values set by FW? */

	/* Enable Sounding Dimension Field only if SU BF is enabled */
	if (subfer) {
		if (sound_dim > (ar->num_tx_chains - 1))
			sound_dim = ar->num_tx_chains - 1;

		sound_dim <<= IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
		sound_dim &=  IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
		*vht_cap |= sound_dim;
	}

	/* Use the STS advertised by FW unless SU Beamformee is not supported*/
	if (!subfee)
		*vht_cap &= ~(IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK);
}

static struct ieee80211_sta_vht_cap
ath11k_create_vht_cap(struct ath11k *ar, u32 rate_cap_tx_chainmask,
		      u32 rate_cap_rx_chainmask)
{
	struct ieee80211_sta_vht_cap vht_cap = {0};
	u16 txmcs_map, rxmcs_map;
	int i;

	vht_cap.vht_supported = 1;
	vht_cap.cap = ar->pdev->cap.vht_cap;

	ath11k_set_vht_txbf_cap(ar, &vht_cap.cap);

	/* TODO: Enable back VHT160 mode once association issues are fixed */
	/* Disabling VHT160 and VHT80+80 modes */
	vht_cap.cap &= ~IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;
	vht_cap.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_160;

	rxmcs_map = 0;
	txmcs_map = 0;
	for (i = 0; i < 8; i++) {
		if (i < ar->num_tx_chains && rate_cap_tx_chainmask & BIT(i))
			txmcs_map |= IEEE80211_VHT_MCS_SUPPORT_0_9 << (i * 2);
		else
			txmcs_map |= IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * 2);

		if (i < ar->num_rx_chains && rate_cap_rx_chainmask & BIT(i))
			rxmcs_map |= IEEE80211_VHT_MCS_SUPPORT_0_9 << (i * 2);
		else
			rxmcs_map |= IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * 2);
	}

	if (rate_cap_tx_chainmask <= 1)
		vht_cap.cap &= ~IEEE80211_VHT_CAP_TXSTBC;

	vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(rxmcs_map);
	vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(txmcs_map);

	return vht_cap;
}

static void ath11k_mac_setup_ht_vht_cap(struct ath11k *ar,
					struct ath11k_pdev_cap *cap,
					u32 *ht_cap_info)
{
	struct ieee80211_supported_band *band;
	u32 rate_cap_tx_chainmask;
	u32 rate_cap_rx_chainmask;
	u32 ht_cap;

	rate_cap_tx_chainmask = ar->cfg_tx_chainmask >> cap->tx_chain_mask_shift;
	rate_cap_rx_chainmask = ar->cfg_rx_chainmask >> cap->rx_chain_mask_shift;

	if (cap->supported_bands & WMI_HOST_WLAN_2G_CAP) {
		band = &ar->mac.sbands[NL80211_BAND_2GHZ];
		ht_cap = cap->band[NL80211_BAND_2GHZ].ht_cap_info;
		if (ht_cap_info)
			*ht_cap_info = ht_cap;
		band->ht_cap = ath11k_create_ht_cap(ar, ht_cap,
						    rate_cap_rx_chainmask);
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP && !ar->supports_6ghz) {
		band = &ar->mac.sbands[NL80211_BAND_5GHZ];
		ht_cap = cap->band[NL80211_BAND_5GHZ].ht_cap_info;
		if (ht_cap_info)
			*ht_cap_info = ht_cap;
		band->ht_cap = ath11k_create_ht_cap(ar, ht_cap,
						    rate_cap_rx_chainmask);
		band->vht_cap = ath11k_create_vht_cap(ar, rate_cap_tx_chainmask,
						      rate_cap_rx_chainmask);
	}
}

static int ath11k_check_chain_mask(struct ath11k *ar, u32 ant, bool is_tx_ant)
{
	/* TODO: Check the request chainmask against the supported
	 * chainmask table which is advertised in extented_service_ready event
	 */

	return 0;
}

static void ath11k_gen_ppe_thresh(struct ath11k_ppe_threshold *fw_ppet,
				  u8 *he_ppet)
{
	int nss, ru;
	u8 bit = 7;

	he_ppet[0] = fw_ppet->numss_m1 & IEEE80211_PPE_THRES_NSS_MASK;
	he_ppet[0] |= (fw_ppet->ru_bit_mask <<
		       IEEE80211_PPE_THRES_RU_INDEX_BITMASK_POS) &
		      IEEE80211_PPE_THRES_RU_INDEX_BITMASK_MASK;
	for (nss = 0; nss <= fw_ppet->numss_m1; nss++) {
		for (ru = 0; ru < 4; ru++) {
			u8 val;
			int i;

			if ((fw_ppet->ru_bit_mask & BIT(ru)) == 0)
				continue;
			val = (fw_ppet->ppet16_ppet8_ru3_ru0[nss] >> (ru * 6)) &
			       0x3f;
			val = ((val >> 3) & 0x7) | ((val & 0x7) << 3);
			for (i = 5; i >= 0; i--) {
				he_ppet[bit / 8] |=
					((val >> i) & 0x1) << ((bit % 8));
				bit++;
			}
		}
	}
}

static void
ath11k_mac_filter_he_cap_mesh(struct ieee80211_he_cap_elem *he_cap_elem)
{
	u8 m;

	m = IEEE80211_HE_MAC_CAP0_TWT_RES |
	    IEEE80211_HE_MAC_CAP0_TWT_REQ;
	he_cap_elem->mac_cap_info[0] &= ~m;

	m = IEEE80211_HE_MAC_CAP2_TRS |
	    IEEE80211_HE_MAC_CAP2_BCAST_TWT |
	    IEEE80211_HE_MAC_CAP2_MU_CASCADING;
	he_cap_elem->mac_cap_info[2] &= ~m;

	m = IEEE80211_HE_MAC_CAP3_FLEX_TWT_SCHED |
	    IEEE80211_HE_MAC_CAP2_BCAST_TWT |
	    IEEE80211_HE_MAC_CAP2_MU_CASCADING;
	he_cap_elem->mac_cap_info[3] &= ~m;

	m = IEEE80211_HE_MAC_CAP4_BSRP_BQRP_A_MPDU_AGG |
	    IEEE80211_HE_MAC_CAP4_BQR;
	he_cap_elem->mac_cap_info[4] &= ~m;

	m = IEEE80211_HE_MAC_CAP5_SUBCHAN_SELECTIVE_TRANSMISSION |
	    IEEE80211_HE_MAC_CAP5_UL_2x996_TONE_RU |
	    IEEE80211_HE_MAC_CAP5_PUNCTURED_SOUNDING |
	    IEEE80211_HE_MAC_CAP5_HT_VHT_TRIG_FRAME_RX;
	he_cap_elem->mac_cap_info[5] &= ~m;

	m = IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
	    IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO;
	he_cap_elem->phy_cap_info[2] &= ~m;

	m = IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU |
	    IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_MASK |
	    IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_MASK;
	he_cap_elem->phy_cap_info[3] &= ~m;

	m = IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER;
	he_cap_elem->phy_cap_info[4] &= ~m;

	m = IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
	he_cap_elem->phy_cap_info[5] &= ~m;

	m = IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
	    IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
	    IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB |
	    IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO;
	he_cap_elem->phy_cap_info[6] &= ~m;

	m = IEEE80211_HE_PHY_CAP7_PSR_BASED_SR |
	    IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP |
	    IEEE80211_HE_PHY_CAP7_STBC_TX_ABOVE_80MHZ |
	    IEEE80211_HE_PHY_CAP7_STBC_RX_ABOVE_80MHZ;
	he_cap_elem->phy_cap_info[7] &= ~m;

	m = IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI |
	    IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G |
	    IEEE80211_HE_PHY_CAP8_20MHZ_IN_160MHZ_HE_PPDU |
	    IEEE80211_HE_PHY_CAP8_80MHZ_IN_160MHZ_HE_PPDU;
	he_cap_elem->phy_cap_info[8] &= ~m;

	m = IEEE80211_HE_PHY_CAP9_LONGER_THAN_16_SIGB_OFDM_SYM |
	    IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
	    IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU |
	    IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU |
	    IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
	    IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;
	he_cap_elem->phy_cap_info[9] &= ~m;
}

static __le16 ath11k_mac_setup_he_6ghz_cap(struct ath11k_pdev_cap *pcap,
					   struct ath11k_band_cap *bcap)
{
	u8 val;

	bcap->he_6ghz_capa = IEEE80211_HT_MPDU_DENSITY_NONE;
	if (bcap->ht_cap_info & WMI_HT_CAP_DYNAMIC_SMPS)
		bcap->he_6ghz_capa |=
			FIELD_PREP(IEEE80211_HE_6GHZ_CAP_SM_PS,
				   WLAN_HT_CAP_SM_PS_DYNAMIC);
	else
		bcap->he_6ghz_capa |=
			FIELD_PREP(IEEE80211_HE_6GHZ_CAP_SM_PS,
				   WLAN_HT_CAP_SM_PS_DISABLED);
	val = FIELD_GET(IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK,
			pcap->vht_cap);
	bcap->he_6ghz_capa |=
		FIELD_PREP(IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP, val);
	val = FIELD_GET(IEEE80211_VHT_CAP_MAX_MPDU_MASK, pcap->vht_cap);
	bcap->he_6ghz_capa |=
		FIELD_PREP(IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN, val);
	if (pcap->vht_cap & IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN)
		bcap->he_6ghz_capa |= IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS;
	if (pcap->vht_cap & IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN)
		bcap->he_6ghz_capa |= IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS;

	return cpu_to_le16(bcap->he_6ghz_capa);
}

static int ath11k_mac_copy_he_cap(struct ath11k *ar,
				  struct ath11k_pdev_cap *cap,
				  struct ieee80211_sband_iftype_data *data,
				  int band)
{
	int i, idx = 0;

	for (i = 0; i < NUM_NL80211_IFTYPES; i++) {
		struct ieee80211_sta_he_cap *he_cap = &data[idx].he_cap;
		struct ath11k_band_cap *band_cap = &cap->band[band];
		struct ieee80211_he_cap_elem *he_cap_elem =
				&he_cap->he_cap_elem;

		switch (i) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_AP:
		case NL80211_IFTYPE_MESH_POINT:
			break;

		default:
			continue;
		}

		data[idx].types_mask = BIT(i);
		he_cap->has_he = true;
		memcpy(he_cap_elem->mac_cap_info, band_cap->he_cap_info,
		       sizeof(he_cap_elem->mac_cap_info));
		memcpy(he_cap_elem->phy_cap_info, band_cap->he_cap_phy_info,
		       sizeof(he_cap_elem->phy_cap_info));

		he_cap_elem->mac_cap_info[1] &=
			IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_MASK;

		he_cap_elem->phy_cap_info[5] &=
			~IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_MASK;
		he_cap_elem->phy_cap_info[5] |= ar->num_tx_chains - 1;

		switch (i) {
		case NL80211_IFTYPE_AP:
			he_cap_elem->phy_cap_info[3] &=
				~IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_MASK;
			he_cap_elem->phy_cap_info[9] |=
				IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU;
			break;
		case NL80211_IFTYPE_STATION:
			he_cap_elem->mac_cap_info[0] &=
				~IEEE80211_HE_MAC_CAP0_TWT_RES;
			he_cap_elem->mac_cap_info[0] |=
				IEEE80211_HE_MAC_CAP0_TWT_REQ;
			he_cap_elem->phy_cap_info[9] |=
				IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU;
			break;
		case NL80211_IFTYPE_MESH_POINT:
			ath11k_mac_filter_he_cap_mesh(he_cap_elem);
			break;
		}

		he_cap->he_mcs_nss_supp.rx_mcs_80 =
			cpu_to_le16(band_cap->he_mcs & 0xffff);
		he_cap->he_mcs_nss_supp.tx_mcs_80 =
			cpu_to_le16(band_cap->he_mcs & 0xffff);
		he_cap->he_mcs_nss_supp.rx_mcs_160 =
			cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
		he_cap->he_mcs_nss_supp.tx_mcs_160 =
			cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
		he_cap->he_mcs_nss_supp.rx_mcs_80p80 =
			cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);
		he_cap->he_mcs_nss_supp.tx_mcs_80p80 =
			cpu_to_le16((band_cap->he_mcs >> 16) & 0xffff);

		memset(he_cap->ppe_thres, 0, sizeof(he_cap->ppe_thres));
		if (he_cap_elem->phy_cap_info[6] &
		    IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT)
			ath11k_gen_ppe_thresh(&band_cap->he_ppet,
					      he_cap->ppe_thres);

		if (band == NL80211_BAND_6GHZ) {
			data[idx].he_6ghz_capa.capa =
				ath11k_mac_setup_he_6ghz_cap(cap, band_cap);
		}
		idx++;
	}

	return idx;
}

static void ath11k_mac_setup_he_cap(struct ath11k *ar,
				    struct ath11k_pdev_cap *cap)
{
	struct ieee80211_supported_band *band;
	int count;

	if (cap->supported_bands & WMI_HOST_WLAN_2G_CAP) {
		count = ath11k_mac_copy_he_cap(ar, cap,
					       ar->mac.iftype[NL80211_BAND_2GHZ],
					       NL80211_BAND_2GHZ);
		band = &ar->mac.sbands[NL80211_BAND_2GHZ];
		band->iftype_data = ar->mac.iftype[NL80211_BAND_2GHZ];
		band->n_iftype_data = count;
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP) {
		count = ath11k_mac_copy_he_cap(ar, cap,
					       ar->mac.iftype[NL80211_BAND_5GHZ],
					       NL80211_BAND_5GHZ);
		band = &ar->mac.sbands[NL80211_BAND_5GHZ];
		band->iftype_data = ar->mac.iftype[NL80211_BAND_5GHZ];
		band->n_iftype_data = count;
	}

	if (cap->supported_bands & WMI_HOST_WLAN_5G_CAP &&
	    ar->supports_6ghz) {
		count = ath11k_mac_copy_he_cap(ar, cap,
					       ar->mac.iftype[NL80211_BAND_6GHZ],
					       NL80211_BAND_6GHZ);
		band = &ar->mac.sbands[NL80211_BAND_6GHZ];
		band->iftype_data = ar->mac.iftype[NL80211_BAND_6GHZ];
		band->n_iftype_data = count;
	}
}

static int __ath11k_set_antenna(struct ath11k *ar, u32 tx_ant, u32 rx_ant)
{
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	if (ath11k_check_chain_mask(ar, tx_ant, true))
		return -EINVAL;

	if (ath11k_check_chain_mask(ar, rx_ant, false))
		return -EINVAL;

	ar->cfg_tx_chainmask = tx_ant;
	ar->cfg_rx_chainmask = rx_ant;

	if (ar->state != ATH11K_STATE_ON &&
	    ar->state != ATH11K_STATE_RESTARTED)
		return 0;

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_TX_CHAIN_MASK,
					tx_ant, ar->pdev->pdev_id);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set tx-chainmask: %d, req 0x%x\n",
			    ret, tx_ant);
		return ret;
	}

	ar->num_tx_chains = get_num_chains(tx_ant);

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_RX_CHAIN_MASK,
					rx_ant, ar->pdev->pdev_id);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set rx-chainmask: %d, req 0x%x\n",
			    ret, rx_ant);
		return ret;
	}

	ar->num_rx_chains = get_num_chains(rx_ant);

	/* Reload HT/VHT/HE capability */
	ath11k_mac_setup_ht_vht_cap(ar, &ar->pdev->cap, NULL);
	ath11k_mac_setup_he_cap(ar, &ar->pdev->cap);

	return 0;
}

int ath11k_mac_tx_mgmt_pending_free(int buf_id, void *skb, void *ctx)
{
	struct sk_buff *msdu = skb;
	struct ieee80211_tx_info *info;
	struct ath11k *ar = ctx;
	struct ath11k_base *ab = ar->ab;

	spin_lock_bh(&ar->txmgmt_idr_lock);
	idr_remove(&ar->txmgmt_idr, buf_id);
	spin_unlock_bh(&ar->txmgmt_idr_lock);
	dma_unmap_single(ab->dev, ATH11K_SKB_CB(msdu)->paddr, msdu->len,
			 DMA_TO_DEVICE);

	info = IEEE80211_SKB_CB(msdu);
	memset(&info->status, 0, sizeof(info->status));

	ieee80211_free_txskb(ar->hw, msdu);

	return 0;
}

static int ath11k_mac_vif_txmgmt_idr_remove(int buf_id, void *skb, void *ctx)
{
	struct ieee80211_vif *vif = ctx;
	struct ath11k_skb_cb *skb_cb = ATH11K_SKB_CB((struct sk_buff *)skb);
	struct sk_buff *msdu = skb;
	struct ath11k *ar = skb_cb->ar;
	struct ath11k_base *ab = ar->ab;

	if (skb_cb->vif == vif) {
		spin_lock_bh(&ar->txmgmt_idr_lock);
		idr_remove(&ar->txmgmt_idr, buf_id);
		spin_unlock_bh(&ar->txmgmt_idr_lock);
		dma_unmap_single(ab->dev, skb_cb->paddr, msdu->len,
				 DMA_TO_DEVICE);
	}

	return 0;
}

static int ath11k_mac_mgmt_tx_wmi(struct ath11k *ar, struct ath11k_vif *arvif,
				  struct sk_buff *skb)
{
	struct ath11k_base *ab = ar->ab;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *info;
	dma_addr_t paddr;
	int buf_id;
	int ret;

	spin_lock_bh(&ar->txmgmt_idr_lock);
	buf_id = idr_alloc(&ar->txmgmt_idr, skb, 0,
			   ATH11K_TX_MGMT_NUM_PENDING_MAX, GFP_ATOMIC);
	spin_unlock_bh(&ar->txmgmt_idr_lock);
	if (buf_id < 0)
		return -ENOSPC;

	info = IEEE80211_SKB_CB(skb);
	if (!(info->flags & IEEE80211_TX_CTL_HW_80211_ENCAP)) {
		if ((ieee80211_is_action(hdr->frame_control) ||
		     ieee80211_is_deauth(hdr->frame_control) ||
		     ieee80211_is_disassoc(hdr->frame_control)) &&
		     ieee80211_has_protected(hdr->frame_control)) {
			skb_put(skb, IEEE80211_CCMP_MIC_LEN);
		}
	}

	paddr = dma_map_single(ab->dev, skb->data, skb->len, DMA_TO_DEVICE);
	if (dma_mapping_error(ab->dev, paddr)) {
		ath11k_warn(ab, "failed to DMA map mgmt Tx buffer\n");
		ret = -EIO;
		goto err_free_idr;
	}

	ATH11K_SKB_CB(skb)->paddr = paddr;

	ret = ath11k_wmi_mgmt_send(ar, arvif->vdev_id, buf_id, skb);
	if (ret) {
		ath11k_warn(ar->ab, "failed to send mgmt frame: %d\n", ret);
		goto err_unmap_buf;
	}

	return 0;

err_unmap_buf:
	dma_unmap_single(ab->dev, ATH11K_SKB_CB(skb)->paddr,
			 skb->len, DMA_TO_DEVICE);
err_free_idr:
	spin_lock_bh(&ar->txmgmt_idr_lock);
	idr_remove(&ar->txmgmt_idr, buf_id);
	spin_unlock_bh(&ar->txmgmt_idr_lock);

	return ret;
}

static void ath11k_mgmt_over_wmi_tx_purge(struct ath11k *ar)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&ar->wmi_mgmt_tx_queue)) != NULL)
		ieee80211_free_txskb(ar->hw, skb);
}

static void ath11k_mgmt_over_wmi_tx_work(struct work_struct *work)
{
	struct ath11k *ar = container_of(work, struct ath11k, wmi_mgmt_tx_work);
	struct ath11k_skb_cb *skb_cb;
	struct ath11k_vif *arvif;
	struct sk_buff *skb;
	int ret;

	while ((skb = skb_dequeue(&ar->wmi_mgmt_tx_queue)) != NULL) {
		skb_cb = ATH11K_SKB_CB(skb);
		if (!skb_cb->vif) {
			ath11k_warn(ar->ab, "no vif found for mgmt frame\n");
			ieee80211_free_txskb(ar->hw, skb);
			continue;
		}

		arvif = ath11k_vif_to_arvif(skb_cb->vif);
		if (ar->allocated_vdev_map & (1LL << arvif->vdev_id) &&
		    arvif->is_started) {
			ret = ath11k_mac_mgmt_tx_wmi(ar, arvif, skb);
			if (ret) {
				ath11k_warn(ar->ab, "failed to tx mgmt frame, vdev_id %d :%d\n",
					    arvif->vdev_id, ret);
				ieee80211_free_txskb(ar->hw, skb);
			} else {
				atomic_inc(&ar->num_pending_mgmt_tx);
			}
		} else {
			ath11k_warn(ar->ab,
				    "dropping mgmt frame for vdev %d, is_started %d\n",
				    arvif->vdev_id,
				    arvif->is_started);
			ieee80211_free_txskb(ar->hw, skb);
		}
	}
}

static int ath11k_mac_mgmt_tx(struct ath11k *ar, struct sk_buff *skb,
			      bool is_prb_rsp)
{
	struct sk_buff_head *q = &ar->wmi_mgmt_tx_queue;

	if (test_bit(ATH11K_FLAG_CRASH_FLUSH, &ar->ab->dev_flags))
		return -ESHUTDOWN;

	/* Drop probe response packets when the pending management tx
	 * count has reached a certain threshold, so as to prioritize
	 * other mgmt packets like auth and assoc to be sent on time
	 * for establishing successful connections.
	 */
	if (is_prb_rsp &&
	    atomic_read(&ar->num_pending_mgmt_tx) > ATH11K_PRB_RSP_DROP_THRESHOLD) {
		ath11k_warn(ar->ab,
			    "dropping probe response as pending queue is almost full\n");
		return -ENOSPC;
	}

	if (skb_queue_len_lockless(q) >= ATH11K_TX_MGMT_NUM_PENDING_MAX) {
		ath11k_warn(ar->ab, "mgmt tx queue is full\n");
		return -ENOSPC;
	}

	skb_queue_tail(q, skb);
	ieee80211_queue_work(ar->hw, &ar->wmi_mgmt_tx_work);

	return 0;
}

static void ath11k_mac_op_tx(struct ieee80211_hw *hw,
			     struct ieee80211_tx_control *control,
			     struct sk_buff *skb)
{
	struct ath11k_skb_cb *skb_cb = ATH11K_SKB_CB(skb);
	struct ath11k *ar = hw->priv;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_vif *vif = info->control.vif;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_key_conf *key = info->control.hw_key;
	u32 info_flags = info->flags;
	bool is_prb_rsp;
	int ret;

	memset(skb_cb, 0, sizeof(*skb_cb));
	skb_cb->vif = vif;

	if (key) {
		skb_cb->cipher = key->cipher;
		skb_cb->flags |= ATH11K_SKB_CIPHER_SET;
	}

	if (info_flags & IEEE80211_TX_CTL_HW_80211_ENCAP) {
		skb_cb->flags |= ATH11K_SKB_HW_80211_ENCAP;
	} else if (ieee80211_is_mgmt(hdr->frame_control)) {
		is_prb_rsp = ieee80211_is_probe_resp(hdr->frame_control);
		ret = ath11k_mac_mgmt_tx(ar, skb, is_prb_rsp);
		if (ret) {
			ath11k_warn(ar->ab, "failed to queue management frame %d\n",
				    ret);
			ieee80211_free_txskb(ar->hw, skb);
		}
		return;
	}

	ret = ath11k_dp_tx(ar, arvif, skb);
	if (ret) {
		ath11k_warn(ar->ab, "failed to transmit frame %d\n", ret);
		ieee80211_free_txskb(ar->hw, skb);
	}
}

void ath11k_mac_drain_tx(struct ath11k *ar)
{
	/* make sure rcu-protected mac80211 tx path itself is drained */
	synchronize_net();

	cancel_work_sync(&ar->wmi_mgmt_tx_work);
	ath11k_mgmt_over_wmi_tx_purge(ar);
}

static int ath11k_mac_config_mon_status_default(struct ath11k *ar, bool enable)
{
	struct htt_rx_ring_tlv_filter tlv_filter = {0};
	struct ath11k_base *ab = ar->ab;
	int i, ret = 0;
	u32 ring_id;

	if (enable) {
		tlv_filter = ath11k_mac_mon_status_filter_default;
		if (ath11k_debugfs_rx_filter(ar))
			tlv_filter.rx_filter = ath11k_debugfs_rx_filter(ar);
	}

	for (i = 0; i < ab->hw_params.num_rxmda_per_pdev; i++) {
		ring_id = ar->dp.rx_mon_status_refill_ring[i].refill_buf_ring.ring_id;
		ret = ath11k_dp_tx_htt_rx_filter_setup(ar->ab, ring_id,
						       ar->dp.mac_id + i,
						       HAL_RXDMA_MONITOR_STATUS,
						       DP_RX_BUFFER_SIZE,
						       &tlv_filter);
	}

	if (enable && !ar->ab->hw_params.rxdma1_enable)
		mod_timer(&ar->ab->mon_reap_timer, jiffies +
			  msecs_to_jiffies(ATH11K_MON_TIMER_INTERVAL));

	return ret;
}

static int ath11k_mac_op_start(struct ieee80211_hw *hw)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_pdev *pdev = ar->pdev;
	int ret;

	ath11k_mac_drain_tx(ar);
	mutex_lock(&ar->conf_mutex);

	switch (ar->state) {
	case ATH11K_STATE_OFF:
		ar->state = ATH11K_STATE_ON;
		break;
	case ATH11K_STATE_RESTARTING:
		ar->state = ATH11K_STATE_RESTARTED;
		break;
	case ATH11K_STATE_RESTARTED:
	case ATH11K_STATE_WEDGED:
	case ATH11K_STATE_ON:
		WARN_ON(1);
		ret = -EINVAL;
		goto err;
	}

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_PMF_QOS,
					1, pdev->pdev_id);

	if (ret) {
		ath11k_err(ar->ab, "failed to enable PMF QOS: (%d\n", ret);
		goto err;
	}

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_DYNAMIC_BW, 1,
					pdev->pdev_id);
	if (ret) {
		ath11k_err(ar->ab, "failed to enable dynamic bw: %d\n", ret);
		goto err;
	}

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_ARP_AC_OVERRIDE,
					0, pdev->pdev_id);
	if (ret) {
		ath11k_err(ab, "failed to set ac override for ARP: %d\n",
			   ret);
		goto err;
	}

	ret = ath11k_wmi_send_dfs_phyerr_offload_enable_cmd(ar, pdev->pdev_id);
	if (ret) {
		ath11k_err(ab, "failed to offload radar detection: %d\n",
			   ret);
		goto err;
	}

	ret = ath11k_dp_tx_htt_h2t_ppdu_stats_req(ar,
						  HTT_PPDU_STATS_TAG_DEFAULT);
	if (ret) {
		ath11k_err(ab, "failed to req ppdu stats: %d\n", ret);
		goto err;
	}

	ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_MESH_MCAST_ENABLE,
					1, pdev->pdev_id);

	if (ret) {
		ath11k_err(ar->ab, "failed to enable MESH MCAST ENABLE: (%d\n", ret);
		goto err;
	}

	__ath11k_set_antenna(ar, ar->cfg_tx_chainmask, ar->cfg_rx_chainmask);

	/* TODO: Do we need to enable ANI? */

	ath11k_reg_update_chan_list(ar);

	ar->num_started_vdevs = 0;
	ar->num_created_vdevs = 0;
	ar->num_peers = 0;
	ar->allocated_vdev_map = 0;

	/* Configure monitor status ring with default rx_filter to get rx status
	 * such as rssi, rx_duration.
	 */
	ret = ath11k_mac_config_mon_status_default(ar, true);
	if (ret) {
		ath11k_err(ab, "failed to configure monitor status ring with default rx_filter: (%d)\n",
			   ret);
		goto err;
	}

	/* Configure the hash seed for hash based reo dest ring selection */
	ath11k_wmi_pdev_lro_cfg(ar, ar->pdev->pdev_id);

	/* allow device to enter IMPS */
	if (ab->hw_params.idle_ps) {
		ret = ath11k_wmi_pdev_set_param(ar, WMI_PDEV_PARAM_IDLE_PS_CONFIG,
						1, pdev->pdev_id);
		if (ret) {
			ath11k_err(ab, "failed to enable idle ps: %d\n", ret);
			goto err;
		}
	}

	mutex_unlock(&ar->conf_mutex);

	rcu_assign_pointer(ab->pdevs_active[ar->pdev_idx],
			   &ab->pdevs[ar->pdev_idx]);

	return 0;

err:
	ar->state = ATH11K_STATE_OFF;
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void ath11k_mac_op_stop(struct ieee80211_hw *hw)
{
	struct ath11k *ar = hw->priv;
	struct htt_ppdu_stats_info *ppdu_stats, *tmp;
	int ret;

	ath11k_mac_drain_tx(ar);

	mutex_lock(&ar->conf_mutex);
	ret = ath11k_mac_config_mon_status_default(ar, false);
	if (ret)
		ath11k_err(ar->ab, "failed to clear rx_filter for monitor status ring: (%d)\n",
			   ret);

	clear_bit(ATH11K_CAC_RUNNING, &ar->dev_flags);
	ar->state = ATH11K_STATE_OFF;
	mutex_unlock(&ar->conf_mutex);

	cancel_delayed_work_sync(&ar->scan.timeout);
	cancel_work_sync(&ar->regd_update_work);

	spin_lock_bh(&ar->data_lock);
	list_for_each_entry_safe(ppdu_stats, tmp, &ar->ppdu_stats_info, list) {
		list_del(&ppdu_stats->list);
		kfree(ppdu_stats);
	}
	spin_unlock_bh(&ar->data_lock);

	rcu_assign_pointer(ar->ab->pdevs_active[ar->pdev_idx], NULL);

	synchronize_rcu();

	atomic_set(&ar->num_pending_mgmt_tx, 0);
}

static void
ath11k_mac_setup_vdev_create_params(struct ath11k_vif *arvif,
				    struct vdev_create_params *params)
{
	struct ath11k *ar = arvif->ar;
	struct ath11k_pdev *pdev = ar->pdev;

	params->if_id = arvif->vdev_id;
	params->type = arvif->vdev_type;
	params->subtype = arvif->vdev_subtype;
	params->pdev_id = pdev->pdev_id;

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP) {
		params->chains[NL80211_BAND_2GHZ].tx = ar->num_tx_chains;
		params->chains[NL80211_BAND_2GHZ].rx = ar->num_rx_chains;
	}
	if (pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP) {
		params->chains[NL80211_BAND_5GHZ].tx = ar->num_tx_chains;
		params->chains[NL80211_BAND_5GHZ].rx = ar->num_rx_chains;
	}
	if (pdev->cap.supported_bands & WMI_HOST_WLAN_5G_CAP &&
	    ar->supports_6ghz) {
		params->chains[NL80211_BAND_6GHZ].tx = ar->num_tx_chains;
		params->chains[NL80211_BAND_6GHZ].rx = ar->num_rx_chains;
	}
}

static u32
ath11k_mac_prepare_he_mode(struct ath11k_pdev *pdev, u32 viftype)
{
	struct ath11k_pdev_cap *pdev_cap = &pdev->cap;
	struct ath11k_band_cap *cap_band = NULL;
	u32 *hecap_phy_ptr = NULL;
	u32 hemode = 0;

	if (pdev->cap.supported_bands & WMI_HOST_WLAN_2G_CAP)
		cap_band = &pdev_cap->band[NL80211_BAND_2GHZ];
	else
		cap_band = &pdev_cap->band[NL80211_BAND_5GHZ];

	hecap_phy_ptr = &cap_band->he_cap_phy_info[0];

	hemode = FIELD_PREP(HE_MODE_SU_TX_BFEE, HE_SU_BFEE_ENABLE) |
		 FIELD_PREP(HE_MODE_SU_TX_BFER, HECAP_PHY_SUBFMR_GET(hecap_phy_ptr)) |
		 FIELD_PREP(HE_MODE_UL_MUMIMO, HECAP_PHY_ULMUMIMO_GET(hecap_phy_ptr));

	/* TODO WDS and other modes */
	if (viftype == NL80211_IFTYPE_AP) {
		hemode |= FIELD_PREP(HE_MODE_MU_TX_BFER,
			  HECAP_PHY_MUBFMR_GET(hecap_phy_ptr)) |
			  FIELD_PREP(HE_MODE_DL_OFDMA, HE_DL_MUOFDMA_ENABLE) |
			  FIELD_PREP(HE_MODE_UL_OFDMA, HE_UL_MUOFDMA_ENABLE);
	} else {
		hemode |= FIELD_PREP(HE_MODE_MU_TX_BFEE, HE_MU_BFEE_ENABLE);
	}

	return hemode;
}

static int ath11k_set_he_mu_sounding_mode(struct ath11k *ar,
					  struct ath11k_vif *arvif)
{
	u32 param_id, param_value;
	struct ath11k_base *ab = ar->ab;
	int ret = 0;

	param_id = WMI_VDEV_PARAM_SET_HEMU_MODE;
	param_value = ath11k_mac_prepare_he_mode(ar->pdev, arvif->vif->type);
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath11k_warn(ab, "failed to set vdev %d HE MU mode: %d param_value %x\n",
			    arvif->vdev_id, ret, param_value);
		return ret;
	}
	param_id = WMI_VDEV_PARAM_SET_HE_SOUNDING_MODE;
	param_value =
		FIELD_PREP(HE_VHT_SOUNDING_MODE, HE_VHT_SOUNDING_MODE_ENABLE) |
		FIELD_PREP(HE_TRIG_NONTRIG_SOUNDING_MODE,
			   HE_TRIG_NONTRIG_SOUNDING_MODE_ENABLE);
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath11k_warn(ab, "failed to set vdev %d HE MU mode: %d\n",
			    arvif->vdev_id, ret);
		return ret;
	}
	return ret;
}

static void ath11k_mac_op_update_vif_offload(struct ieee80211_hw *hw,
					     struct ieee80211_vif *vif)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	u32 param_id, param_value;
	int ret;

	param_id = WMI_VDEV_PARAM_TX_ENCAP_TYPE;
	if (ath11k_frame_mode != ATH11K_HW_TXRX_ETHERNET ||
	    (vif->type != NL80211_IFTYPE_STATION &&
	     vif->type != NL80211_IFTYPE_AP))
		vif->offload_flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;

	if (vif->offload_flags & IEEE80211_OFFLOAD_ENCAP_ENABLED)
		param_value = ATH11K_HW_TXRX_ETHERNET;
	else if (test_bit(ATH11K_FLAG_RAW_MODE, &ab->dev_flags))
		param_value = ATH11K_HW_TXRX_RAW;
	else
		param_value = ATH11K_HW_TXRX_NATIVE_WIFI;

	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath11k_warn(ab, "failed to set vdev %d tx encap mode: %d\n",
			    arvif->vdev_id, ret);
		vif->offload_flags &= ~IEEE80211_OFFLOAD_ENCAP_ENABLED;
	}
}

static int ath11k_mac_op_add_interface(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct vdev_create_params vdev_param = {0};
	struct peer_create_params peer_param;
	u32 param_id, param_value;
	u16 nss;
	int i;
	int ret;
	int bit;

	vif->driver_flags |= IEEE80211_VIF_SUPPORTS_UAPSD;

	mutex_lock(&ar->conf_mutex);

	if (vif->type == NL80211_IFTYPE_AP &&
	    ar->num_peers > (ar->max_num_peers - 1)) {
		ath11k_warn(ab, "failed to create vdev due to insufficient peer entry resource in firmware\n");
		ret = -ENOBUFS;
		goto err;
	}

	if (ar->num_created_vdevs > (TARGET_NUM_VDEVS - 1)) {
		ath11k_warn(ab, "failed to create vdev, reached max vdev limit %d\n",
			    TARGET_NUM_VDEVS);
		ret = -EBUSY;
		goto err;
	}

	memset(arvif, 0, sizeof(*arvif));

	arvif->ar = ar;
	arvif->vif = vif;

	INIT_LIST_HEAD(&arvif->list);
	INIT_DELAYED_WORK(&arvif->connection_loss_work,
			  ath11k_mac_vif_sta_connection_loss_work);

	for (i = 0; i < ARRAY_SIZE(arvif->bitrate_mask.control); i++) {
		arvif->bitrate_mask.control[i].legacy = 0xffffffff;
		memset(arvif->bitrate_mask.control[i].ht_mcs, 0xff,
		       sizeof(arvif->bitrate_mask.control[i].ht_mcs));
		memset(arvif->bitrate_mask.control[i].vht_mcs, 0xff,
		       sizeof(arvif->bitrate_mask.control[i].vht_mcs));
	}

	bit = __ffs64(ab->free_vdev_map);

	arvif->vdev_id = bit;
	arvif->vdev_subtype = WMI_VDEV_SUBTYPE_NONE;

	switch (vif->type) {
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_STATION:
		arvif->vdev_type = WMI_VDEV_TYPE_STA;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		arvif->vdev_subtype = WMI_VDEV_SUBTYPE_MESH_11S;
		fallthrough;
	case NL80211_IFTYPE_AP:
		arvif->vdev_type = WMI_VDEV_TYPE_AP;
		break;
	case NL80211_IFTYPE_MONITOR:
		arvif->vdev_type = WMI_VDEV_TYPE_MONITOR;
		break;
	default:
		WARN_ON(1);
		break;
	}

	ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "mac add interface id %d type %d subtype %d map %llx\n",
		   arvif->vdev_id, arvif->vdev_type, arvif->vdev_subtype,
		   ab->free_vdev_map);

	vif->cab_queue = arvif->vdev_id % (ATH11K_HW_MAX_QUEUES - 1);
	for (i = 0; i < ARRAY_SIZE(vif->hw_queue); i++)
		vif->hw_queue[i] = i % (ATH11K_HW_MAX_QUEUES - 1);

	ath11k_mac_setup_vdev_create_params(arvif, &vdev_param);

	ret = ath11k_wmi_vdev_create(ar, vif->addr, &vdev_param);
	if (ret) {
		ath11k_warn(ab, "failed to create WMI vdev %d: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	ar->num_created_vdevs++;
	ath11k_dbg(ab, ATH11K_DBG_MAC, "vdev %pM created, vdev_id %d\n",
		   vif->addr, arvif->vdev_id);
	ar->allocated_vdev_map |= 1LL << arvif->vdev_id;
	ab->free_vdev_map &= ~(1LL << arvif->vdev_id);

	spin_lock_bh(&ar->data_lock);
	list_add(&arvif->list, &ar->arvifs);
	spin_unlock_bh(&ar->data_lock);

	ath11k_mac_op_update_vif_offload(hw, vif);

	nss = get_num_chains(ar->cfg_tx_chainmask) ? : 1;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    WMI_VDEV_PARAM_NSS, nss);
	if (ret) {
		ath11k_warn(ab, "failed to set vdev %d chainmask 0x%x, nss %d :%d\n",
			    arvif->vdev_id, ar->cfg_tx_chainmask, nss, ret);
		goto err_vdev_del;
	}

	switch (arvif->vdev_type) {
	case WMI_VDEV_TYPE_AP:
		peer_param.vdev_id = arvif->vdev_id;
		peer_param.peer_addr = vif->addr;
		peer_param.peer_type = WMI_PEER_TYPE_DEFAULT;
		ret = ath11k_peer_create(ar, arvif, NULL, &peer_param);
		if (ret) {
			ath11k_warn(ab, "failed to vdev %d create peer for AP: %d\n",
				    arvif->vdev_id, ret);
			goto err_vdev_del;
		}

		ret = ath11k_mac_set_kickout(arvif);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set vdev %i kickout parameters: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}
		break;
	case WMI_VDEV_TYPE_STA:
		param_id = WMI_STA_PS_PARAM_RX_WAKE_POLICY;
		param_value = WMI_STA_PS_RX_WAKE_POLICY_WAKE;
		ret = ath11k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set vdev %d RX wake policy: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		param_id = WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD;
		param_value = WMI_STA_PS_TX_WAKE_THRESHOLD_ALWAYS;
		ret = ath11k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set vdev %d TX wake threshold: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		param_id = WMI_STA_PS_PARAM_PSPOLL_COUNT;
		param_value = WMI_STA_PS_PSPOLL_COUNT_NO_MAX;
		ret = ath11k_wmi_set_sta_ps_param(ar, arvif->vdev_id,
						  param_id, param_value);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set vdev %d pspoll count: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}

		ret = ath11k_wmi_pdev_set_ps_mode(ar, arvif->vdev_id, false);
		if (ret) {
			ath11k_warn(ar->ab, "failed to disable vdev %d ps mode: %d\n",
				    arvif->vdev_id, ret);
			goto err_peer_del;
		}
		break;
	default:
		break;
	}

	arvif->txpower = vif->bss_conf.txpower;
	ret = ath11k_mac_txpower_recalc(ar);
	if (ret)
		goto err_peer_del;

	param_id = WMI_VDEV_PARAM_RTS_THRESHOLD;
	param_value = ar->hw->wiphy->rts_threshold;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    param_id, param_value);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set rts threshold for vdev %d: %d\n",
			    arvif->vdev_id, ret);
	}

	ath11k_dp_vdev_tx_attach(ar, arvif);

	mutex_unlock(&ar->conf_mutex);

	return 0;

err_peer_del:
	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		reinit_completion(&ar->peer_delete_done);

		ret = ath11k_wmi_send_peer_delete_cmd(ar, vif->addr,
						      arvif->vdev_id);
		if (ret) {
			ath11k_warn(ar->ab, "failed to delete peer vdev_id %d addr %pM\n",
				    arvif->vdev_id, vif->addr);
			goto err;
		}

		ret = ath11k_wait_for_peer_delete_done(ar, arvif->vdev_id,
						       vif->addr);
		if (ret)
			goto err;

		ar->num_peers--;
	}

err_vdev_del:
	ath11k_wmi_vdev_delete(ar, arvif->vdev_id);
	ar->num_created_vdevs--;
	ar->allocated_vdev_map &= ~(1LL << arvif->vdev_id);
	ab->free_vdev_map |= 1LL << arvif->vdev_id;
	spin_lock_bh(&ar->data_lock);
	list_del(&arvif->list);
	spin_unlock_bh(&ar->data_lock);

err:
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static int ath11k_mac_vif_unref(int buf_id, void *skb, void *ctx)
{
	struct ieee80211_vif *vif = (struct ieee80211_vif *)ctx;
	struct ath11k_skb_cb *skb_cb = ATH11K_SKB_CB((struct sk_buff *)skb);

	if (skb_cb->vif == vif)
		skb_cb->vif = NULL;

	return 0;
}

static void ath11k_mac_op_remove_interface(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_vif *arvif = ath11k_vif_to_arvif(vif);
	struct ath11k_base *ab = ar->ab;
	unsigned long time_left;
	int ret;
	int i;

	cancel_delayed_work_sync(&arvif->connection_loss_work);

	mutex_lock(&ar->conf_mutex);

	ath11k_dbg(ab, ATH11K_DBG_MAC, "mac remove interface (vdev %d)\n",
		   arvif->vdev_id);

	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		ret = ath11k_peer_delete(ar, arvif->vdev_id, vif->addr);
		if (ret)
			ath11k_warn(ab, "failed to submit AP self-peer removal on vdev %d: %d\n",
				    arvif->vdev_id, ret);
	}

	reinit_completion(&ar->vdev_delete_done);

	ret = ath11k_wmi_vdev_delete(ar, arvif->vdev_id);
	if (ret) {
		ath11k_warn(ab, "failed to delete WMI vdev %d: %d\n",
			    arvif->vdev_id, ret);
		goto err_vdev_del;
	}

	time_left = wait_for_completion_timeout(&ar->vdev_delete_done,
						ATH11K_VDEV_DELETE_TIMEOUT_HZ);
	if (time_left == 0) {
		ath11k_warn(ab, "Timeout in receiving vdev delete response\n");
		goto err_vdev_del;
	}

	ab->free_vdev_map |= 1LL << (arvif->vdev_id);
	ar->allocated_vdev_map &= ~(1LL << arvif->vdev_id);
	ar->num_created_vdevs--;

	ath11k_dbg(ab, ATH11K_DBG_MAC, "vdev %pM deleted, vdev_id %d\n",
		   vif->addr, arvif->vdev_id);

err_vdev_del:
	spin_lock_bh(&ar->data_lock);
	list_del(&arvif->list);
	spin_unlock_bh(&ar->data_lock);

	ath11k_peer_cleanup(ar, arvif->vdev_id);

	idr_for_each(&ar->txmgmt_idr,
		     ath11k_mac_vif_txmgmt_idr_remove, vif);

	for (i = 0; i < DP_TCL_NUM_RING_MAX; i++) {
		spin_lock_bh(&ab->dp.tx_ring[i].tx_idr_lock);
		idr_for_each(&ab->dp.tx_ring[i].txbuf_idr,
			     ath11k_mac_vif_unref, vif);
		spin_unlock_bh(&ab->dp.tx_ring[i].tx_idr_lock);
	}

	/* Recalc txpower for remaining vdev */
	ath11k_mac_txpower_recalc(ar);
	clear_bit(ATH11K_FLAG_MONITOR_ENABLED, &ar->monitor_flags);

	/* TODO: recal traffic pause state based on the available vdevs */

	mutex_unlock(&ar->conf_mutex);
}

/* FIXME: Has to be verified. */
#define SUPPORTED_FILTERS			\
	(FIF_ALLMULTI |				\
	FIF_CONTROL |				\
	FIF_PSPOLL |				\
	FIF_OTHER_BSS |				\
	FIF_BCN_PRBRESP_PROMISC |		\
	FIF_PROBE_REQ |				\
	FIF_FCSFAIL)

static void ath11k_mac_op_configure_filter(struct ieee80211_hw *hw,
					   unsigned int changed_flags,
					   unsigned int *total_flags,
					   u64 multicast)
{
	struct ath11k *ar = hw->priv;
	bool reset_flag = false;
	int ret = 0;

	mutex_lock(&ar->conf_mutex);

	changed_flags &= SUPPORTED_FILTERS;
	*total_flags &= SUPPORTED_FILTERS;
	ar->filter_flags = *total_flags;

	/* For monitor mode */
	reset_flag = !(ar->filter_flags & FIF_BCN_PRBRESP_PROMISC);

	ret = ath11k_dp_tx_htt_monitor_mode_ring_config(ar, reset_flag);
	if (!ret) {
		if (!reset_flag)
			set_bit(ATH11K_FLAG_MONITOR_ENABLED, &ar->monitor_flags);
		else
			clear_bit(ATH11K_FLAG_MONITOR_ENABLED, &ar->monitor_flags);
	} else {
		ath11k_warn(ar->ab,
			    "fail to set monitor filter: %d\n", ret);
	}
	ath11k_dbg(ar->ab, ATH11K_DBG_MAC,
		   "changed_flags:0x%x, total_flags:0x%x, reset_flag:%d\n",
		   changed_flags, *total_flags, reset_flag);

	mutex_unlock(&ar->conf_mutex);
}

static int ath11k_mac_op_get_antenna(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant)
{
	struct ath11k *ar = hw->priv;

	mutex_lock(&ar->conf_mutex);

	*tx_ant = ar->cfg_tx_chainmask;
	*rx_ant = ar->cfg_rx_chainmask;

	mutex_unlock(&ar->conf_mutex);

	return 0;
}

static int ath11k_mac_op_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant)
{
	struct ath11k *ar = hw->priv;
	int ret;

	mutex_lock(&ar->conf_mutex);
	ret = __ath11k_set_antenna(ar, tx_ant, rx_ant);
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static int ath11k_mac_op_ampdu_action(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_ampdu_params *params)
{
	struct ath11k *ar = hw->priv;
	int ret = -EINVAL;

	mutex_lock(&ar->conf_mutex);

	switch (params->action) {
	case IEEE80211_AMPDU_RX_START:
		ret = ath11k_dp_rx_ampdu_start(ar, params);
		break;
	case IEEE80211_AMPDU_RX_STOP:
		ret = ath11k_dp_rx_ampdu_stop(ar, params);
		break;
	case IEEE80211_AMPDU_TX_START:
	case IEEE80211_AMPDU_TX_STOP_CONT:
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		/* Tx A-MPDU aggregation offloaded to hw/fw so deny mac80211
		 * Tx aggregation requests.
		 */
		ret = -EOPNOTSUPP;
		break;
	}

	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static int ath11k_mac_op_add_chanctx(struct ieee80211_hw *hw,
				     struct ieee80211_chanctx_conf *ctx)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac chanctx add freq %u width %d ptr %pK\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx);

	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	/* TODO: In case of multiple channel context, populate rx_channel from
	 * Rx PPDU desc information.
	 */
	ar->rx_channel = ctx->def.chan;
	spin_unlock_bh(&ar->data_lock);

	mutex_unlock(&ar->conf_mutex);

	return 0;
}

static void ath11k_mac_op_remove_chanctx(struct ieee80211_hw *hw,
					 struct ieee80211_chanctx_conf *ctx)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac chanctx remove freq %u width %d ptr %pK\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx);

	mutex_lock(&ar->conf_mutex);

	spin_lock_bh(&ar->data_lock);
	/* TODO: In case of there is one more channel context left, populate
	 * rx_channel with the channel of that remaining channel context.
	 */
	ar->rx_channel = NULL;
	spin_unlock_bh(&ar->data_lock);

	mutex_unlock(&ar->conf_mutex);
}

static inline int ath11k_mac_vdev_setup_sync(struct ath11k *ar)
{
	lockdep_assert_held(&ar->conf_mutex);

	if (test_bit(ATH11K_FLAG_CRASH_FLUSH, &ar->ab->dev_flags))
		return -ESHUTDOWN;

	if (!wait_for_completion_timeout(&ar->vdev_setup_done,
					 ATH11K_VDEV_SETUP_TIMEOUT_HZ))
		return -ETIMEDOUT;

	return ar->last_wmi_vdev_start_status ? -EINVAL : 0;
}

static int
ath11k_mac_vdev_start_restart(struct ath11k_vif *arvif,
			      const struct cfg80211_chan_def *chandef,
			      bool restart)
{
	struct ath11k *ar = arvif->ar;
	struct ath11k_base *ab = ar->ab;
	struct wmi_vdev_start_req_arg arg = {};
	int he_support = arvif->vif->bss_conf.he_support;
	int ret = 0;

	lockdep_assert_held(&ar->conf_mutex);

	reinit_completion(&ar->vdev_setup_done);

	arg.vdev_id = arvif->vdev_id;
	arg.dtim_period = arvif->dtim_period;
	arg.bcn_intval = arvif->beacon_interval;

	arg.channel.freq = chandef->chan->center_freq;
	arg.channel.band_center_freq1 = chandef->center_freq1;
	arg.channel.band_center_freq2 = chandef->center_freq2;
	arg.channel.mode =
		ath11k_phymodes[chandef->chan->band][chandef->width];

	arg.channel.min_power = 0;
	arg.channel.max_power = chandef->chan->max_power * 2;
	arg.channel.max_reg_power = chandef->chan->max_reg_power * 2;
	arg.channel.max_antenna_gain = chandef->chan->max_antenna_gain * 2;

	arg.pref_tx_streams = ar->num_tx_chains;
	arg.pref_rx_streams = ar->num_rx_chains;

	if (arvif->vdev_type == WMI_VDEV_TYPE_AP) {
		arg.ssid = arvif->u.ap.ssid;
		arg.ssid_len = arvif->u.ap.ssid_len;
		arg.hidden_ssid = arvif->u.ap.hidden_ssid;

		/* For now allow DFS for AP mode */
		arg.channel.chan_radar =
			!!(chandef->chan->flags & IEEE80211_CHAN_RADAR);

		arg.channel.freq2_radar =
			!!(chandef->chan->flags & IEEE80211_CHAN_RADAR);

		arg.channel.passive = arg.channel.chan_radar;

		spin_lock_bh(&ab->base_lock);
		arg.regdomain = ar->ab->dfs_region;
		spin_unlock_bh(&ab->base_lock);

		if (he_support) {
			ret = ath11k_set_he_mu_sounding_mode(ar, arvif);
			if (ret) {
				ath11k_warn(ar->ab, "failed to set he mode vdev %i\n",
					    arg.vdev_id);
				return ret;
			}
		}
	}

	arg.channel.passive |= !!(chandef->chan->flags & IEEE80211_CHAN_NO_IR);

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac vdev %d start center_freq %d phymode %s\n",
		   arg.vdev_id, arg.channel.freq,
		   ath11k_wmi_phymode_str(arg.channel.mode));

	ret = ath11k_wmi_vdev_start(ar, &arg, restart);
	if (ret) {
		ath11k_warn(ar->ab, "failed to %s WMI vdev %i\n",
			    restart ? "restart" : "start", arg.vdev_id);
		return ret;
	}

	ret = ath11k_mac_vdev_setup_sync(ar);
	if (ret) {
		ath11k_warn(ab, "failed to synchronize setup for vdev %i %s: %d\n",
			    arg.vdev_id, restart ? "restart" : "start", ret);
		return ret;
	}

	ar->num_started_vdevs++;
	ath11k_dbg(ab, ATH11K_DBG_MAC,  "vdev %pM started, vdev_id %d\n",
		   arvif->vif->addr, arvif->vdev_id);

	/* Enable CAC Flag in the driver by checking the channel DFS cac time,
	 * i.e dfs_cac_ms value which will be valid only for radar channels
	 * and state as NL80211_DFS_USABLE which indicates CAC needs to be
	 * done before channel usage. This flags is used to drop rx packets.
	 * during CAC.
	 */
	/* TODO Set the flag for other interface types as required */
	if (arvif->vdev_type == WMI_VDEV_TYPE_AP &&
	    chandef->chan->dfs_cac_ms &&
	    chandef->chan->dfs_state == NL80211_DFS_USABLE) {
		set_bit(ATH11K_CAC_RUNNING, &ar->dev_flags);
		ath11k_dbg(ab, ATH11K_DBG_MAC,
			   "CAC Started in chan_freq %d for vdev %d\n",
			   arg.channel.freq, arg.vdev_id);
	}

	ret = ath11k_mac_set_txbf_conf(arvif);
	if (ret)
		ath11k_warn(ab, "failed to set txbf conf for vdev %d: %d\n",
			    arvif->vdev_id, ret);

	return 0;
}

static int ath11k_mac_vdev_stop(struct ath11k_vif *arvif)
{
	struct ath11k *ar = arvif->ar;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	reinit_completion(&ar->vdev_setup_done);

	ret = ath11k_wmi_vdev_stop(ar, arvif->vdev_id);
	if (ret) {
		ath11k_warn(ar->ab, "failed to stop WMI vdev %i: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	ret = ath11k_mac_vdev_setup_sync(ar);
	if (ret) {
		ath11k_warn(ar->ab, "failed to synchronize setup for vdev %i: %d\n",
			    arvif->vdev_id, ret);
		goto err;
	}

	WARN_ON(ar->num_started_vdevs == 0);

	ar->num_started_vdevs--;
	ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "vdev %pM stopped, vdev_id %d\n",
		   arvif->vif->addr, arvif->vdev_id);

	if (test_bit(ATH11K_CAC_RUNNING, &ar->dev_flags)) {
		clear_bit(ATH11K_CAC_RUNNING, &ar->dev_flags);
		ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "CAC Stopped for vdev %d\n",
			   arvif->vdev_id);
	}

	return 0;
err:
	return ret;
}

static int ath11k_mac_vdev_start(struct ath11k_vif *arvif,
				 const struct cfg80211_chan_def *chandef)
{
	return ath11k_mac_vdev_start_restart(arvif, chandef, false);
}

static int ath11k_mac_vdev_restart(struct ath11k_vif *arvif,
				   const struct cfg80211_chan_def *chandef)
{
	return ath11k_mac_vdev_start_restart(arvif, chandef, true);
}

struct ath11k_mac_change_chanctx_arg {
	struct ieee80211_chanctx_conf *ctx;
	struct ieee80211_vif_chanctx_switch *vifs;
	int n_vifs;
	int next_vif;
};

static void
ath11k_mac_change_chanctx_cnt_iter(void *data, u8 *mac,
				   struct ieee80211_vif *vif)
{
	struct ath11k_mac_change_chanctx_arg *arg = data;

	if (rcu_access_pointer(vif->chanctx_conf) != arg->ctx)
		return;

	arg->n_vifs++;
}

static void
ath11k_mac_change_chanctx_fill_iter(void *data, u8 *mac,
				    struct ieee80211_vif *vif)
{
	struct ath11k_mac_change_chanctx_arg *arg = data;
	struct ieee80211_chanctx_conf *ctx;

	ctx = rcu_access_pointer(vif->chanctx_conf);
	if (ctx != arg->ctx)
		return;

	if (WARN_ON(arg->next_vif == arg->n_vifs))
		return;

	arg->vifs[arg->next_vif].vif = vif;
	arg->vifs[arg->next_vif].old_ctx = ctx;
	arg->vifs[arg->next_vif].new_ctx = ctx;
	arg->next_vif++;
}

static void
ath11k_mac_update_vif_chan(struct ath11k *ar,
			   struct ieee80211_vif_chanctx_switch *vifs,
			   int n_vifs)
{
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif;
	int ret;
	int i;

	lockdep_assert_held(&ar->conf_mutex);

	for (i = 0; i < n_vifs; i++) {
		arvif = (void *)vifs[i].vif->drv_priv;

		ath11k_dbg(ab, ATH11K_DBG_MAC,
			   "mac chanctx switch vdev_id %i freq %u->%u width %d->%d\n",
			   arvif->vdev_id,
			   vifs[i].old_ctx->def.chan->center_freq,
			   vifs[i].new_ctx->def.chan->center_freq,
			   vifs[i].old_ctx->def.width,
			   vifs[i].new_ctx->def.width);

		if (WARN_ON(!arvif->is_started))
			continue;

		if (WARN_ON(!arvif->is_up))
			continue;

		ret = ath11k_wmi_vdev_down(ar, arvif->vdev_id);
		if (ret) {
			ath11k_warn(ab, "failed to down vdev %d: %d\n",
				    arvif->vdev_id, ret);
			continue;
		}
	}

	/* All relevant vdevs are downed and associated channel resources
	 * should be available for the channel switch now.
	 */

	/* TODO: Update ar->rx_channel */

	for (i = 0; i < n_vifs; i++) {
		arvif = (void *)vifs[i].vif->drv_priv;

		if (WARN_ON(!arvif->is_started))
			continue;

		if (WARN_ON(!arvif->is_up))
			continue;

		ret = ath11k_mac_setup_bcn_tmpl(arvif);
		if (ret)
			ath11k_warn(ab, "failed to update bcn tmpl during csa: %d\n",
				    ret);

		ret = ath11k_mac_vdev_restart(arvif, &vifs[i].new_ctx->def);
		if (ret) {
			ath11k_warn(ab, "failed to restart vdev %d: %d\n",
				    arvif->vdev_id, ret);
			continue;
		}

		ret = ath11k_wmi_vdev_up(arvif->ar, arvif->vdev_id, arvif->aid,
					 arvif->bssid);
		if (ret) {
			ath11k_warn(ab, "failed to bring vdev up %d: %d\n",
				    arvif->vdev_id, ret);
			continue;
		}
	}
}

static void
ath11k_mac_update_active_vif_chan(struct ath11k *ar,
				  struct ieee80211_chanctx_conf *ctx)
{
	struct ath11k_mac_change_chanctx_arg arg = { .ctx = ctx };

	lockdep_assert_held(&ar->conf_mutex);

	ieee80211_iterate_active_interfaces_atomic(ar->hw,
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath11k_mac_change_chanctx_cnt_iter,
						   &arg);
	if (arg.n_vifs == 0)
		return;

	arg.vifs = kcalloc(arg.n_vifs, sizeof(arg.vifs[0]), GFP_KERNEL);
	if (!arg.vifs)
		return;

	ieee80211_iterate_active_interfaces_atomic(ar->hw,
						   IEEE80211_IFACE_ITER_NORMAL,
						   ath11k_mac_change_chanctx_fill_iter,
						   &arg);

	ath11k_mac_update_vif_chan(ar, arg.vifs, arg.n_vifs);

	kfree(arg.vifs);
}

static void ath11k_mac_op_change_chanctx(struct ieee80211_hw *hw,
					 struct ieee80211_chanctx_conf *ctx,
					 u32 changed)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;

	mutex_lock(&ar->conf_mutex);

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac chanctx change freq %u width %d ptr %pK changed %x\n",
		   ctx->def.chan->center_freq, ctx->def.width, ctx, changed);

	/* This shouldn't really happen because channel switching should use
	 * switch_vif_chanctx().
	 */
	if (WARN_ON(changed & IEEE80211_CHANCTX_CHANGE_CHANNEL))
		goto unlock;

	if (changed & IEEE80211_CHANCTX_CHANGE_WIDTH)
		ath11k_mac_update_active_vif_chan(ar, ctx);

	/* TODO: Recalc radar detection */

unlock:
	mutex_unlock(&ar->conf_mutex);
}

static int ath11k_start_vdev_delay(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	int ret;

	if (WARN_ON(arvif->is_started))
		return -EBUSY;

	ret = ath11k_mac_vdev_start(arvif, &arvif->chanctx.def);
	if (ret) {
		ath11k_warn(ab, "failed to start vdev %i addr %pM on freq %d: %d\n",
			    arvif->vdev_id, vif->addr,
			    arvif->chanctx.def.chan->center_freq, ret);
		return ret;
	}

	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ret = ath11k_monitor_vdev_up(ar, arvif->vdev_id);
		if (ret) {
			ath11k_warn(ab, "failed put monitor up: %d\n", ret);
			return ret;
		}
	}

	arvif->is_started = true;

	/* TODO: Setup ps and cts/rts protection */
	return 0;
}

static int
ath11k_mac_op_assign_vif_chanctx(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_chanctx_conf *ctx)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	int ret;
	struct peer_create_params param;

	mutex_lock(&ar->conf_mutex);

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac chanctx assign ptr %pK vdev_id %i\n",
		   ctx, arvif->vdev_id);

	/* for QCA6390 bss peer must be created before vdev_start */
	if (ab->hw_params.vdev_start_delay &&
	    arvif->vdev_type != WMI_VDEV_TYPE_AP &&
	    arvif->vdev_type != WMI_VDEV_TYPE_MONITOR &&
	    !ath11k_peer_find_by_vdev_id(ab, arvif->vdev_id)) {
		memcpy(&arvif->chanctx, ctx, sizeof(*ctx));
		ret = 0;
		goto out;
	}

	if (WARN_ON(arvif->is_started)) {
		ret = -EBUSY;
		goto out;
	}

	if (ab->hw_params.vdev_start_delay &&
	    arvif->vdev_type != WMI_VDEV_TYPE_AP &&
	    arvif->vdev_type != WMI_VDEV_TYPE_MONITOR) {
		param.vdev_id = arvif->vdev_id;
		param.peer_type = WMI_PEER_TYPE_DEFAULT;
		param.peer_addr = ar->mac_addr;

		ret = ath11k_peer_create(ar, arvif, NULL, &param);
		if (ret) {
			ath11k_warn(ab, "failed to create peer after vdev start delay: %d",
				    ret);
			goto out;
		}
	}

	ret = ath11k_mac_vdev_start(arvif, &ctx->def);
	if (ret) {
		ath11k_warn(ab, "failed to start vdev %i addr %pM on freq %d: %d\n",
			    arvif->vdev_id, vif->addr,
			    ctx->def.chan->center_freq, ret);
		goto out;
	}
	if (arvif->vdev_type == WMI_VDEV_TYPE_MONITOR) {
		ret = ath11k_monitor_vdev_up(ar, arvif->vdev_id);
		if (ret)
			goto out;
	}

	arvif->is_started = true;

	/* TODO: Setup ps and cts/rts protection */

	ret = 0;

out:
	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void
ath11k_mac_op_unassign_vif_chanctx(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_chanctx_conf *ctx)
{
	struct ath11k *ar = hw->priv;
	struct ath11k_base *ab = ar->ab;
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	int ret;

	mutex_lock(&ar->conf_mutex);

	ath11k_dbg(ab, ATH11K_DBG_MAC,
		   "mac chanctx unassign ptr %pK vdev_id %i\n",
		   ctx, arvif->vdev_id);

	WARN_ON(!arvif->is_started);

	if (ab->hw_params.vdev_start_delay &&
	    arvif->vdev_type == WMI_VDEV_TYPE_MONITOR &&
	    ath11k_peer_find_by_addr(ab, ar->mac_addr))
		ath11k_peer_delete(ar, arvif->vdev_id, ar->mac_addr);

	ret = ath11k_mac_vdev_stop(arvif);
	if (ret)
		ath11k_warn(ab, "failed to stop vdev %i: %d\n",
			    arvif->vdev_id, ret);

	arvif->is_started = false;

	if (ab->hw_params.vdev_start_delay &&
	    arvif->vdev_type == WMI_VDEV_TYPE_MONITOR)
		ath11k_wmi_vdev_down(ar, arvif->vdev_id);

	mutex_unlock(&ar->conf_mutex);
}

static int
ath11k_mac_op_switch_vif_chanctx(struct ieee80211_hw *hw,
				 struct ieee80211_vif_chanctx_switch *vifs,
				 int n_vifs,
				 enum ieee80211_chanctx_switch_mode mode)
{
	struct ath11k *ar = hw->priv;

	mutex_lock(&ar->conf_mutex);

	ath11k_dbg(ar->ab, ATH11K_DBG_MAC,
		   "mac chanctx switch n_vifs %d mode %d\n",
		   n_vifs, mode);
	ath11k_mac_update_vif_chan(ar, vifs, n_vifs);

	mutex_unlock(&ar->conf_mutex);

	return 0;
}

static int
ath11k_set_vdev_param_to_all_vifs(struct ath11k *ar, int param, u32 value)
{
	struct ath11k_vif *arvif;
	int ret = 0;

	mutex_lock(&ar->conf_mutex);
	list_for_each_entry(arvif, &ar->arvifs, list) {
		ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "setting mac vdev %d param %d value %d\n",
			   param, arvif->vdev_id, value);

		ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
						    param, value);
		if (ret) {
			ath11k_warn(ar->ab, "failed to set param %d for vdev %d: %d\n",
				    param, arvif->vdev_id, ret);
			break;
		}
	}
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

/* mac80211 stores device specific RTS/Fragmentation threshold value,
 * this is set interface specific to firmware from ath11k driver
 */
static int ath11k_mac_op_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	struct ath11k *ar = hw->priv;
	int param_id = WMI_VDEV_PARAM_RTS_THRESHOLD;

	return ath11k_set_vdev_param_to_all_vifs(ar, param_id, value);
}

static int ath11k_mac_op_set_frag_threshold(struct ieee80211_hw *hw, u32 value)
{
	/* Even though there's a WMI vdev param for fragmentation threshold no
	 * known firmware actually implements it. Moreover it is not possible to
	 * rely frame fragmentation to mac80211 because firmware clears the
	 * "more fragments" bit in frame control making it impossible for remote
	 * devices to reassemble frames.
	 *
	 * Hence implement a dummy callback just to say fragmentation isn't
	 * supported. This effectively prevents mac80211 from doing frame
	 * fragmentation in software.
	 */
	return -EOPNOTSUPP;
}

static void ath11k_mac_op_flush(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
				u32 queues, bool drop)
{
	struct ath11k *ar = hw->priv;
	long time_left;

	if (drop)
		return;

	time_left = wait_event_timeout(ar->dp.tx_empty_waitq,
				       (atomic_read(&ar->dp.num_tx_pending) == 0),
				       ATH11K_FLUSH_TIMEOUT);
	if (time_left == 0)
		ath11k_warn(ar->ab, "failed to flush transmit queue %ld\n", time_left);
}

static int
ath11k_mac_bitrate_mask_num_ht_rates(struct ath11k *ar,
				     enum nl80211_band band,
				     const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(mask->control[band].ht_mcs); i++)
		num_rates += hweight16(mask->control[band].ht_mcs[i]);

	return num_rates;
}

static bool
ath11k_mac_has_single_legacy_rate(struct ath11k *ar,
				  enum nl80211_band band,
				  const struct cfg80211_bitrate_mask *mask)
{
	int num_rates = 0;

	num_rates = hweight32(mask->control[band].legacy);

	if (ath11k_mac_bitrate_mask_num_ht_rates(ar, band, mask))
		return false;

	if (ath11k_mac_bitrate_mask_num_vht_rates(ar, band, mask))
		return false;

	return num_rates == 1;
}

static bool
ath11k_mac_bitrate_mask_get_single_nss(struct ath11k *ar,
				       enum nl80211_band band,
				       const struct cfg80211_bitrate_mask *mask,
				       int *nss)
{
	struct ieee80211_supported_band *sband = &ar->mac.sbands[band];
	u16 vht_mcs_map = le16_to_cpu(sband->vht_cap.vht_mcs.tx_mcs_map);
	u8 ht_nss_mask = 0;
	u8 vht_nss_mask = 0;
	int i;

	/* No need to consider legacy here. Basic rates are always present
	 * in bitrate mask
	 */

	for (i = 0; i < ARRAY_SIZE(mask->control[band].ht_mcs); i++) {
		if (mask->control[band].ht_mcs[i] == 0)
			continue;
		else if (mask->control[band].ht_mcs[i] ==
			 sband->ht_cap.mcs.rx_mask[i])
			ht_nss_mask |= BIT(i);
		else
			return false;
	}

	for (i = 0; i < ARRAY_SIZE(mask->control[band].vht_mcs); i++) {
		if (mask->control[band].vht_mcs[i] == 0)
			continue;
		else if (mask->control[band].vht_mcs[i] ==
			 ath11k_mac_get_max_vht_mcs_map(vht_mcs_map, i))
			vht_nss_mask |= BIT(i);
		else
			return false;
	}

	if (ht_nss_mask != vht_nss_mask)
		return false;

	if (ht_nss_mask == 0)
		return false;

	if (BIT(fls(ht_nss_mask)) - 1 != ht_nss_mask)
		return false;

	*nss = fls(ht_nss_mask);

	return true;
}

static int
ath11k_mac_get_single_legacy_rate(struct ath11k *ar,
				  enum nl80211_band band,
				  const struct cfg80211_bitrate_mask *mask,
				  u32 *rate, u8 *nss)
{
	int rate_idx;
	u16 bitrate;
	u8 preamble;
	u8 hw_rate;

	if (hweight32(mask->control[band].legacy) != 1)
		return -EINVAL;

	rate_idx = ffs(mask->control[band].legacy) - 1;

	if (band == NL80211_BAND_5GHZ || band == NL80211_BAND_6GHZ)
		rate_idx += ATH11K_MAC_FIRST_OFDM_RATE_IDX;

	hw_rate = ath11k_legacy_rates[rate_idx].hw_value;
	bitrate = ath11k_legacy_rates[rate_idx].bitrate;

	if (ath11k_mac_bitrate_is_cck(bitrate))
		preamble = WMI_RATE_PREAMBLE_CCK;
	else
		preamble = WMI_RATE_PREAMBLE_OFDM;

	*nss = 1;
	*rate = ATH11K_HW_RATE_CODE(hw_rate, 0, preamble);

	return 0;
}

static int ath11k_mac_set_fixed_rate_params(struct ath11k_vif *arvif,
					    u32 rate, u8 nss, u8 sgi, u8 ldpc)
{
	struct ath11k *ar = arvif->ar;
	u32 vdev_param;
	int ret;

	lockdep_assert_held(&ar->conf_mutex);

	ath11k_dbg(ar->ab, ATH11K_DBG_MAC, "mac set fixed rate params vdev %i rate 0x%02x nss %u sgi %u\n",
		   arvif->vdev_id, rate, nss, sgi);

	vdev_param = WMI_VDEV_PARAM_FIXED_RATE;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, rate);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set fixed rate param 0x%02x: %d\n",
			    rate, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_NSS;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, nss);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set nss param %d: %d\n",
			    nss, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_SGI;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, sgi);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set sgi param %d: %d\n",
			    sgi, ret);
		return ret;
	}

	vdev_param = WMI_VDEV_PARAM_LDPC;
	ret = ath11k_wmi_vdev_set_param_cmd(ar, arvif->vdev_id,
					    vdev_param, ldpc);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set ldpc param %d: %d\n",
			    ldpc, ret);
		return ret;
	}

	return 0;
}

static bool
ath11k_mac_vht_mcs_range_present(struct ath11k *ar,
				 enum nl80211_band band,
				 const struct cfg80211_bitrate_mask *mask)
{
	int i;
	u16 vht_mcs;

	for (i = 0; i < NL80211_VHT_NSS_MAX; i++) {
		vht_mcs = mask->control[band].vht_mcs[i];

		switch (vht_mcs) {
		case 0:
		case BIT(8) - 1:
		case BIT(9) - 1:
		case BIT(10) - 1:
			break;
		default:
			return false;
		}
	}

	return true;
}

static void ath11k_mac_set_bitrate_mask_iter(void *data,
					     struct ieee80211_sta *sta)
{
	struct ath11k_vif *arvif = data;
	struct ath11k_sta *arsta = (struct ath11k_sta *)sta->drv_priv;
	struct ath11k *ar = arvif->ar;

	spin_lock_bh(&ar->data_lock);
	arsta->changed |= IEEE80211_RC_SUPP_RATES_CHANGED;
	spin_unlock_bh(&ar->data_lock);

	ieee80211_queue_work(ar->hw, &arsta->update_wk);
}

static void ath11k_mac_disable_peer_fixed_rate(void *data,
					       struct ieee80211_sta *sta)
{
	struct ath11k_vif *arvif = data;
	struct ath11k *ar = arvif->ar;
	int ret;

	ret = ath11k_wmi_set_peer_param(ar, sta->addr,
					arvif->vdev_id,
					WMI_PEER_PARAM_FIXED_RATE,
					WMI_FIXED_RATE_NONE);
	if (ret)
		ath11k_warn(ar->ab,
			    "failed to disable peer fixed rate for STA %pM ret %d\n",
			    sta->addr, ret);
}

static int
ath11k_mac_op_set_bitrate_mask(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       const struct cfg80211_bitrate_mask *mask)
{
	struct ath11k_vif *arvif = (void *)vif->drv_priv;
	struct cfg80211_chan_def def;
	struct ath11k *ar = arvif->ar;
	enum nl80211_band band;
	const u8 *ht_mcs_mask;
	const u16 *vht_mcs_mask;
	u32 rate;
	u8 nss;
	u8 sgi;
	u8 ldpc;
	int single_nss;
	int ret;
	int num_rates;

	if (ath11k_mac_vif_chan(vif, &def))
		return -EPERM;

	band = def.chan->band;
	ht_mcs_mask = mask->control[band].ht_mcs;
	vht_mcs_mask = mask->control[band].vht_mcs;
	ldpc = !!(ar->ht_cap_info & WMI_HT_CAP_LDPC);

	sgi = mask->control[band].gi;
	if (sgi == NL80211_TXRATE_FORCE_LGI)
		return -EINVAL;

	/* mac80211 doesn't support sending a fixed HT/VHT MCS alone, rather it
	 * requires passing atleast one of used basic rates along with them.
	 * Fixed rate setting across different preambles(legacy, HT, VHT) is
	 * not supported by the FW. Hence use of FIXED_RATE vdev param is not
	 * suitable for setting single HT/VHT rates.
	 * But, there could be a single basic rate passed from userspace which
	 * can be done through the FIXED_RATE param.
	 */
	if (ath11k_mac_has_single_legacy_rate(ar, band, mask)) {
		ret = ath11k_mac_get_single_legacy_rate(ar, band, mask, &rate,
							&nss);
		if (ret) {
			ath11k_warn(ar->ab, "failed to get single legacy rate for vdev %i: %d\n",
				    arvif->vdev_id, ret);
			return ret;
		}
		ieee80211_iterate_stations_atomic(ar->hw,
						  ath11k_mac_disable_peer_fixed_rate,
						  arvif);
	} else if (ath11k_mac_bitrate_mask_get_single_nss(ar, band, mask,
							  &single_nss)) {
		rate = WMI_FIXED_RATE_NONE;
		nss = single_nss;
	} else {
		rate = WMI_FIXED_RATE_NONE;
		nss = min_t(u32, ar->num_tx_chains,
			    max(ath11k_mac_max_ht_nss(ht_mcs_mask),
				ath11k_mac_max_vht_nss(vht_mcs_mask)));

		/* If multiple rates across different preambles are given
		 * we can reconfigure this info with all peers using PEER_ASSOC
		 * command with the below exception cases.
		 * - Single VHT Rate : peer_assoc command accommodates only MCS
		 * range values i.e 0-7, 0-8, 0-9 for VHT. Though mac80211
		 * mandates passing basic rates along with HT/VHT rates, FW
		 * doesn't allow switching from VHT to Legacy. Hence instead of
		 * setting legacy and VHT rates using RATEMASK_CMD vdev cmd,
		 * we could set this VHT rate as peer fixed rate param, which
		 * will override FIXED rate and FW rate control algorithm.
		 * If single VHT rate is passed along with HT rates, we select
		 * the VHT rate as fixed rate for vht peers.
		 * - Multiple VHT Rates : When Multiple VHT rates are given,this
		 * can be set using RATEMASK CMD which uses FW rate-ctl alg.
		 * TODO: Setting multiple VHT MCS and replacing peer_assoc with
		 * RATEMASK_CMDID can cover all use cases of setting rates
		 * across multiple preambles and rates within same type.
		 * But requires more validation of the command at this point.
		 */

		num_rates = ath11k_mac_bitrate_mask_num_vht_rates(ar, band,
								  mask);

		if (!ath11k_mac_vht_mcs_range_present(ar, band, mask) &&
		    num_rates > 1) {
			/* TODO: Handle multiple VHT MCS values setting using
			 * RATEMASK CMD
			 */
			ath11k_warn(ar->ab,
				    "Setting more than one MCS Value in bitrate mask not supported\n");
			return -EINVAL;
		}

		ieee80211_iterate_stations_atomic(ar->hw,
						  ath11k_mac_disable_peer_fixed_rate,
						  arvif);

		mutex_lock(&ar->conf_mutex);

		arvif->bitrate_mask = *mask;
		ieee80211_iterate_stations_atomic(ar->hw,
						  ath11k_mac_set_bitrate_mask_iter,
						  arvif);

		mutex_unlock(&ar->conf_mutex);
	}

	mutex_lock(&ar->conf_mutex);

	ret = ath11k_mac_set_fixed_rate_params(arvif, rate, nss, sgi, ldpc);
	if (ret) {
		ath11k_warn(ar->ab, "failed to set fixed rate params on vdev %i: %d\n",
			    arvif->vdev_id, ret);
	}

	mutex_unlock(&ar->conf_mutex);

	return ret;
}

static void
ath11k_mac_op_reconfig_complete(struct ieee80211_hw *hw,
				enum ieee80211_reconfig_type reconfig_type)
{
	struct ath11k *ar = hw->priv;

	if (reconfig_type != IEEE80211_RECONFIG_TYPE_RESTART)
		return;

	mutex_lock(&ar->conf_mutex);

	if (ar->state == ATH11K_STATE_RESTARTED) {
		ath11k_warn(ar->ab, "pdev %d successfully recovered\n",
			    ar->pdev->pdev_id);
		ar->state = ATH11K_STATE_ON;
		ieee80211_wake_queues(ar->hw);
	}

	mutex_unlock(&ar->conf_mutex);
}

static void
ath11k_mac_update_bss_chan_survey(struct ath11k *ar,
				  struct ieee80211_channel *channel)
{
	int ret;
	enum wmi_bss_chan_info_req_type type = WMI_BSS_SURVEY_REQ_TYPE_READ;

	lockdep_assert_held(&ar->conf_mutex);

	if (!test_bit(WMI_TLV_SERVICE_BSS_CHANNEL_INFO_64, ar->ab->wmi_ab.svc_map) ||
	    ar->rx_channel != channel)
		return;

	if (ar->scan.state != ATH11K_SCAN_IDLE) {
		ath11k_dbg(ar->ab, ATH11K_DBG_MAC,
			   "ignoring bss chan info req while scanning..\n");
		return;
	}

	reinit_completion(&ar->bss_survey_done);

	ret = ath11k_wmi_pdev_bss_chan_info_request(ar, type);
	if (ret) {
		ath11k_warn(ar->ab, "failed to send pdev bss chan info request\n");
		return;
	}

	ret = wait_for_completion_timeout(&ar->bss_survey_done, 3 * HZ);
	if (ret == 0)
		ath11k_warn(ar->ab, "bss channel survey timed out\n");
}

static int ath11k_mac_op_get_survey(struct ieee80211_hw *hw, int idx,
				    struct survey_info *survey)
{
	struct ath11k *ar = hw->priv;
	struct ieee80211_supported_band *sband;
	struct survey_info *ar_survey;
	int ret = 0;

	if (idx >= ATH11K_NUM_CHANS)
		return -ENOENT;

	ar_survey = &ar->survey[idx];

	mutex_lock(&ar->conf_mutex);

	sband = hw->wiphy->bands[NL80211_BAND_2GHZ];
	if (sband && idx >= sband->n_channels) {
		idx -= sband->n_channels;
		sband = NULL;
	}

	if (!sband)
		sband = hw->wiphy->bands[NL80211_BAND_5GHZ];

	if (!sband || idx >= sband->n_channels) {
		ret = -ENOENT;
		goto exit;
	}

	ath11k_mac_update_bss_chan_survey(ar, &sband->channels[idx]);

	spin_lock_bh(&ar->data_lock);
	memcpy(survey, ar_survey, sizeof(*survey));
	spin_unlock_bh(&ar->data_lock);

	survey->channel = &sband->channels[idx];

	if (ar->rx_channel == survey->channel)
		survey->filled |= SURVEY_INFO_IN_USE;

exit:
	mutex_unlock(&ar->conf_mutex);
	return ret;
}

static void ath11k_mac_op_sta_statistics(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 struct ieee80211_sta *sta,
					 struct station_info *sinfo)
{
	struct ath11k_sta *arsta = (struct ath11k_sta *)sta->drv_priv;

	sinfo->rx_duration = arsta->rx_duration;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_RX_DURATION);

	sinfo->tx_duration = arsta->tx_duration;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_DURATION);

	if (!arsta->txrate.legacy && !arsta->txrate.nss)
		return;

	if (arsta->txrate.legacy) {
		sinfo->txrate.legacy = arsta->txrate.legacy;
	} else {
		sinfo->txrate.mcs = arsta->txrate.mcs;
		sinfo->txrate.nss = arsta->txrate.nss;
		sinfo->txrate.bw = arsta->txrate.bw;
		sinfo->txrate.he_gi = arsta->txrate.he_gi;
		sinfo->txrate.he_dcm = arsta->txrate.he_dcm;
		sinfo->txrate.he_ru_alloc = arsta->txrate.he_ru_alloc;
	}
	sinfo->txrate.flags = arsta->txrate.flags;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BITRATE);

	/* TODO: Use real NF instead of default one. */
	sinfo->signal = arsta->rssi_comb + ATH11K_DEFAULT_NOISE_FLOOR;
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_SIGNAL);
}

static const struct ieee80211_ops ath11k_ops = {
	.tx				= ath11k_mac_op_tx,
	.start                          = ath11k_mac_op_start,
	.stop                           = ath11k_mac_op_stop,
	.reconfig_complete              = ath11k_mac_op_reconfig_complete,
	.add_interface                  = ath11k_mac_op_add_interface,
	.remove_interface		= ath11k_mac_op_remove_interface,
	.update_vif_offload		= ath11k_mac_op_update_vif_offload,
	.config                         = ath11k_mac_op_config,
	.bss_info_changed               = ath11k_mac_op_bss_info_changed,
	.configure_filter		= ath11k_mac_op_configure_filter,
	.hw_scan                        = ath11k_mac_op_hw_scan,
	.cancel_hw_scan                 = ath11k_mac_op_cancel_hw_scan,
	.set_key                        = ath11k_mac_op_set_key,
	.sta_state                      = ath11k_mac_op_sta_state,
	.sta_set_txpwr			= ath11k_mac_op_sta_set_txpwr,
	.sta_rc_update			= ath11k_mac_op_sta_rc_update,
	.conf_tx                        = ath11k_mac_op_conf_tx,
	.set_antenna			= ath11k_mac_op_set_antenna,
	.get_antenna			= ath11k_mac_op_get_antenna,
	.ampdu_action			= ath11k_mac_op_ampdu_action,
	.add_chanctx			= ath11k_mac_op_add_chanctx,
	.remove_chanctx			= ath11k_mac_op_remove_chanctx,
	.change_chanctx			= ath11k_mac_op_change_chanctx,
	.assign_vif_chanctx		= ath11k_mac_op_assign_vif_chanctx,
	.unassign_vif_chanctx		= ath11k_mac_op_unassign_vif_chanctx,
	.switch_vif_chanctx		= ath11k_mac_op_switch_vif_chanctx,
	.set_rts_threshold		= ath11k_mac_op_set_rts_threshold,
	.set_frag_threshold		= ath11k_mac_op_set_frag_threshold,
	.set_bitrate_mask		= ath11k_mac_op_set_bitrate_mask,
	.get_survey			= ath11k_mac_op_get_survey,
	.flush				= ath11k_mac_op_flush,
	.sta_statistics			= ath11k_mac_op_sta_statistics,
	CFG80211_TESTMODE_CMD(ath11k_tm_cmd)
#ifdef CONFIG_ATH11K_DEBUGFS
	.sta_add_debugfs		= ath11k_debugfs_sta_op_add,
#endif
};

static void ath11k_mac_update_ch_list(struct ath11k *ar,
				      struct ieee80211_supported_band *band,
				      u32 freq_low, u32 freq_high)
{
	int i;

	if (!(freq_low && freq_high))
		return;

	for (i = 0; i < band->n_channels; i++) {
		if (band->channels[i].center_freq < freq_low ||
		    band->channels[i].center_freq > freq_high)
			band->channels[i].flags |= IEEE80211_CHAN_DISABLED;
	}
}

static u32 ath11k_get_phy_id(struct ath11k *ar, u32 band)
{
	struct ath11k_pdev *pdev = ar->pdev;
	struct ath11k_pdev_cap *pdev_cap = &pdev->cap;

	if (band == WMI_HOST_WLAN_2G_CAP)
		return pdev_cap->band[NL80211_BAND_2GHZ].phy_id;

	if (band == WMI_HOST_WLAN_5G_CAP)
		return pdev_cap->band[NL80211_BAND_5GHZ].phy_id;

	ath11k_warn(ar->ab, "unsupported phy cap:%d\n", band);

	return 0;
}

static int ath11k_mac_setup_channels_rates(struct ath11k *ar,
					   u32 supported_bands)
{
	struct ieee80211_supported_band *band;
	struct ath11k_hal_reg_capabilities_ext *reg_cap;
	void *channels;
	u32 phy_id;

	BUILD_BUG_ON((ARRAY_SIZE(ath11k_2ghz_channels) +
		      ARRAY_SIZE(ath11k_5ghz_channels) +
		      ARRAY_SIZE(ath11k_6ghz_channels)) !=
		     ATH11K_NUM_CHANS);

	reg_cap = &ar->ab->hal_reg_cap[ar->pdev_idx];

	if (supported_bands & WMI_HOST_WLAN_2G_CAP) {
		channels = kmemdup(ath11k_2ghz_channels,
				   sizeof(ath11k_2ghz_channels),
				   GFP_KERNEL);
		if (!channels)
			return -ENOMEM;

		band = &ar->mac.sbands[NL80211_BAND_2GHZ];
		band->band = NL80211_BAND_2GHZ;
		band->n_channels = ARRAY_SIZE(ath11k_2ghz_channels);
		band->channels = channels;
		band->n_bitrates = ath11k_g_rates_size;
		band->bitrates = ath11k_g_rates;
		ar->hw->wiphy->bands[NL80211_BAND_2GHZ] = band;

		if (ar->ab->hw_params.single_pdev_only) {
			phy_id = ath11k_get_phy_id(ar, WMI_HOST_WLAN_2G_CAP);
			reg_cap = &ar->ab->hal_reg_cap[phy_id];
		}
		ath11k_mac_update_ch_list(ar, band,
					  reg_cap->low_2ghz_chan,
					  reg_cap->high_2ghz_chan);
	}

	if (supported_bands & WMI_HOST_WLAN_5G_CAP) {
		if (reg_cap->high_5ghz_chan >= ATH11K_MAX_6G_FREQ) {
			channels = kmemdup(ath11k_6ghz_channels,
					   sizeof(ath11k_6ghz_channels), GFP_KERNEL);
			if (!channels) {
				kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
				return -ENOMEM;
			}

			ar->supports_6ghz = true;
			band = &ar->mac.sbands[NL80211_BAND_6GHZ];
			band->band = NL80211_BAND_6GHZ;
			band->n_channels = ARRAY_SIZE(ath11k_6ghz_channels);
			band->channels = channels;
			band->n_bitrates = ath11k_a_rates_size;
			band->bitrates = ath11k_a_rates;
			ar->hw->wiphy->bands[NL80211_BAND_6GHZ] = band;
			ath11k_mac_update_ch_list(ar, band,
						  reg_cap->low_5ghz_chan,
						  reg_cap->high_5ghz_chan);
		}

		if (reg_cap->low_5ghz_chan < ATH11K_MIN_6G_FREQ) {
			channels = kmemdup(ath11k_5ghz_channels,
					   sizeof(ath11k_5ghz_channels),
					   GFP_KERNEL);
			if (!channels) {
				kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
				kfree(ar->mac.sbands[NL80211_BAND_6GHZ].channels);
				return -ENOMEM;
			}

			band = &ar->mac.sbands[NL80211_BAND_5GHZ];
			band->band = NL80211_BAND_5GHZ;
			band->n_channels = ARRAY_SIZE(ath11k_5ghz_channels);
			band->channels = channels;
			band->n_bitrates = ath11k_a_rates_size;
			band->bitrates = ath11k_a_rates;
			ar->hw->wiphy->bands[NL80211_BAND_5GHZ] = band;

			if (ar->ab->hw_params.single_pdev_only) {
				phy_id = ath11k_get_phy_id(ar, WMI_HOST_WLAN_5G_CAP);
				reg_cap = &ar->ab->hal_reg_cap[phy_id];
			}

			ath11k_mac_update_ch_list(ar, band,
						  reg_cap->low_5ghz_chan,
						  reg_cap->high_5ghz_chan);
		}
	}

	return 0;
}

static int ath11k_mac_setup_iface_combinations(struct ath11k *ar)
{
	struct ath11k_base *ab = ar->ab;
	struct ieee80211_iface_combination *combinations;
	struct ieee80211_iface_limit *limits;
	int n_limits;

	combinations = kzalloc(sizeof(*combinations), GFP_KERNEL);
	if (!combinations)
		return -ENOMEM;

	n_limits = 2;

	limits = kcalloc(n_limits, sizeof(*limits), GFP_KERNEL);
	if (!limits) {
		kfree(combinations);
		return -ENOMEM;
	}

	limits[0].max = 1;
	limits[0].types |= BIT(NL80211_IFTYPE_STATION);

	limits[1].max = 16;
	limits[1].types |= BIT(NL80211_IFTYPE_AP);

	if (IS_ENABLED(CONFIG_MAC80211_MESH) &&
	    ab->hw_params.interface_modes & BIT(NL80211_IFTYPE_MESH_POINT))
		limits[1].types |= BIT(NL80211_IFTYPE_MESH_POINT);

	combinations[0].limits = limits;
	combinations[0].n_limits = n_limits;
	combinations[0].max_interfaces = 16;
	combinations[0].num_different_channels = 1;
	combinations[0].beacon_int_infra_match = true;
	combinations[0].beacon_int_min_gcd = 100;
	combinations[0].radar_detect_widths = BIT(NL80211_CHAN_WIDTH_20_NOHT) |
						BIT(NL80211_CHAN_WIDTH_20) |
						BIT(NL80211_CHAN_WIDTH_40) |
						BIT(NL80211_CHAN_WIDTH_80);

	ar->hw->wiphy->iface_combinations = combinations;
	ar->hw->wiphy->n_iface_combinations = 1;

	return 0;
}

static const u8 ath11k_if_types_ext_capa[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
};

static const u8 ath11k_if_types_ext_capa_sta[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
	[9] = WLAN_EXT_CAPA10_TWT_REQUESTER_SUPPORT,
};

static const u8 ath11k_if_types_ext_capa_ap[] = {
	[0] = WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING,
	[7] = WLAN_EXT_CAPA8_OPMODE_NOTIF,
	[9] = WLAN_EXT_CAPA10_TWT_RESPONDER_SUPPORT,
};

static const struct wiphy_iftype_ext_capab ath11k_iftypes_ext_capa[] = {
	{
		.extended_capabilities = ath11k_if_types_ext_capa,
		.extended_capabilities_mask = ath11k_if_types_ext_capa,
		.extended_capabilities_len = sizeof(ath11k_if_types_ext_capa),
	}, {
		.iftype = NL80211_IFTYPE_STATION,
		.extended_capabilities = ath11k_if_types_ext_capa_sta,
		.extended_capabilities_mask = ath11k_if_types_ext_capa_sta,
		.extended_capabilities_len =
				sizeof(ath11k_if_types_ext_capa_sta),
	}, {
		.iftype = NL80211_IFTYPE_AP,
		.extended_capabilities = ath11k_if_types_ext_capa_ap,
		.extended_capabilities_mask = ath11k_if_types_ext_capa_ap,
		.extended_capabilities_len =
				sizeof(ath11k_if_types_ext_capa_ap),
	},
};

static void __ath11k_mac_unregister(struct ath11k *ar)
{
	cancel_work_sync(&ar->regd_update_work);

	ieee80211_unregister_hw(ar->hw);

	idr_for_each(&ar->txmgmt_idr, ath11k_mac_tx_mgmt_pending_free, ar);
	idr_destroy(&ar->txmgmt_idr);

	kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_5GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_6GHZ].channels);

	kfree(ar->hw->wiphy->iface_combinations[0].limits);
	kfree(ar->hw->wiphy->iface_combinations);

	SET_IEEE80211_DEV(ar->hw, NULL);
}

void ath11k_mac_unregister(struct ath11k_base *ab)
{
	struct ath11k *ar;
	struct ath11k_pdev *pdev;
	int i;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = &ab->pdevs[i];
		ar = pdev->ar;
		if (!ar)
			continue;

		__ath11k_mac_unregister(ar);
	}
}

static int __ath11k_mac_register(struct ath11k *ar)
{
	struct ath11k_base *ab = ar->ab;
	struct ath11k_pdev_cap *cap = &ar->pdev->cap;
	static const u32 cipher_suites[] = {
		WLAN_CIPHER_SUITE_TKIP,
		WLAN_CIPHER_SUITE_CCMP,
		WLAN_CIPHER_SUITE_AES_CMAC,
		WLAN_CIPHER_SUITE_BIP_CMAC_256,
		WLAN_CIPHER_SUITE_BIP_GMAC_128,
		WLAN_CIPHER_SUITE_BIP_GMAC_256,
		WLAN_CIPHER_SUITE_GCMP,
		WLAN_CIPHER_SUITE_GCMP_256,
		WLAN_CIPHER_SUITE_CCMP_256,
	};
	int ret;
	u32 ht_cap = 0;

	ath11k_pdev_caps_update(ar);

	SET_IEEE80211_PERM_ADDR(ar->hw, ar->mac_addr);

	SET_IEEE80211_DEV(ar->hw, ab->dev);

	ret = ath11k_mac_setup_channels_rates(ar,
					      cap->supported_bands);
	if (ret)
		goto err;

	ath11k_mac_setup_ht_vht_cap(ar, cap, &ht_cap);
	ath11k_mac_setup_he_cap(ar, cap);

	ret = ath11k_mac_setup_iface_combinations(ar);
	if (ret) {
		ath11k_err(ar->ab, "failed to setup interface combinations: %d\n", ret);
		goto err_free_channels;
	}

	ar->hw->wiphy->available_antennas_rx = cap->rx_chain_mask;
	ar->hw->wiphy->available_antennas_tx = cap->tx_chain_mask;

	ar->hw->wiphy->interface_modes = ab->hw_params.interface_modes;

	ieee80211_hw_set(ar->hw, SIGNAL_DBM);
	ieee80211_hw_set(ar->hw, SUPPORTS_PS);
	ieee80211_hw_set(ar->hw, SUPPORTS_DYNAMIC_PS);
	ieee80211_hw_set(ar->hw, MFP_CAPABLE);
	ieee80211_hw_set(ar->hw, REPORTS_TX_ACK_STATUS);
	ieee80211_hw_set(ar->hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(ar->hw, AP_LINK_PS);
	ieee80211_hw_set(ar->hw, SPECTRUM_MGMT);
	ieee80211_hw_set(ar->hw, CONNECTION_MONITOR);
	ieee80211_hw_set(ar->hw, SUPPORTS_PER_STA_GTK);
	ieee80211_hw_set(ar->hw, WANT_MONITOR_VIF);
	ieee80211_hw_set(ar->hw, CHANCTX_STA_CSA);
	ieee80211_hw_set(ar->hw, QUEUE_CONTROL);
	ieee80211_hw_set(ar->hw, SUPPORTS_TX_FRAG);
	ieee80211_hw_set(ar->hw, REPORTS_LOW_ACK);
	ieee80211_hw_set(ar->hw, SUPPORTS_TX_ENCAP_OFFLOAD);
	if (ht_cap & WMI_HT_CAP_ENABLED) {
		ieee80211_hw_set(ar->hw, AMPDU_AGGREGATION);
		ieee80211_hw_set(ar->hw, TX_AMPDU_SETUP_IN_HW);
		ieee80211_hw_set(ar->hw, SUPPORTS_REORDERING_BUFFER);
		ieee80211_hw_set(ar->hw, SUPPORTS_AMSDU_IN_AMPDU);
		ieee80211_hw_set(ar->hw, USES_RSS);
	}

	ar->hw->wiphy->features |= NL80211_FEATURE_STATIC_SMPS;
	ar->hw->wiphy->flags |= WIPHY_FLAG_IBSS_RSN;

	/* TODO: Check if HT capability advertised from firmware is different
	 * for each band for a dual band capable radio. It will be tricky to
	 * handle it when the ht capability different for each band.
	 */
	if (ht_cap & WMI_HT_CAP_DYNAMIC_SMPS)
		ar->hw->wiphy->features |= NL80211_FEATURE_DYNAMIC_SMPS;

	ar->hw->wiphy->max_scan_ssids = WLAN_SCAN_PARAMS_MAX_SSID;
	ar->hw->wiphy->max_scan_ie_len = WLAN_SCAN_PARAMS_MAX_IE_LEN;

	ar->hw->max_listen_interval = ATH11K_MAX_HW_LISTEN_INTERVAL;

	ar->hw->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	ar->hw->wiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
	ar->hw->wiphy->max_remain_on_channel_duration = 5000;

	ar->hw->wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
	ar->hw->wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE |
				   NL80211_FEATURE_AP_SCAN;

	ar->max_num_stations = TARGET_NUM_STATIONS;
	ar->max_num_peers = TARGET_NUM_PEERS_PDEV;

	ar->hw->wiphy->max_ap_assoc_sta = ar->max_num_stations;

	ar->hw->queues = ATH11K_HW_MAX_QUEUES;
	ar->hw->wiphy->tx_queue_len = ATH11K_QUEUE_LEN;
	ar->hw->offchannel_tx_hw_queue = ATH11K_HW_MAX_QUEUES - 1;
	ar->hw->max_rx_aggregation_subframes = IEEE80211_MAX_AMPDU_BUF;

	ar->hw->vif_data_size = sizeof(struct ath11k_vif);
	ar->hw->sta_data_size = sizeof(struct ath11k_sta);

	wiphy_ext_feature_set(ar->hw->wiphy, NL80211_EXT_FEATURE_CQM_RSSI_LIST);
	wiphy_ext_feature_set(ar->hw->wiphy, NL80211_EXT_FEATURE_STA_TX_PWR);

	ar->hw->wiphy->cipher_suites = cipher_suites;
	ar->hw->wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);

	ar->hw->wiphy->iftype_ext_capab = ath11k_iftypes_ext_capa;
	ar->hw->wiphy->num_iftype_ext_capab =
		ARRAY_SIZE(ath11k_iftypes_ext_capa);

	if (ar->supports_6ghz) {
		wiphy_ext_feature_set(ar->hw->wiphy,
				      NL80211_EXT_FEATURE_FILS_DISCOVERY);
		wiphy_ext_feature_set(ar->hw->wiphy,
				      NL80211_EXT_FEATURE_UNSOL_BCAST_PROBE_RESP);
	}

	ath11k_reg_init(ar);

	if (!test_bit(ATH11K_FLAG_RAW_MODE, &ab->dev_flags)) {
		ar->hw->netdev_features = NETIF_F_HW_CSUM;
		ieee80211_hw_set(ar->hw, SW_CRYPTO_CONTROL);
		ieee80211_hw_set(ar->hw, SUPPORT_FAST_XMIT);
	}

	ret = ieee80211_register_hw(ar->hw);
	if (ret) {
		ath11k_err(ar->ab, "ieee80211 registration failed: %d\n", ret);
		goto err_free_if_combs;
	}

	if (!ab->hw_params.supports_monitor)
		/* There's a race between calling ieee80211_register_hw()
		 * and here where the monitor mode is enabled for a little
		 * while. But that time is so short and in practise it make
		 * a difference in real life.
		 */
		ar->hw->wiphy->interface_modes &= ~BIT(NL80211_IFTYPE_MONITOR);

	/* Apply the regd received during initialization */
	ret = ath11k_regd_update(ar, true);
	if (ret) {
		ath11k_err(ar->ab, "ath11k regd update failed: %d\n", ret);
		goto err_unregister_hw;
	}

	ret = ath11k_debugfs_register(ar);
	if (ret) {
		ath11k_err(ar->ab, "debugfs registration failed: %d\n", ret);
		goto err_unregister_hw;
	}

	return 0;

err_unregister_hw:
	ieee80211_unregister_hw(ar->hw);

err_free_if_combs:
	kfree(ar->hw->wiphy->iface_combinations[0].limits);
	kfree(ar->hw->wiphy->iface_combinations);

err_free_channels:
	kfree(ar->mac.sbands[NL80211_BAND_2GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_5GHZ].channels);
	kfree(ar->mac.sbands[NL80211_BAND_6GHZ].channels);

err:
	SET_IEEE80211_DEV(ar->hw, NULL);
	return ret;
}

int ath11k_mac_register(struct ath11k_base *ab)
{
	struct ath11k *ar;
	struct ath11k_pdev *pdev;
	int i;
	int ret;

	if (test_bit(ATH11K_FLAG_REGISTERED, &ab->dev_flags))
		return 0;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = &ab->pdevs[i];
		ar = pdev->ar;
		if (ab->pdevs_macaddr_valid) {
			ether_addr_copy(ar->mac_addr, pdev->mac_addr);
		} else {
			ether_addr_copy(ar->mac_addr, ab->mac_addr);
			ar->mac_addr[4] += i;
		}

		ret = __ath11k_mac_register(ar);
		if (ret)
			goto err_cleanup;

		idr_init(&ar->txmgmt_idr);
		spin_lock_init(&ar->txmgmt_idr_lock);
	}

	/* Initialize channel counters frequency value in hertz */
	ab->cc_freq_hz = IPQ8074_CC_FREQ_HERTZ;
	ab->free_vdev_map = (1LL << (ab->num_radios * TARGET_NUM_VDEVS)) - 1;

	return 0;

err_cleanup:
	for (i = i - 1; i >= 0; i--) {
		pdev = &ab->pdevs[i];
		ar = pdev->ar;
		__ath11k_mac_unregister(ar);
	}

	return ret;
}

int ath11k_mac_allocate(struct ath11k_base *ab)
{
	struct ieee80211_hw *hw;
	struct ath11k *ar;
	struct ath11k_pdev *pdev;
	int ret;
	int i;

	if (test_bit(ATH11K_FLAG_REGISTERED, &ab->dev_flags))
		return 0;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = &ab->pdevs[i];
		hw = ieee80211_alloc_hw(sizeof(struct ath11k), &ath11k_ops);
		if (!hw) {
			ath11k_warn(ab, "failed to allocate mac80211 hw device\n");
			ret = -ENOMEM;
			goto err_free_mac;
		}

		ar = hw->priv;
		ar->hw = hw;
		ar->ab = ab;
		ar->pdev = pdev;
		ar->pdev_idx = i;
		ar->lmac_id = ath11k_hw_get_mac_from_pdev_id(&ab->hw_params, i);

		ar->wmi = &ab->wmi_ab.wmi[i];
		/* FIXME wmi[0] is already initialized during attach,
		 * Should we do this again?
		 */
		ath11k_wmi_pdev_attach(ab, i);

		ar->cfg_tx_chainmask = pdev->cap.tx_chain_mask;
		ar->cfg_rx_chainmask = pdev->cap.rx_chain_mask;
		ar->num_tx_chains = get_num_chains(pdev->cap.tx_chain_mask);
		ar->num_rx_chains = get_num_chains(pdev->cap.rx_chain_mask);

		pdev->ar = ar;
		spin_lock_init(&ar->data_lock);
		INIT_LIST_HEAD(&ar->arvifs);
		INIT_LIST_HEAD(&ar->ppdu_stats_info);
		mutex_init(&ar->conf_mutex);
		init_completion(&ar->vdev_setup_done);
		init_completion(&ar->vdev_delete_done);
		init_completion(&ar->peer_assoc_done);
		init_completion(&ar->peer_delete_done);
		init_completion(&ar->install_key_done);
		init_completion(&ar->bss_survey_done);
		init_completion(&ar->scan.started);
		init_completion(&ar->scan.completed);
		init_completion(&ar->thermal.wmi_sync);

		INIT_DELAYED_WORK(&ar->scan.timeout, ath11k_scan_timeout_work);
		INIT_WORK(&ar->regd_update_work, ath11k_regd_update_work);

		INIT_WORK(&ar->wmi_mgmt_tx_work, ath11k_mgmt_over_wmi_tx_work);
		skb_queue_head_init(&ar->wmi_mgmt_tx_queue);
		clear_bit(ATH11K_FLAG_MONITOR_ENABLED, &ar->monitor_flags);
	}

	return 0;

err_free_mac:
	ath11k_mac_destroy(ab);

	return ret;
}

void ath11k_mac_destroy(struct ath11k_base *ab)
{
	struct ath11k *ar;
	struct ath11k_pdev *pdev;
	int i;

	for (i = 0; i < ab->num_radios; i++) {
		pdev = &ab->pdevs[i];
		ar = pdev->ar;
		if (!ar)
			continue;

		ieee80211_free_hw(ar->hw);
		pdev->ar = NULL;
	}
}