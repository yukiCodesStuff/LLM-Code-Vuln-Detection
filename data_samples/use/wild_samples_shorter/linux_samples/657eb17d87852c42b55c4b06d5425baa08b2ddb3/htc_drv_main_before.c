	struct ath9k_vif_iter_data *iter_data = data;
	int i;

	for (i = 0; i < ETH_ALEN; i++)
		iter_data->mask[i] &= ~(iter_data->hw_macaddr[i] ^ mac[i]);
}

static void ath9k_htc_set_bssid_mask(struct ath9k_htc_priv *priv,
				     struct ieee80211_vif *vif)
{
	struct ath_common *common = ath9k_hw_common(priv->ah);
	struct ath9k_vif_iter_data iter_data;

	/*
	 * Use the hardware MAC address as reference, the hardware uses it
	 * together with the BSSID mask when matching addresses.
	 */
	iter_data.hw_macaddr = common->macaddr;
	memset(&iter_data.mask, 0xff, ETH_ALEN);

	if (vif)
		ath9k_htc_bssid_iter(&iter_data, vif->addr, vif);
		ath9k_htc_bssid_iter, &iter_data);

	memcpy(common->bssidmask, iter_data.mask, ETH_ALEN);
	ath_hw_setbssidmask(common);
}

static void ath9k_htc_set_opmode(struct ath9k_htc_priv *priv)
		goto out;
	}

	ath9k_htc_set_bssid_mask(priv, vif);

	priv->vif_slot |= (1 << avp->index);
	priv->nvifs++;


	ath9k_htc_set_opmode(priv);

	ath9k_htc_set_bssid_mask(priv, vif);

	/*
	 * Stop ANI only if there are no associated station interfaces.
	 */