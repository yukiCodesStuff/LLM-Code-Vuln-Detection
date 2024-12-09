	u8 rx_queue;
	bool check_sequential_pn; /* needed for CCMP/GCMP */
	u8 last_pn[6]; /* PN of the last fragment if CCMP was used */
};


struct ieee80211_bss {