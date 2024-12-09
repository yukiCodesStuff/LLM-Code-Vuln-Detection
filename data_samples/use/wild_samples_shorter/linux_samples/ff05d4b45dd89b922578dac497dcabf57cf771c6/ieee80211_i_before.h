
	/* whether a parse error occurred while retrieving these elements */
	bool parse_error;
};

static inline struct ieee80211_local *hw_to_local(
	struct ieee80211_hw *hw)