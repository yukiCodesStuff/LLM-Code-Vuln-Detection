
	/* whether a parse error occurred while retrieving these elements */
	bool parse_error;

	/*
	 * scratch buffer that can be used for various element parsing related
	 * tasks, e.g., element de-fragmentation etc.
	 */
	size_t scratch_len;
	u8 *scratch_pos;
	u8 scratch[];
};

static inline struct ieee80211_local *hw_to_local(
	struct ieee80211_hw *hw)