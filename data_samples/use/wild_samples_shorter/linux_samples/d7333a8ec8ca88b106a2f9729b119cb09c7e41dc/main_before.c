	CHAN5G(5620, 124, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5640, 128, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH),
	CHAN5G(5660, 132, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW),
	CHAN5G(5700, 140, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5745, 149, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW),
	CHAN5G(5765, 153, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW),
	CHAN5G(5785, 157, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5805, 161, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH),
		.cap =	IEEE80211_HT_CAP_GRN_FLD |
			IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 |
			IEEE80211_HT_CAP_LSIG_TXOP_PROT,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
		.mcs = {
				}
			}
		}
		/* FIXME: Only enable bmps support when encryption is enabled.
		 * For any reasons, when connected to open/no-security BSS,
		 * the wcn36xx controller in bmps mode does not forward
		 * 'wake-up' beacons despite AP sends DTIM with station AID.
		 * It could be due to a firmware issue or to the way driver
		 * configure the station.
		 */
		if (vif->type == NL80211_IFTYPE_STATION)
			vif_priv->allow_bmps = true;
		break;
	case DISABLE_KEY:
		if (!(IEEE80211_KEY_FLAG_PAIRWISE & key_conf->flags)) {
			if (vif_priv->bss_index != WCN36XX_HAL_BSS_INVALID_IDX)
			   struct ieee80211_scan_request *hw_req)
{
	struct wcn36xx *wcn = hw->priv;
	int i;

	if (!get_feat_caps(wcn->fw_feat_caps, SCAN_OFFLOAD)) {
		/* fallback to mac80211 software scan */
		return 1;
	}

	/* For unknown reason, the hardware offloaded scan only works with
	 * 2.4Ghz channels, fallback to software scan in other cases.
	 */
	for (i = 0; i < hw_req->req.n_channels; i++) {
		if (hw_req->req.channels[i]->band != NL80211_BAND_2GHZ)
			return 1;
	}

	mutex_lock(&wcn->scan_lock);
	if (wcn->scan_req) {

	mutex_unlock(&wcn->scan_lock);

	return wcn36xx_smd_start_hw_scan(wcn, vif, &hw_req->req);
}

static void wcn36xx_cancel_hw_scan(struct ieee80211_hw *hw,
				    vif->addr,
				    bss_conf->aid);
			vif_priv->sta_assoc = false;
			vif_priv->allow_bmps = false;
			wcn36xx_smd_set_link_st(wcn,
						bss_conf->bssid,
						vif->addr,
						WCN36XX_HAL_LINK_IDLE_STATE);
			goto out;
		ret = wcn36xx_smd_wlan_host_suspend_ind(wcn);
	}
out:
	mutex_unlock(&wcn->conf_mutex);
	return ret;
}
		wcn36xx_smd_ipv6_ns_offload(wcn, vif, false);
		wcn36xx_smd_arp_offload(wcn, vif, false);
	}
	mutex_unlock(&wcn->conf_mutex);

	return 0;
}
	ieee80211_hw_set(wcn->hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(wcn->hw, SINGLE_SCAN_ON_ALL_BANDS);
	ieee80211_hw_set(wcn->hw, REPORTS_TX_ACK_STATUS);
	ieee80211_hw_set(wcn->hw, CONNECTION_MONITOR);

	wcn->hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_ADHOC) |
	mutex_init(&wcn->conf_mutex);
	mutex_init(&wcn->hal_mutex);
	mutex_init(&wcn->scan_lock);

	wcn->hal_buf = devm_kmalloc(wcn->dev, WCN36XX_HAL_BUF_SIZE, GFP_KERNEL);
	if (!wcn->hal_buf) {
		ret = -ENOMEM;
	iounmap(wcn->dxe_base);
	iounmap(wcn->ccu_base);

	mutex_destroy(&wcn->hal_mutex);
	ieee80211_free_hw(hw);

	return 0;