{
	struct net_device *netdev =  wdev->netdev;
	struct qtnf_vif *vif;

	if (WARN_ON(!netdev))
		return -EFAULT;

	vif = qtnf_netdev_get_priv(wdev->netdev);

	if (qtnf_cmd_send_del_intf(vif))
		pr_err("VIF%u.%u: failed to delete VIF\n", vif->mac->macid,
		       vif->vifid);

	/* Stop data */
	netif_tx_stop_all_queues(netdev);
	if (netif_carrier_ok(netdev))
		netif_carrier_off(netdev);

	if (netdev->reg_state == NETREG_REGISTERED)
		unregister_netdevice(netdev);

	vif->netdev->ieee80211_ptr = NULL;
	vif->netdev = NULL;
	vif->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
	eth_zero_addr(vif->mac_addr);

	return 0;
}

static struct wireless_dev *qtnf_add_virtual_intf(struct wiphy *wiphy,
						  const char *name,
						  unsigned char name_assign_t,
						  enum nl80211_iftype type,
						  struct vif_params *params)
{
	struct qtnf_wmac *mac;
	struct qtnf_vif *vif;
	u8 *mac_addr = NULL;

	mac = wiphy_priv(wiphy);

	if (!mac)
		return ERR_PTR(-EFAULT);

	switch (type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_AP:
		vif = qtnf_mac_get_free_vif(mac);
		if (!vif) {
			pr_err("MAC%u: no free VIF available\n", mac->macid);
			return ERR_PTR(-EFAULT);
		}

		eth_zero_addr(vif->mac_addr);
		vif->bss_priority = QTNF_DEF_BSS_PRIORITY;
		vif->wdev.wiphy = wiphy;
		vif->wdev.iftype = type;
		vif->sta_state = QTNF_STA_DISCONNECTED;
		break;
	default:
		pr_err("MAC%u: unsupported IF type %d\n", mac->macid, type);
		return ERR_PTR(-ENOTSUPP);
	}
{
	struct qtnf_vif *vif = qtnf_netdev_get_priv(dev);
	int ret;

	ret = qtnf_cmd_send_stop_ap(vif);
	if (ret) {
		pr_err("VIF%u.%u: failed to stop AP operation in FW\n",
		       vif->mac->macid, vif->vifid);
		vif->bss_status &= ~QTNF_STATE_AP_START;
		vif->bss_status &= ~QTNF_STATE_AP_CONFIG;

		netif_carrier_off(vif->netdev);
	}
{
	struct qtnf_vif *vif = qtnf_netdev_get_priv(dev);
	int ret;

	if (params->mac &&
	    (vif->wdev.iftype == NL80211_IFTYPE_AP) &&
	    !is_broadcast_ether_addr(params->mac) &&
	    !qtnf_sta_list_lookup(&vif->sta_list, params->mac))
		return 0;

	qtnf_scan_done(vif->mac, true);

	ret = qtnf_cmd_send_del_sta(vif, params);
	if (ret)
		pr_err("VIF%u.%u: failed to delete STA %pM\n",
		       vif->mac->macid, vif->vifid, params->mac);
	return ret;
}

static void qtnf_scan_timeout(unsigned long data)
{
	struct qtnf_wmac *mac = (struct qtnf_wmac *)data;

	pr_warn("mac%d scan timed out\n", mac->macid);
	qtnf_scan_done(mac, true);
}

static int
qtnf_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
	struct qtnf_wmac *mac = wiphy_priv(wiphy);

	mac->scan_req = request;

	if (qtnf_cmd_send_scan(mac)) {
		pr_err("MAC%u: failed to start scan\n", mac->macid);
		mac->scan_req = NULL;
		return -EFAULT;
	}
		switch (vif->sta_state) {
		case QTNF_STA_DISCONNECTED:
			break;
		case QTNF_STA_CONNECTING:
			cfg80211_connect_result(vif->netdev,
						vif->bss_cfg.bssid, NULL, 0,
						NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL);
			qtnf_disconnect(vif->wdev.wiphy, ndev,
					WLAN_REASON_DEAUTH_LEAVING);
			break;
		case QTNF_STA_CONNECTED:
			cfg80211_disconnected(vif->netdev,
					      WLAN_REASON_DEAUTH_LEAVING,
					      NULL, 0, 1, GFP_KERNEL);
			qtnf_disconnect(vif->wdev.wiphy, ndev,
					WLAN_REASON_DEAUTH_LEAVING);
			break;
		}

		vif->sta_state = QTNF_STA_DISCONNECTED;
		qtnf_scan_done(mac, true);
	}
}

void qtnf_cfg80211_vif_reset(struct qtnf_vif *vif)
{
	if (vif->wdev.iftype == NL80211_IFTYPE_STATION) {
		switch (vif->sta_state) {
		case QTNF_STA_CONNECTING:
			cfg80211_connect_result(vif->netdev,
						vif->bss_cfg.bssid, NULL, 0,
						NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL);
			break;
		case QTNF_STA_CONNECTED:
			cfg80211_disconnected(vif->netdev,
					      WLAN_REASON_DEAUTH_LEAVING,
					      NULL, 0, 1, GFP_KERNEL);
			break;
		case QTNF_STA_DISCONNECTED:
			break;
		}