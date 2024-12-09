	struct cfg80211_scan_info info = {
		.aborted = aborted,
	};

	mutex_lock(&mac->mac_lock);

	if (mac->scan_req) {
		cfg80211_scan_done(mac->scan_req, &info);
		mac->scan_req = NULL;
	}