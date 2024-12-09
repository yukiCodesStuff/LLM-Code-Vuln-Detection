	CHAN5G(5620, 124, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5640, 128, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH),
	CHAN5G(5660, 132, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW),
	CHAN5G(5680, 136, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW),
	CHAN5G(5700, 140, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5720, 144, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH),
	CHAN5G(5745, 149, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW),
	CHAN5G(5765, 153, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW),
	CHAN5G(5785, 157, PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH),
	CHAN5G(5805, 161, PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH),
		.cap =	IEEE80211_HT_CAP_GRN_FLD |
			IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 |
			IEEE80211_HT_CAP_LSIG_TXOP_PROT |
			IEEE80211_HT_CAP_SGI_40 |
			IEEE80211_HT_CAP_SUP_WIDTH_20_40,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
		.mcs = {
				}
			}
		}
		break;
	case DISABLE_KEY:
		if (!(IEEE80211_KEY_FLAG_PAIRWISE & key_conf->flags)) {
			if (vif_priv->bss_index != WCN36XX_HAL_BSS_INVALID_IDX)
			   struct ieee80211_scan_request *hw_req)
{
	struct wcn36xx *wcn = hw->priv;

	if (!get_feat_caps(wcn->fw_feat_caps, SCAN_OFFLOAD)) {
		/* fallback to mac80211 software scan */
		return 1;
	}

	/* Firmware scan offload is limited to 48 channels, fallback to
	 * software driven scanning otherwise.
	 */
	if (hw_req->req.n_channels > 48) {
		wcn36xx_warn("Offload scan aborted, n_channels=%u",
			     hw_req->req.n_channels);
		return 1;
	}

	mutex_lock(&wcn->scan_lock);
	if (wcn->scan_req) {

	mutex_unlock(&wcn->scan_lock);

	wcn36xx_smd_update_channel_list(wcn, &hw_req->req);
	return wcn36xx_smd_start_hw_scan(wcn, vif, &hw_req->req);
}

static void wcn36xx_cancel_hw_scan(struct ieee80211_hw *hw,
				    vif->addr,
				    bss_conf->aid);
			vif_priv->sta_assoc = false;
			wcn36xx_smd_set_link_st(wcn,
						bss_conf->bssid,
						vif->addr,
						WCN36XX_HAL_LINK_IDLE_STATE);
			goto out;
		ret = wcn36xx_smd_wlan_host_suspend_ind(wcn);
	}

	/* Disable IRQ, we don't want to handle any packet before mac80211 is
	 * resumed and ready to receive packets.
	 */
	disable_irq(wcn->tx_irq);
	disable_irq(wcn->rx_irq);

out:
	mutex_unlock(&wcn->conf_mutex);
	return ret;
}
		wcn36xx_smd_ipv6_ns_offload(wcn, vif, false);
		wcn36xx_smd_arp_offload(wcn, vif, false);
	}

	enable_irq(wcn->tx_irq);
	enable_irq(wcn->rx_irq);

	mutex_unlock(&wcn->conf_mutex);

	return 0;
}
	ieee80211_hw_set(wcn->hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(wcn->hw, SINGLE_SCAN_ON_ALL_BANDS);
	ieee80211_hw_set(wcn->hw, REPORTS_TX_ACK_STATUS);

	wcn->hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_ADHOC) |
	mutex_init(&wcn->conf_mutex);
	mutex_init(&wcn->hal_mutex);
	mutex_init(&wcn->scan_lock);
	__skb_queue_head_init(&wcn->amsdu);

	wcn->hal_buf = devm_kmalloc(wcn->dev, WCN36XX_HAL_BUF_SIZE, GFP_KERNEL);
	if (!wcn->hal_buf) {
		ret = -ENOMEM;
	iounmap(wcn->dxe_base);
	iounmap(wcn->ccu_base);

	__skb_queue_purge(&wcn->amsdu);

	mutex_destroy(&wcn->hal_mutex);
	ieee80211_free_hw(hw);

	return 0;