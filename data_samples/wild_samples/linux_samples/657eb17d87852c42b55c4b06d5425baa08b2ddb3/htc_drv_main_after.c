{
	struct ath9k_vif_iter_data *iter_data = data;
	int i;

	if (iter_data->hw_macaddr != NULL) {
		for (i = 0; i < ETH_ALEN; i++)
			iter_data->mask[i] &= ~(iter_data->hw_macaddr[i] ^ mac[i]);
	} else {
		iter_data->hw_macaddr = mac;
	}
}
{
	struct ath_common *common = ath9k_hw_common(priv->ah);
	struct ath9k_vif_iter_data iter_data;

	/*
	 * Pick the MAC address of the first interface as the new hardware
	 * MAC address. The hardware will use it together with the BSSID mask
	 * when matching addresses.
	 */
	iter_data.hw_macaddr = NULL;
	memset(&iter_data.mask, 0xff, ETH_ALEN);

	if (vif)
		ath9k_htc_bssid_iter(&iter_data, vif->addr, vif);

	/* Get list of all active MAC addresses */
	ieee80211_iterate_active_interfaces_atomic(
		priv->hw, IEEE80211_IFACE_ITER_RESUME_ALL,
		ath9k_htc_bssid_iter, &iter_data);

	memcpy(common->bssidmask, iter_data.mask, ETH_ALEN);

	if (iter_data.hw_macaddr)
		memcpy(common->macaddr, iter_data.hw_macaddr, ETH_ALEN);

	ath_hw_setbssidmask(common);
}

static void ath9k_htc_set_opmode(struct ath9k_htc_priv *priv)
{
	if (priv->num_ibss_vif)
		priv->ah->opmode = NL80211_IFTYPE_ADHOC;
	else if (priv->num_ap_vif)
		priv->ah->opmode = NL80211_IFTYPE_AP;
	else if (priv->num_mbss_vif)
		priv->ah->opmode = NL80211_IFTYPE_MESH_POINT;
	else
		priv->ah->opmode = NL80211_IFTYPE_STATION;

	ath9k_hw_setopmode(priv->ah);
}

void ath9k_htc_reset(struct ath9k_htc_priv *priv)
{
	struct ath_hw *ah = priv->ah;
	struct ath_common *common = ath9k_hw_common(ah);
	struct ieee80211_channel *channel = priv->hw->conf.chandef.chan;
	struct ath9k_hw_cal_data *caldata = NULL;
	enum htc_phymode mode;
	__be16 htc_mode;
	u8 cmd_rsp;
	int ret;

	mutex_lock(&priv->mutex);
	ath9k_htc_ps_wakeup(priv);

	ath9k_htc_stop_ani(priv);
	ieee80211_stop_queues(priv->hw);

	del_timer_sync(&priv->tx.cleanup_timer);
	ath9k_htc_tx_drain(priv);

	WMI_CMD(WMI_DISABLE_INTR_CMDID);
	WMI_CMD(WMI_DRAIN_TXQ_ALL_CMDID);
	WMI_CMD(WMI_STOP_RECV_CMDID);

	ath9k_wmi_event_drain(priv);

	caldata = &priv->caldata;
	ret = ath9k_hw_reset(ah, ah->curchan, caldata, false);
	if (ret) {
		ath_err(common,
			"Unable to reset device (%u Mhz) reset status %d\n",
			channel->center_freq, ret);
	}
	if (ret) {
		WMI_CMD_BUF(WMI_VAP_REMOVE_CMDID, &hvif);
		goto out;
	}

	ath9k_htc_set_mac_bssid_mask(priv, vif);

	priv->vif_slot |= (1 << avp->index);
	priv->nvifs++;

	INC_VIF(priv, vif->type);

	if ((vif->type == NL80211_IFTYPE_AP) ||
	    (vif->type == NL80211_IFTYPE_MESH_POINT) ||
	    (vif->type == NL80211_IFTYPE_ADHOC))
		ath9k_htc_assign_bslot(priv, vif);

	ath9k_htc_set_opmode(priv);

	if ((priv->ah->opmode == NL80211_IFTYPE_AP) &&
	    !test_bit(OP_ANI_RUNNING, &priv->op_flags)) {
		ath9k_hw_set_tsfadjust(priv->ah, true);
		ath9k_htc_start_ani(priv);
	}

	ath_dbg(common, CONFIG, "Attach a VIF of type: %d at idx: %d\n",
		vif->type, avp->index);

out:
	ath9k_htc_ps_restore(priv);
	mutex_unlock(&priv->mutex);

	return ret;
}

