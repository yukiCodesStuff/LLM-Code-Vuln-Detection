
	vif = qtnf_netdev_get_priv(wdev->netdev);

	qtnf_scan_done(vif->mac, true);

	if (qtnf_cmd_send_del_intf(vif))
		pr_err("VIF%u.%u: failed to delete VIF\n", vif->mac->macid,
		       vif->vifid);

	struct qtnf_vif *vif = qtnf_netdev_get_priv(dev);
	int ret;

	qtnf_scan_done(vif->mac, true);

	ret = qtnf_cmd_send_stop_ap(vif);
	if (ret) {
		pr_err("VIF%u.%u: failed to stop AP operation in FW\n",
		       vif->mac->macid, vif->vifid);
	    !qtnf_sta_list_lookup(&vif->sta_list, params->mac))
		return 0;

	ret = qtnf_cmd_send_del_sta(vif, params);
	if (ret)
		pr_err("VIF%u.%u: failed to delete STA %pM\n",
		       vif->mac->macid, vif->vifid, params->mac);
		}

		vif->sta_state = QTNF_STA_DISCONNECTED;
	}

	qtnf_scan_done(mac, true);
}

void qtnf_cfg80211_vif_reset(struct qtnf_vif *vif)
{