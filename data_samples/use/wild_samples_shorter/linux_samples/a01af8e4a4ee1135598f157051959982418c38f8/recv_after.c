	bool stopped;

	spin_lock_bh(&sc->rx.rxbuflock);
	ath9k_hw_abortpcurecv(ah);
	ath9k_hw_setrxfilter(ah, 0);
	stopped = ath9k_hw_stopdmarecv(ah);

	if (sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_EDMA)