static void ath9k_htc_remove_interface(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif)
{
	struct ath9k_htc_priv *priv = hw->priv;
	struct ath_common *common = ath9k_hw_common(priv->ah);
	struct ath9k_htc_vif *avp = (void *)vif->drv_priv;
	struct ath9k_htc_target_vif hvif;
	int ret = 0;
	u8 cmd_rsp;

	mutex_lock(&priv->mutex);
	ath9k_htc_ps_wakeup(priv);

	memset(&hvif, 0, sizeof(struct ath9k_htc_target_vif));
	memcpy(&hvif.myaddr, vif->addr, ETH_ALEN);
	hvif.index = avp->index;
	WMI_CMD_BUF(WMI_VAP_REMOVE_CMDID, &hvif);
	if (ret) {
		ath_err(common, "Unable to remove interface at idx: %d\n",
			avp->index);
	}
	priv->nvifs--;
	priv->vif_slot &= ~(1 << avp->index);

	ath9k_htc_remove_station(priv, vif, NULL);

	DEC_VIF(priv, vif->type);

	if ((vif->type == NL80211_IFTYPE_AP) ||
	     vif->type == NL80211_IFTYPE_MESH_POINT ||
	    (vif->type == NL80211_IFTYPE_ADHOC))
		ath9k_htc_remove_bslot(priv, vif);

	ath9k_htc_set_opmode(priv);

	ath9k_htc_set_mac_bssid_mask(priv, vif);

	/*
	 * Stop ANI only if there are no associated station interfaces.
	 */
	if ((vif->type == NL80211_IFTYPE_AP) && (priv->num_ap_vif == 0)) {
		priv->rearm_ani = false;
		ieee80211_iterate_active_interfaces_atomic(
			priv->hw, IEEE80211_IFACE_ITER_RESUME_ALL,
			ath9k_htc_vif_iter, priv);
		if (!priv->rearm_ani)
			ath9k_htc_stop_ani(priv);
	}

	ath_dbg(common, CONFIG, "Detach Interface at idx: %d\n", avp->index);

	ath9k_htc_ps_restore(priv);
	mutex_unlock(&priv->mutex);
}

static int ath9k_htc_config(struct ieee80211_hw *hw, u32 changed)
{
	struct ath9k_htc_priv *priv = hw->priv;
	struct ath_common *common = ath9k_hw_common(priv->ah);
	struct ieee80211_conf *conf = &hw->conf;
	bool chip_reset = false;
	int ret = 0;

	mutex_lock(&priv->mutex);
	ath9k_htc_ps_wakeup(priv);

	if (changed & IEEE80211_CONF_CHANGE_IDLE) {
		mutex_lock(&priv->htc_pm_lock);

		priv->ps_idle = !!(conf->flags & IEEE80211_CONF_IDLE);
		if (!priv->ps_idle)
			chip_reset = true;

		mutex_unlock(&priv->htc_pm_lock);
	}

	/*
	 * Monitor interface should be added before
	 * IEEE80211_CONF_CHANGE_CHANNEL is handled.
	 */
	if (changed & IEEE80211_CONF_CHANGE_MONITOR) {
		if ((conf->flags & IEEE80211_CONF_MONITOR) &&
		    !priv->ah->is_monitoring)
			ath9k_htc_add_monitor_interface(priv);
		else if (priv->ah->is_monitoring)
			ath9k_htc_remove_monitor_interface(priv);
	}

	if ((changed & IEEE80211_CONF_CHANGE_CHANNEL) || chip_reset) {
		struct ieee80211_channel *curchan = hw->conf.chandef.chan;
		int pos = curchan->hw_value;

		ath_dbg(common, CONFIG, "Set channel: %d MHz\n",
			curchan->center_freq);

		ath9k_cmn_get_channel(hw, priv->ah, &hw->conf.chandef);
		if (ath9k_htc_set_channel(priv, hw, &priv->ah->channels[pos]) < 0) {
			ath_err(common, "Unable to set channel\n");
			ret = -EINVAL;
			goto out;
		}
	if (ret) {
		ath_err(common, "Unable to remove interface at idx: %d\n",
			avp->index);
	}
	priv->nvifs--;
	priv->vif_slot &= ~(1 << avp->index);

	ath9k_htc_remove_station(priv, vif, NULL);

	DEC_VIF(priv, vif->type);

	if ((vif->type == NL80211_IFTYPE_AP) ||
	     vif->type == NL80211_IFTYPE_MESH_POINT ||
	    (vif->type == NL80211_IFTYPE_ADHOC))
		ath9k_htc_remove_bslot(priv, vif);

	ath9k_htc_set_opmode(priv);

	ath9k_htc_set_mac_bssid_mask(priv, vif);

	/*
	 * Stop ANI only if there are no associated station interfaces.
	 */
	if ((vif->type == NL80211_IFTYPE_AP) && (priv->num_ap_vif == 0)) {
		priv->rearm_ani = false;
		ieee80211_iterate_active_interfaces_atomic(
			priv->hw, IEEE80211_IFACE_ITER_RESUME_ALL,
			ath9k_htc_vif_iter, priv);
		if (!priv->rearm_ani)
			ath9k_htc_stop_ani(priv);
	}