	 * Otherwise the chip never moved to full sleep,
	 * when no interface is up.
	 */
	ath9k_hw_disable(sc->sc_ah);
	ath9k_hw_setpower(sc->sc_ah, ATH9K_PM_FULL_SLEEP);

	return 0;