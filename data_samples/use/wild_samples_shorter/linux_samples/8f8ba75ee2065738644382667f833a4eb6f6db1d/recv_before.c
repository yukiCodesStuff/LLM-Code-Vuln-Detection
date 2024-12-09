	struct ieee80211_hw *hw = sc->hw;
	struct ieee80211_hdr *hdr;
	int retval;
	bool decrypt_error = false;
	struct ath_rx_status rs;
	enum ath9k_rx_qtype qtype;
	bool edma = !!(ah->caps.hw_caps & ATH9K_HW_CAP_EDMA);
	int dma_type;
	tsf_lower = tsf & 0xffffffff;

	do {
		/* If handling rx interrupt and flush is in progress => exit */
		if (test_bit(SC_OP_RXFLUSH, &sc->sc_flags) && (flush == 0))
			break;
