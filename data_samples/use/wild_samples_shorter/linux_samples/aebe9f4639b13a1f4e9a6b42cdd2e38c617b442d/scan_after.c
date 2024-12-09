	size_t new_ie_len;
	struct cfg80211_bss_ies *new_ies;
	const struct cfg80211_bss_ies *old;
	size_t cpy_len;

	lockdep_assert_held(&wiphy_to_rdev(wiphy)->bss_lock);

	ie = mgmt->u.probe_resp.variable;