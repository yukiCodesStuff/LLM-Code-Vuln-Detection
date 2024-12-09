{
	struct ath_hw *ah = sc->sc_ah;
	struct ath_common *common = ath9k_hw_common(ah);

	hw->flags = IEEE80211_HW_RX_INCLUDES_FCS |
		IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING |
		IEEE80211_HW_SIGNAL_DBM |
		IEEE80211_HW_SUPPORTS_PS |
		IEEE80211_HW_PS_NULLFUNC_STACK |
		IEEE80211_HW_SPECTRUM_MGMT |
		IEEE80211_HW_REPORTS_TX_ACK_STATUS |
		IEEE80211_HW_SUPPORTS_RC_TABLE;

	if (sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_HT)
		 hw->flags |= IEEE80211_HW_AMPDU_AGGREGATION;

	if (AR_SREV_9160_10_OR_LATER(sc->sc_ah) || ath9k_modparam_nohwcrypt)
		hw->flags |= IEEE80211_HW_MFP_CAPABLE;

	hw->wiphy->interface_modes =
		BIT(NL80211_IFTYPE_P2P_GO) |
		BIT(NL80211_IFTYPE_P2P_CLIENT) |
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_WDS) |
		BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_ADHOC) |
		BIT(NL80211_IFTYPE_MESH_POINT);

	hw->wiphy->iface_combinations = if_comb;
	hw->wiphy->n_iface_combinations = ARRAY_SIZE(if_comb);

	if (AR_SREV_5416(sc->sc_ah))
		hw->wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

	hw->wiphy->flags |= WIPHY_FLAG_IBSS_RSN;
	hw->wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
	hw->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;

#ifdef CONFIG_PM_SLEEP

	if ((ah->caps.hw_caps & ATH9K_HW_WOW_DEVICE_CAPABLE) &&
	    device_can_wakeup(sc->dev)) {

		hw->wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT |
					  WIPHY_WOWLAN_DISCONNECT;
		hw->wiphy->wowlan.n_patterns = MAX_NUM_USER_PATTERN;
		hw->wiphy->wowlan.pattern_min_len = 1;
		hw->wiphy->wowlan.pattern_max_len = MAX_PATTERN_SIZE;

	}
	    device_can_wakeup(sc->dev)) {

		hw->wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT |
					  WIPHY_WOWLAN_DISCONNECT;
		hw->wiphy->wowlan.n_patterns = MAX_NUM_USER_PATTERN;
		hw->wiphy->wowlan.pattern_min_len = 1;
		hw->wiphy->wowlan.pattern_max_len = MAX_PATTERN_SIZE;

	}

	atomic_set(&sc->wow_sleep_proc_intr, -1);
	atomic_set(&sc->wow_got_bmiss_intr, -1);

#endif

	hw->queues = 4;
	hw->max_rates = 4;
	hw->channel_change_time = 5000;
	hw->max_listen_interval = 1;
	hw->max_rate_tries = 10;
	hw->sta_data_size = sizeof(struct ath_node);
	hw->vif_data_size = sizeof(struct ath_vif);

	hw->wiphy->available_antennas_rx = BIT(ah->caps.max_rxchains) - 1;
	hw->wiphy->available_antennas_tx = BIT(ah->caps.max_txchains) - 1;

	/* single chain devices with rx diversity */
	if (ah->caps.hw_caps & ATH9K_HW_CAP_ANT_DIV_COMB)
		hw->wiphy->available_antennas_rx = BIT(0) | BIT(1);

	sc->ant_rx = hw->wiphy->available_antennas_rx;
	sc->ant_tx = hw->wiphy->available_antennas_tx;

#ifdef CONFIG_ATH9K_RATE_CONTROL
	hw->rate_control_algorithm = "ath9k_rate_control";
#endif

	if (sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_2GHZ)
		hw->wiphy->bands[IEEE80211_BAND_2GHZ] =
			&sc->sbands[IEEE80211_BAND_2GHZ];
	if (sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_5GHZ)
		hw->wiphy->bands[IEEE80211_BAND_5GHZ] =
			&sc->sbands[IEEE80211_BAND_5GHZ];

	ath9k_reload_chainmask_settings(sc);

	SET_IEEE80211_PERM_ADDR(hw, common->macaddr);
}

int ath9k_init_device(u16 devid, struct ath_softc *sc,
		    const struct ath_bus_ops *bus_ops)
{
	struct ieee80211_hw *hw = sc->hw;
	struct ath_common *common;
	struct ath_hw *ah;
	int error = 0;
	struct ath_regulatory *reg;

	/* Bring up device */
	error = ath9k_init_softc(devid, sc, bus_ops);
	if (error)
		return error;

	ah = sc->sc_ah;
	common = ath9k_hw_common(ah);
	ath9k_set_hw_capab(sc, hw);

	/* Initialize regulatory */
	error = ath_regd_init(&common->regulatory, sc->hw->wiphy,
			      ath9k_reg_notifier);
	if (error)
		goto deinit;

	reg = &common->regulatory;

	/* Setup TX DMA */
	error = ath_tx_init(sc, ATH_TXBUF);
	if (error != 0)
		goto deinit;

	/* Setup RX DMA */
	error = ath_rx_init(sc, ATH_RXBUF);
	if (error != 0)
		goto deinit;

	ath9k_init_txpower_limits(sc);

#ifdef CONFIG_MAC80211_LEDS
	/* must be initialized before ieee80211_register_hw */
	sc->led_cdev.default_trigger = ieee80211_create_tpt_led_trigger(sc->hw,
		IEEE80211_TPT_LEDTRIG_FL_RADIO, ath9k_tpt_blink,
		ARRAY_SIZE(ath9k_tpt_blink));
#endif

	/* Register with mac80211 */
	error = ieee80211_register_hw(hw);
	if (error)
		goto rx_cleanup;

	error = ath9k_init_debug(ah);
	if (error) {
		ath_err(common, "Unable to create debugfs files\n");
		goto unregister;
	}